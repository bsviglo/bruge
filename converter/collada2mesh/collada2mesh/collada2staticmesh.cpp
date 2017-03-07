#include "collada2mesh.hpp"
#include "pugixml/pugixml.hpp"
#include <vector>
#include <map>
#include <sstream>

using namespace pugi;
using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	enum EColladaSourceStreamType
	{
		COLLADA_SOURCE_STREAM_TYPE_POSITION = 0,
		COLLADA_SOURCE_STREAM_TYPE_TEXCOORD = 1,
		COLLADA_SOURCE_STREAM_TYPE_NORMAL	= 2,
		COLLADA_SOURCE_STREAM_TYPE_TANGENT	= 3,
		COLLADA_SOURCE_STREAM_TYPE_BINORNAL = 4,
		//COLLADA_SOURCE_STREAM_TYPE_COLOR	= 5,
		COLLADA_SOURCE_STREAM_TYPE_COUNT
	};

	//-- Note: to guaranty compact one byte aligned packing.
#pragma pack(push, 1)

	//----------------------------------------------------------------------------------------------
	struct Vertex
	{
		struct Common
		{
			vec3f m_pos;
			vec2f m_uv;
			vec3f m_normal;
		};

		struct Tangent
		{
			vec3f m_tangent;
			vec3f m_binormal;
		};

		Common  m_common;
		Tangent m_tangent;
	};

#pragma pack(pop)

	//----------------------------------------------------------------------------------------------
	struct VertexIndex
	{
		VertexIndex() : m_pos(0), m_uv(0), m_normal(0), m_tangent(0), m_binormal(0) { }

		bool operator < (const VertexIndex& right) const
		{
			if (m_pos		!= right.m_pos)			return m_pos		< right.m_pos;
			if (m_uv		!= right.m_uv)			return m_uv			< right.m_uv;
			if (m_normal	!= right.m_normal)		return m_normal		< right.m_normal;
			if (m_tangent	!= right.m_tangent)		return m_tangent	< right.m_tangent;
			if (m_binormal	!= right.m_binormal)	return m_binormal	< right.m_binormal;

			return false;
		}

		uint16 m_pos;
		uint16 m_uv;
		uint16 m_normal;
		uint16 m_tangent;
		uint16 m_binormal;
	};

	//----------------------------------------------------------------------------------------------
	struct MeshSource
	{
		std::string				 m_name;
		std::vector<VertexIndex> m_indices;
		std::vector<vec3f>		 m_posArray;
		std::vector<vec2f>		 m_uvArray;
		std::vector<vec3f>		 m_normalArray;
		std::vector<vec3f>		 m_tangentArray;
		std::vector<vec3f>		 m_binormalArray;
	};
	typedef std::vector<MeshSource> MeshSources;

	//----------------------------------------------------------------------------------------------
	struct Mesh
	{
		std::vector<uint16> m_indices;
		std::vector<Vertex> m_vertices;
	};
	typedef std::vector<Mesh> Meshes;

	//-- Simple indexator needed to build the indices buffer by gathering only unique vertices from
	//-- the two or more independent data storages. For example vertices, texture coordinates and
	//-- normals arrays and appropriate for them vertex faces, texture coordinate faces and normal
	//-- faces.
	//----------------------------------------------------------------------------------------------
	template <typename VERTEX>
	class Indexator
	{
	public:
		typedef std::map<VERTEX, uint16> VertexSet;

	public:
		Indexator() : m_indices(0) { }

		uint16 index(const VERTEX& vert)
		{
			auto iter = m_vertices.find(vert);
			if (iter == m_vertices.end())
			{
				m_vertices[vert] = m_indices;
				return m_indices++;
			}
			return iter->second;
		}

		const VertexSet& vertices() const
		{
			return m_vertices;
		}

	private:
		VertexSet m_vertices;
		uint16    m_indices;
	};

	//----------------------------------------------------------------------------------------------
	void reindexVertices(
		std::vector<uint16>& oIndices,
		std::vector<Vertex>& oVertices,
		const MeshSource& iSrc
		)
	{
		Indexator<VertexIndex> indexator;
		oIndices.resize(iSrc.m_indices.size());

		//-- 1. gather only unique vertices.
		for (uint i = 0; i < iSrc.m_indices.size(); ++i)
		{
			oIndices[i] = indexator.index(iSrc.m_indices[i]);
		}

		//-- 2. For now indexator contains only unique vertices. What is left to do that is restore
		//--    vertices by indices in appropriate arrays.
		oVertices.resize(indexator.vertices().size());
		for (auto idx : indexator.vertices())
		{
			Vertex vert;
			vert.m_common.m_pos	      = iSrc.m_posArray[idx.first.m_pos];
			vert.m_common.m_uv        = iSrc.m_uvArray[idx.first.m_uv];
			vert.m_common.m_normal    = iSrc.m_normalArray[idx.first.m_normal];
			vert.m_tangent.m_tangent  = iSrc.m_tangentArray[idx.first.m_tangent];
			vert.m_tangent.m_binormal = iSrc.m_binormalArray[idx.first.m_binormal];

			oVertices[idx.second] = vert;
		}

		//-- revert indexing order.
#if 1
		for (uint i = 0; i < oIndices.size(); i += 3)
		{
			std::swap(oIndices[i + 1], oIndices[i + 2]);
		}
#endif
	}

	//----------------------------------------------------------------------------------------------
	void verifyCOLLADA(xml_document& oDoc, const ROData& iData)
	{
		//-- try to parse document.
		if (!oDoc.load_buffer(iData.ptr(), iData.length()))
		{
			throw "Can't parse COLLADA file. Maybe it has been corrupted.";
		}

		//-- extract root element.
		xml_node root = oDoc.document_element();

		//-- check required xml's section before starting parsing.
		auto lib_scene = root.child("library_visual_scenes").child("visual_scene");
		auto lib_geoms = root.child("library_geometries");

		if (!lib_geoms || !lib_scene)
		{
			throw "COLLADA document doesn't have either <visual_scene> or <library_geometries>.";
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherMeshSources(xml_document& doc, MeshSources& oSources)
	{
		auto lib_scene = doc.document_element().child("library_visual_scenes").child("visual_scene");
		auto lib_geoms = doc.document_element().child("library_geometries");

		//-- try to find all geometry's urls.
		std::vector<std::string> geometryURLs;
		for (auto node : lib_scene.children("node"))
		{
			if (auto instance_geometry = node.child("instance_geometry"))
			{
				std::string url = instance_geometry.attribute("url").value();
				geometryURLs.push_back(url.substr(1, url.length()));
			}
		}

		//-- resize meshes array because now we know total count of them.
		oSources.resize(geometryURLs.size());

		//-- try to load geometry.
		for (uint i = 0; i < geometryURLs.size(); ++i)
		{
			if (auto geom = lib_geoms.find_child_by_attribute("geometry", "id", geometryURLs[i].c_str()))
			{
				if (auto mesh = geom.child("mesh"))
				{
					MeshSource& meshSrc = oSources[i];
					meshSrc.m_name = geom.attribute("name").value();

					//-- prepare source streams
					std::string srcStreams[COLLADA_SOURCE_STREAM_TYPE_COUNT];
					srcStreams[COLLADA_SOURCE_STREAM_TYPE_POSITION] = mesh.child("vertices").find_child_by_attribute("input", "semantic", "POSITION").attribute("source").value();
					srcStreams[COLLADA_SOURCE_STREAM_TYPE_NORMAL]	= mesh.child("triangles").find_child_by_attribute("input", "semantic", "NORMAL").attribute("source").value();
					srcStreams[COLLADA_SOURCE_STREAM_TYPE_TEXCOORD] = mesh.child("triangles").find_child_by_attribute("input", "semantic", "TEXCOORD").attribute("source").value();
					srcStreams[COLLADA_SOURCE_STREAM_TYPE_TANGENT]	= mesh.child("triangles").find_child_by_attribute("input", "semantic", "TEXTANGENT").attribute("source").value();
					srcStreams[COLLADA_SOURCE_STREAM_TYPE_BINORNAL] = mesh.child("triangles").find_child_by_attribute("input", "semantic", "TEXBINORMAL").attribute("source").value();

					for (auto& stream : srcStreams)
					{
						//-- remove # char
						stream = stream.substr(1, stream.length());

						if (stream.empty())
						{
							throw "COLLADA file doesn't contain all the needed source vertex streams (tangent and binormal most probably).";
						}
					}

					//-- read triangle indices.
					if (auto triangles = mesh.child("triangles"))
					{
						uint32 count = triangles.attribute("count").as_uint() * 3;

						if (auto p = triangles.child("p"))
						{
							//-- set up a stringstream to read from the raw array.
							std::stringstream stm(p.first_child().value());
							
							meshSrc.m_indices.resize(count);
							for (uint32 i = 0; i < count; ++i)
							{
								uint32 idx = 0;
								stm >> idx;

								//-- ToDo: for now we're assuming that all of the arrays have the same indices.
								VertexIndex v;
								v.m_pos = idx;
								v.m_normal = idx;
								v.m_uv = idx;
								v.m_tangent = idx;
								v.m_binormal = idx;

								meshSrc.m_indices[i] = v;
							}
						}
					}

					//-- now read the sources
					for (auto source : mesh.children("source"))
					{
						if (auto float_array = source.child("float_array"))
						{
							//-- set up a stringstream to read from the raw array.
							std::stringstream stm(float_array.first_child().value());
							std::string	 type		= float_array.attribute("id").value();
							auto         accessor	= source.child("technique_common").child("accessor");
							uint32		 count		= accessor.attribute("count").as_uint();
							uint32		 stride		= accessor.attribute("stride").as_uint();

							if (!accessor)
							{
								throw "COLLADA file has invalid source stream accessor.";
							}

							if (type.find(srcStreams[COLLADA_SOURCE_STREAM_TYPE_POSITION]) != std::string::npos)
							{
								meshSrc.m_posArray.resize(count);
								for (auto& v3 : meshSrc.m_posArray)
								{
									stm >> v3.x; stm >> v3.z; stm >> v3.y;
								}
							}
							if (type.find(srcStreams[COLLADA_SOURCE_STREAM_TYPE_TANGENT]) != std::string::npos)
							{
								meshSrc.m_tangentArray.resize(count);
								for (auto& v3 : meshSrc.m_tangentArray)
								{
									stm >> v3.x; stm >> v3.z; stm >> v3.y;
									v3.normalize();
								}
							}
							if (type.find(srcStreams[COLLADA_SOURCE_STREAM_TYPE_BINORNAL]) != std::string::npos)
							{
								meshSrc.m_binormalArray.resize(count);
								for (auto& v3 : meshSrc.m_binormalArray)
								{
									stm >> v3.x; stm >> v3.z; stm >> v3.y;
									v3.normalize();
								}
							}
							if (type.find(srcStreams[COLLADA_SOURCE_STREAM_TYPE_NORMAL]) != std::string::npos)
							{
								meshSrc.m_normalArray.resize(count);
								for (auto& v3 : meshSrc.m_normalArray)
								{
									stm >> v3.x; stm >> v3.z; stm >> v3.y;
									v3.normalize();
								}
							}
							if (type.find(srcStreams[COLLADA_SOURCE_STREAM_TYPE_TEXCOORD]) != std::string::npos)
							{
								meshSrc.m_uvArray.resize(count);
								for (auto& v2 : meshSrc.m_uvArray)
								{
									stm >> v2.x; stm >> v2.y;
									
									//--
									if (stride == 3)
									{
										float temp;
										stm >> temp;
									}

									v2.y = 1.0f - v2.y;
								}
							}
						}
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void processMeshSources(const MeshSources& iSources, Meshes& oMeshes, AABB& oAABB)
	{
		//-- For now all needed info about meshes has been read and we run indexator to prepare
		//-- vertices for gpu processing.
		oMeshes.resize(iSources.size());
		for (uint i = 0; i < iSources.size(); ++i)
		{
			reindexVertices(oMeshes[i].m_indices, oMeshes[i].m_vertices, iSources[i]);
		}

		//-- calculate bounds for whole mesh.
		for (uint i = 0; i < iSources.size(); ++i)
		{
			const MeshSource& meshSrc = iSources[i];
			for (uint j = 0; j < meshSrc.m_posArray.size(); ++j)
			{
				oAABB.include(meshSrc.m_posArray[j]);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void saveMeshes(const MeshSources& iMeshSources, const Meshes& iMeshes, const AABB& aabb, WOData& oData)
	{
		//-- 1. write header
		{
			StaticMeshFormat::Header header;
			memset(&header, 0, sizeof(header));

			strcpy_s(header.m_format, "static_mesh");
			header.m_version = 1;

			oData.write(header);
		}

		//-- 2. write mesh info.
		{
			StaticMeshFormat::Info info;
			memset(&info, 0, sizeof(info));

			strcpy_s(info.m_format, "xyzuvntb");
			info.m_formatSize   = sizeof(Vertex);
			info.m_numSubMeshes = iMeshes.size();
			info.m_aabb[0] = aabb.m_min.x;
			info.m_aabb[1] = aabb.m_min.y;
			info.m_aabb[2] = aabb.m_min.z;
			info.m_aabb[3] = aabb.m_max.x;
			info.m_aabb[4] = aabb.m_max.y;
			info.m_aabb[5] = aabb.m_max.z;

			oData.write(info);
		}

		//-- 3. write sub-meshes.
		{
			for (uint i = 0; i < iMeshes.size(); ++i)
			{
				const Mesh& mesh = iMeshes[i];

				//-- 3.1. write sub-mesh info.
				StaticMeshFormat::SubInfo subInfo;
				memset(&subInfo, 0, sizeof(subInfo));
				{
					strcpy_s(subInfo.m_name, iMeshSources[i].m_name.c_str());
					subInfo.m_numIndices       = mesh.m_indices.size();
					subInfo.m_numVertices      = mesh.m_vertices.size();
					subInfo.m_numVertexStreams = 2;

					oData.write(subInfo);
				}

				//-- 3.3. write each individual vertex stream.
				std::vector<Vertex::Common>  commonVertices(mesh.m_vertices.size());
				std::vector<Vertex::Tangent> tangentVertices(mesh.m_vertices.size());
				for (uint v = 0; v < mesh.m_vertices.size(); ++v)
				{
					commonVertices[v]  = mesh.m_vertices[v].m_common;
					tangentVertices[v] = mesh.m_vertices[v].m_tangent;
				}

				//-- 3.3.1. write common part of vertices.
				{
					StaticMeshFormat::VertexStream vStream;
					memset(&vStream, 0, sizeof(vStream));

					vStream.m_elemSize = sizeof(Vertex::Common);

					oData.write(vStream);

					oData.writeBytes(&commonVertices[0], sizeof(Vertex::Common) * mesh.m_vertices.size());
				}

				//-- 3.3.2. write tangent part of vertices.
				{
					StaticMeshFormat::VertexStream vStream;
					memset(&vStream, 0, sizeof(vStream));

					vStream.m_elemSize = sizeof(Vertex::Tangent);

					oData.write(vStream);

					oData.writeBytes(&tangentVertices[0], sizeof(Vertex::Tangent) * mesh.m_vertices.size());
				}

				//-- 3.3. write indices.
				oData.writeBytes(&mesh.m_indices[0], sizeof(uint16) * mesh.m_indices.size());
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	void collada2staticmesh(const ROData& iData, WOData& oData)
	{
		//-- firstly verify COLLADA document.
		xml_document doc;
		verifyCOLLADA(doc, iData);

		//-- gather mesh sources.
		MeshSources meshSources;
		gatherMeshSources(doc, meshSources);

		//-- process already gathered data.
		AABB   aabb;
		Meshes meshes;
		processMeshSources(meshSources, meshes, aabb);

		//-- Now just save them.
		saveMeshes(meshSources, meshes, aabb, oData);
	}

} //-- brUGE
