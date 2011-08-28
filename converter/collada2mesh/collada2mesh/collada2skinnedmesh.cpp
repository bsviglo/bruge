#include "collada2mesh.hpp"
#include "utils/string_utils.h"
#include "pugixml/pugixml.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace pugi;
using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	struct MeshInfo
	{
		std::string m_geomURL;
		std::string m_ctrlURL;
		std::string m_skelURL;
	};
	typedef std::vector<MeshInfo> MeshesInfo;

	//-- Note: to guaranty compact one byte aligned packing.
#pragma pack(push, 1)

	//-- 56 + 24 -> 80 bytes.
	//----------------------------------------------------------------------------------------------
	struct Vertex
	{
		struct Common
		{
			vec3f  m_pos;
			vec2f  m_uv;
			vec3f  m_normal;
			vec3ui m_joints;
			vec3f  m_weights;
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

		//-- Note: vertex position, joint indices and weights interpreted by COLLADA as a single
		//--	   entity, so we don't have to sort vertices independently on weights and joint
		//--	   indices.
		uint16 m_pos;
		uint16 m_uv;
		uint16 m_normal;
		uint16 m_tangent;
		uint16 m_binormal;
	};

	//----------------------------------------------------------------------------------------------
	struct MeshSource
	{
		std::string					m_name;
		std::vector<VertexIndex>	m_indices;
		std::vector<vec3f>			m_posArray;
		std::vector<vec2f>			m_uvArray;
		std::vector<vec3f>			m_normalArray;
		std::vector<vec3f>			m_tangentArray;
		std::vector<vec3f>			m_binormalArray;
	};
	typedef std::vector<MeshSource> MeshSources;

	//----------------------------------------------------------------------------------------------
	struct SkinningIndex
	{
		uint16 m_joint;
		uint16 m_weight;
	};

	//----------------------------------------------------------------------------------------------
	struct SkinningSource
	{
		std::vector<uint16>			m_jointsPerVertex;
		std::vector<SkinningIndex>	m_indices;
		std::vector<uint16>			m_jointArray;
		std::vector<float>			m_weightArray;
		std::vector<mat4f>			m_invBindPosArray;
		mat4f						m_bindShapeMatrix;

		std::vector<vec3ui>			m_indexedJointArray;
		std::vector<vec3f>			m_indexedWeightArray;
	};
	typedef std::vector<SkinningSource> SkinningSources;

	//----------------------------------------------------------------------------------------------
	struct JointSource
	{
		JointSource() : m_parent(-1) { }

		std::string m_name;
		std::string m_id;
		std::string m_sid;
		int8		m_parent;
		mat4f		m_matrix;
	};
	typedef std::vector<JointSource> SkeletonSource;

	//----------------------------------------------------------------------------------------------
	struct Mesh
	{
		std::vector<uint16> m_indices;
		std::vector<Vertex> m_vertices;
	};
	typedef std::vector<Mesh> Meshes;

	//----------------------------------------------------------------------------------------------
	struct Joint
	{
		std::string m_name;
		int8		m_parent;
		mat4f		m_invBindPose;
		mat4f		m_matrix;
	};
	typedef std::vector<Joint> Skeleton;

	//----------------------------------------------------------------------------------------------
	struct AnimationSource
	{
		uint16							m_numFrames;
		std::vector<std::vector<mat4f>>	m_jointMatrices;
	};

	//----------------------------------------------------------------------------------------------
	struct Animation
	{
		struct Transform
		{
			quat  m_quat;
			vec3f m_pos;
		};
		typedef std::vector<Transform>  Transforms;

		uint					m_numFrames;
		std::vector<Transforms>	m_jointTransforms;
		std::vector<AABB>		m_jointBounds;
	};

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
		const SkinningSource& iSkinningSrc,
		const MeshSource& iMeshSrc
		)
	{
		Indexator<VertexIndex> indexator;
		oIndices.resize(iMeshSrc.m_indices.size());

		//-- 1. gather only unique vertices.
		for (uint i = 0; i < iMeshSrc.m_indices.size(); ++i)
		{
			oIndices[i] = indexator.index(iMeshSrc.m_indices[i]);
		}

		//-- 2. For now indexator contains only unique vertices. What is left to do that is restore
		//--    vertices by indices in appropriate arrays.
		Vertex vert;
		const Indexator<VertexIndex>::VertexSet& vertexSet = indexator.vertices();
		oVertices.resize(vertexSet.size());
		for (auto i = vertexSet.cbegin(); i != vertexSet.cend(); ++i)
		{
			vert.m_common.m_pos	      = iMeshSrc.m_posArray[i->first.m_pos];
			vert.m_common.m_uv        = iMeshSrc.m_uvArray[i->first.m_uv];
			vert.m_common.m_normal    = iMeshSrc.m_normalArray[i->first.m_normal];
			vert.m_common.m_joints	  = iSkinningSrc.m_indexedJointArray[i->first.m_pos];
			vert.m_common.m_weights   = iSkinningSrc.m_indexedWeightArray[i->first.m_pos];
			vert.m_tangent.m_tangent  = iMeshSrc.m_tangentArray[i->first.m_tangent];
			vert.m_tangent.m_binormal = iMeshSrc.m_binormalArray[i->first.m_binormal];

			oVertices[i->second] = vert;
		}
	}

	//-- returns bone ID based on the sid name.
	//----------------------------------------------------------------------------------------------
	uint8 boneIDBySid(const std::string& sid, const SkeletonSource& skeletonSource)
	{
		for (uint i = 0; i < skeletonSource.size(); ++i)
		{
			if (sid == skeletonSource[i].m_sid)
				return i;
		}

		return 0;
	}

	//----------------------------------------------------------------------------------------------
	mat4f readMatrix(const char* matrixSrc)
	{
		mat4f m;
		std::stringstream stm(matrixSrc);
	
		for (uint i = 0; i < 16; ++i)
		{
			stm >> m[i];
		}

		m.transpose();
		return m;
	}

	//----------------------------------------------------------------------------------------------
	void verifyCOLLADA(xml_document& oDoc, const ROData& iData, bool isAnimation = false)
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
		auto lib_ctrls = root.child("library_controllers");

		if (!lib_geoms || !lib_scene || !lib_ctrls)
		{
			throw "COLLADA file doesn't contain at least one suitable skinned mesh.";
		}

		if (isAnimation)
		{
			if (!root.child("library_animations"))
			{
				throw "COLLADA file doesn't contain at least one animation for skinned mesh.";
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherMeshesInfo(xml_document& doc, MeshesInfo& oInfo)
	{
		auto lib_scene = doc.document_element().child("library_visual_scenes").child("visual_scene");
		auto lib_ctrls = doc.document_element().child("library_controllers");
		auto lib_geoms = doc.document_element().child("library_geometries");

		//-- gather all skinned meshes.
		for (auto node = lib_scene.child("node"); node; node = node.next_sibling("node"))
		{
			//-- skip all nodes with type "JOINT.
			std::string type = node.attribute("type").value();
			if (type != "JOINT")
			{
				MeshInfo info;

				//-- 1. check has this node instance_controller.
				if (auto instance_controller = node.child("instance_controller"))
				{
					std::string url = instance_controller.attribute("url").value();
					info.m_ctrlURL = url.substr(1, url.length());

					//-- 2. check is there some controller with desired URL.
					if (auto ctrl = lib_ctrls.find_child_by_attribute("controller", "id", info.m_ctrlURL.c_str()))
					{
						//-- 3. now get skin attribute and retrieve geometry URL.
						if (auto skin = ctrl.child("skin"))
						{
							std::string url = skin.attribute("source").value();
							info.m_geomURL = url.substr(1, url.length());

							//-- 4. check is there some geometry with desired URL.
							if (auto geom = lib_geoms.find_child_by_attribute("geometry", "id", info.m_geomURL.c_str()))
							{
								std::string url = instance_controller.child("skeleton").first_child().value();
								info.m_skelURL = url.substr(1, url.length());

								//-- 5. check is there some skeleton with desired URL.
								if (auto skeleton = lib_scene.find_child_by_attribute("node", "id", info.m_skelURL.c_str()))
								{
									//-- All is ok!
									oInfo.push_back(info);
								}
							}
						}
					}
				}
			}
		}

		//-- report an error in case if we didn't find any suitable skinned mesh.
		if (oInfo.empty())
		{
			throw "COLLADA file doesn't contains at least one suitable skinned mesh.";
		}

		//-- if we have more than one skinned mesh check that they all reference on the one skeleton
		//-- otherwise report model as invalid.
		for (auto info = oInfo.cbegin(); info != oInfo.cend(); ++info)
		{
			if (info->m_skelURL != oInfo[0].m_skelURL)
			{
				throw "COLLADA file contains skinned meshes with different skeletons.";
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void recursiveBuildSkeleton(xml_node& node, SkeletonSource& oSkeletonSource, int8 parent)
	{
		JointSource joint;
		joint.m_name   = node.attribute("name").value();
		joint.m_id	   = node.attribute("id").value();
		joint.m_sid    = node.attribute("sid").value();
		joint.m_parent = parent;
		joint.m_matrix = readMatrix(node.child("matrix").first_child().value());

		oSkeletonSource.push_back(joint);
		int8 curIndex = oSkeletonSource.size() - 1;

		for (auto child = node.child("node"); child; child = child.next_sibling("node"))
		{
			recursiveBuildSkeleton(child, oSkeletonSource, curIndex);
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherSkeletonSource(xml_document& doc, const MeshesInfo& info, SkeletonSource& oSkeletonSource)
	{
		auto lib_scene = doc.document_element().child("library_visual_scenes").child("visual_scene");
		auto root_node = lib_scene.find_child_by_attribute("node", "id", info[0].m_skelURL.c_str());

		recursiveBuildSkeleton(root_node, oSkeletonSource, -1);
	}

	//----------------------------------------------------------------------------------------------
	void loadGeometry(xml_node& node, MeshSource& oSource)
	{
		if (auto mesh = node.child("mesh"))
		{
			oSource.m_name = node.attribute("name").value();

			for (auto source = mesh.child("source"); source; source = source.next_sibling("source"))
			{
				if (auto float_array = source.child("float_array"))
				{
					//-- set up a stringstream to read from the raw array.
					std::stringstream stm(float_array.first_child().value());
					std::string	 type  = float_array.attribute("id").value();
					uint32		 count = float_array.attribute("count").as_uint();

					if		(type.find("position") != std::string::npos)
					{
						oSource.m_posArray.resize(count / 3);
						for (uint32 i = 0; i < oSource.m_posArray.size(); ++i)
						{
							vec3f& v3 = oSource.m_posArray[i];
							stm >> v3.x; stm >> v3.y; stm >> v3.z;

							//-- convert to meters.
							v3 = v3.scale(0.1f);
						}
					}
					else if (type.find("tangent") != std::string::npos)
					{
						oSource.m_tangentArray.resize(count / 3);
						for (uint32 i = 0; i < oSource.m_tangentArray.size(); ++i)
						{
							vec3f& v3 = oSource.m_tangentArray[i];
							stm >> v3.x; stm >> v3.y; stm >> v3.z;

							v3.normalize();
						}
					}
					else if (type.find("binormal") != std::string::npos)
					{
						oSource.m_binormalArray.resize(count / 3);
						for (uint32 i = 0; i < oSource.m_binormalArray.size(); ++i)
						{
							vec3f& v3 = oSource.m_binormalArray[i];
							stm >> v3.x; stm >> v3.y; stm >> v3.z;

							v3.normalize();
						}
					}
					else if (type.find("normal") != std::string::npos)
					{
						oSource.m_normalArray.resize(count / 3);
						for (uint32 i = 0; i < oSource.m_normalArray.size(); ++i)
						{
							vec3f& v3 = oSource.m_normalArray[i];
							stm >> v3.x; stm >> v3.y; stm >> v3.z;

							v3.normalize();
						}
					}
					else if (type.find("map1") != std::string::npos)
					{
						oSource.m_uvArray.resize(count / 3);
						for (uint32 i = 0; i < oSource.m_uvArray.size(); ++i)
						{
							float temp = 0;
							vec2f& v2 = oSource.m_uvArray[i];
							stm >> v2.x; stm >> v2.y; stm >> temp;
							v2.y = 1.0f - v2.y;
						}
					}
				}
			}

			//-- now read triangle indices.
			if (auto triangles = mesh.child("triangles"))
			{
				uint32 count = triangles.attribute("count").as_uint() * 3;

				if (auto p = triangles.child("p"))
				{
					//-- set up a stringstream to read from the raw array.
					std::stringstream stm(p.first_child().value());

					VertexIndex v;
					oSource.m_indices.resize(count);
					for (uint32 i = 0; i < count; ++i)
					{
						stm >> v.m_pos;
						stm >> v.m_normal;
						stm >> v.m_uv;
						stm >> v.m_tangent;
						v.m_binormal = v.m_tangent;

						oSource.m_indices[i] = v;
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void loadSkinning(xml_node& node, const SkeletonSource& skeletonSource, SkinningSource& oSource)
	{
		auto  skin = node.child("skin");
		oSource.m_bindShapeMatrix = readMatrix(skin.child("bind_shape_matrix").first_child().value());

		for (auto source = skin.child("source"); source; source = source.next_sibling("source"))
		{
			if (auto float_array = source.child("float_array"))
			{
				//-- set up a stringstream to read from the raw array.
				std::stringstream stm(float_array.first_child().value());
				std::string	 type  = float_array.attribute("id").value();
				uint32		 count = float_array.attribute("count").as_uint();

				if (type.find("bind_pos") != std::string::npos)
				{
					oSource.m_invBindPosArray.resize(count / 16);
					for (uint32 i = 0; i < oSource.m_invBindPosArray.size(); ++i)
					{
						mat4f& m = oSource.m_invBindPosArray[i];

						for (uint j = 0; j < 16; ++j)
						{
							stm >> m[j];
						}

						m.transpose();
					}
				}
				else if (type.find("weight") != std::string::npos)
				{
					oSource.m_weightArray.resize(count / 1);
					for (uint32 i = 0; i < oSource.m_weightArray.size(); ++i)
					{
						stm >> oSource.m_weightArray[i];
					}
				}
			}
			else if (auto name_array = source.child("Name_array"))
			{
				//-- set up a stringstream to read from the raw array.
				std::string src   = name_array.first_child().value();
				std::string	type  = name_array.attribute("id").value();

				if (type.find("joint") != std::string::npos)
				{
					StrTokenizer tokenizer(src, " ");
					std::string	token;
					while (tokenizer.hasMoreTokens())
					{
						tokenizer.nextToken(token);
						oSource.m_jointArray.push_back(boneIDBySid(token, skeletonSource));
					}
				}
			}
		}

		//-- Now read vertex weights section.
		if (auto vertex_weights = skin.child("vertex_weights"))
		{
			uint count = vertex_weights.attribute("count").as_uint();
			oSource.m_jointsPerVertex.resize(count);
			
			//-- read how much bones affect each individual vertex. Maybe any arbitrary value [0, +infinity].
			if (auto vcount = vertex_weights.child("vcount"))
			{
				//-- set up a stringstream to read from the raw array.
				std::stringstream stm(vcount.first_child().value());

				for (uint32 i = 0; i < count; ++i)
				{
					stm >> oSource.m_jointsPerVertex[i];
				}
			}

			//-- now read indices for joint and weights.
			if (auto v = vertex_weights.child("v"))
			{
				//-- set up a stringstream to read from the raw array.
				std::stringstream stm(v.first_child().value());

				for (uint i = 0; i < oSource.m_jointsPerVertex.size(); ++i)
				{
					for (uint j = 0; j < oSource.m_jointsPerVertex[i]; ++j)
					{
						SkinningIndex skinningIdx;
						
						stm >> skinningIdx.m_joint;
						stm >> skinningIdx.m_weight;

						oSource.m_indices.push_back(skinningIdx);
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherMeshSources(
		xml_document& doc,
		const MeshesInfo& iMeshesInfo,
		const SkeletonSource& skeletonSource,
		MeshSources& oMeshSources,
		SkinningSources& oSkinningSources
		)
	{
		auto lib_geoms = doc.document_element().child("library_geometries");
		auto lib_ctrls = doc.document_element().child("library_controllers");
		
		//-- try to find all geometry's urls.
		for (auto info = iMeshesInfo.cbegin(); info != iMeshesInfo.cend(); ++info)
		{
			MeshSource meshSource;
			SkinningSource skinningSource;

			//-- 1. load geometry.
			auto geom_node = lib_geoms.find_child_by_attribute("geometry", "id", info->m_geomURL.c_str());
			loadGeometry(geom_node, meshSource);

			//-- 2. load joint indices and weights.
			auto ctrl_node = lib_ctrls.find_child_by_attribute("controller", "id", info->m_ctrlURL.c_str());
			loadSkinning(ctrl_node, skeletonSource, skinningSource);

			oMeshSources.push_back(meshSource);
			oSkinningSources.push_back(skinningSource);
		}
	}

	//----------------------------------------------------------------------------------------------
	void gatherAnimationSource(xml_document& doc, const SkeletonSource& skelSrc, AnimationSource& oAnimSrc)
	{
		auto lib_anims = doc.document_element().child("library_animations");

		//-- check of available animation.
		if (auto animation = lib_anims.child("animation"))
		{
			//-- prepare data.
			oAnimSrc.m_jointMatrices.resize(skelSrc.size());

			for (auto source = animation.child("source"); source; source = source.next_sibling("source"))
			{
				//-- check has this section animation data.
				std::string id = source.attribute("id").value();
				if (id.find("output") != std::string::npos)
				{
					//-- find index of the appropriate joint.
					uint idx = 0;
					bool has = false;
					for (; idx < skelSrc.size(); ++idx)
					{
						if (id.find(skelSrc[idx].m_id + "_") != std::string::npos)
						{
							has = true;
							break;
						}
					}

					//-- skip this source.
					if (!has)
					{
						std::cout << "Skip section: " << id << "\n";
						continue;
					}
										
					//-- check has this section animation matrices.
					if (auto float_array = source.child("float_array"))
					{
						//-- set up a stringstream to read from the raw array.
						std::stringstream stm(float_array.first_child().value());
						std::string	 type  = float_array.attribute("id").value();
						uint32		 count = float_array.attribute("count").as_uint();

						if (type.find("matrix-output") != std::string::npos)
						{
							oAnimSrc.m_jointMatrices[idx].resize(count / 16);
							for (uint32 i = 0; i < oAnimSrc.m_jointMatrices[idx].size(); ++i)
							{
								mat4f& m = oAnimSrc.m_jointMatrices[idx][i];

								for (uint j = 0; j < 16; ++j)
								{
									stm >> m[j];
								}

								m.transpose();
							}

							//-- set number of frames
							oAnimSrc.m_numFrames = oAnimSrc.m_jointMatrices[idx].size();
						}
					}
				}
			}

			//-- find all nodes without data and write default matrices as source.
			for (uint i = 0; i < skelSrc.size(); ++i)
			{
				if (oAnimSrc.m_jointMatrices[i].empty())
				{
					oAnimSrc.m_jointMatrices[i].assign(oAnimSrc.m_numFrames, skelSrc[i].m_matrix);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void processMeshSources(
		const SkeletonSource& skeletonSource,
		const MeshSources& meshSources,
		SkinningSources& skinningSources,
		Meshes& oMeshes,
		Skeleton& oSkeleton,
		AABB& oAABB
		)
	{
		//-- allocate enough space for output array.
		oMeshes.resize(meshSources.size());

		for (uint i = 0; i < oMeshes.size(); ++i)
		{
			Mesh&			  oMesh   = oMeshes[i];
			SkinningSource&   skinSrc = skinningSources[i];
			const MeshSource& meshSrc = meshSources[i];

			//-- firstly process skinning source by re-indexing weights and joints array.
			uint index = 0;
			skinSrc.m_indexedWeightArray.resize(skinSrc.m_jointsPerVertex.size());
			skinSrc.m_indexedJointArray.resize(skinSrc.m_jointsPerVertex.size());

			for (uint j = 0; j < skinSrc.m_jointsPerVertex.size(); ++j)
			{
				std::vector<std::pair<uint16, float>> skinIndices;
				for (uint k = 0; k < skinSrc.m_jointsPerVertex[j]; ++k)
				{
					const SkinningIndex& idx = skinSrc.m_indices[index++];

					skinIndices.push_back(std::make_pair(
						skinSrc.m_jointArray[idx.m_joint], skinSrc.m_weightArray[idx.m_weight]
					));
				}

				//-- now clam retrieved indices so on the output we will have only three joints
				//-- with the biggest influence on the vertex.

				//-- 1. sort indices by weight descending.
				std::sort(skinIndices.begin(), skinIndices.end(),
					[](const std::pair<uint16, float>& lft, const std::pair<uint16, float>& rht) {
						return lft.second > rht.second; 
				});

				//-- 2. now get only first three joints.
				vec3f&  weights = skinSrc.m_indexedWeightArray[j];
				vec3ui& joints  = skinSrc.m_indexedJointArray[j];
				for (uint h = 0; h < skinIndices.size() && h < 3; ++h)
				{
					joints[h]  = skinIndices[h].first;
					weights[h] = skinIndices[h].second;
				}

				//-- 3. renormalize weights just in case if current vertex has more or less than
				//--    three weights.
				float sum = weights[0] + weights[1] + weights[2];
				weights = weights.scale(1.0f / sum);
			}

			//-- now we are ready to process meshes.
			reindexVertices(oMesh.m_indices, oMesh.m_vertices, skinSrc, meshSrc);
		}

		//-- compute skeleton.
		oSkeleton.resize(skeletonSource.size());
		for (uint i = 0; i < skeletonSource.size(); ++i)
		{
			oSkeleton[i].m_name			= skeletonSource[i].m_name;
			oSkeleton[i].m_parent		= skeletonSource[i].m_parent;
			oSkeleton[i].m_invBindPose	= skinningSources[0].m_invBindPosArray[i];
			oSkeleton[i].m_matrix		= skeletonSource[i].m_matrix;

			//-- convert position component of matrices to meters.
			oSkeleton[i].m_invBindPose.m30	*= 0.1f;
			oSkeleton[i].m_invBindPose.m31	*= 0.1f;
			oSkeleton[i].m_invBindPose.m32	*= 0.1f;

			oSkeleton[i].m_matrix.m30		*= 0.1f;
			oSkeleton[i].m_matrix.m31		*= 0.1f;
			oSkeleton[i].m_matrix.m32		*= 0.1f;
		}

		//-- compute absolute transformations set for the bind pose skeleton.
		for (uint i = 1; i < oSkeleton.size(); ++i)
		{
			int8 parentIdx = oSkeleton[i].m_parent;
			oSkeleton[i].m_matrix.postMultiply(oSkeleton[parentIdx].m_matrix);
		}

		//-- compute local space AABB for skinned mesh in the bind pose.
		for (uint i = 0; i < oMeshes.size(); ++i)
		{
			const Mesh& mesh = oMeshes[i];

			for (uint j = 0; j < mesh.m_vertices.size(); ++j)
			{
				oAABB.include(mesh.m_vertices[j].m_common.m_pos);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void processAnimationSource(
		const SkeletonSource& skeletonSource,
		const MeshSources& meshSources,
		SkinningSources& skinningSources,
		AnimationSource& animSource,
		Skeleton& oSkeleton,
		Animation& oAnimation
		)
	{
		Meshes meshes;
		AABB   aabb;
		processMeshSources(skeletonSource, meshSources, skinningSources, meshes, oSkeleton, aabb);

		//-- fill number of frames in animation.
		oAnimation.m_numFrames = animSource.m_numFrames;
		oAnimation.m_jointTransforms.resize(animSource.m_numFrames);
		oAnimation.m_jointBounds.resize(animSource.m_numFrames);

		//-- now convert animation matrices from mat4f to (quat, vec3f).
		for (uint i = 0; i < oAnimation.m_numFrames; ++i)
		{
			Animation::Transforms& transforms = oAnimation.m_jointTransforms[i];

			transforms.resize(oSkeleton.size());
			for (uint j = 0; j < oSkeleton.size(); ++j)
			{
				Animation::Transform&   t = transforms[j];
				mat4f&					m = animSource.m_jointMatrices[j][i];

				//-- convert position component of matrices to meters.
				m.m30 *= +0.1f;
				m.m31 *= +0.1f;
				m.m32 *= +0.1f;

				vec3f scale;
				decomposeMatrix(t.m_quat, scale, t.m_pos, m);

				//if (scale.length() != 1.0f)
				//{
				//	throw "Joint has to not have any scale.";
				//}
			}
		}

		//-- build AABB for the every animation frame.
		std::vector<mat4f> matrices(oSkeleton.size());
		for (uint i = 0; i < oAnimation.m_numFrames; ++i)
		{
			AABB& oAABB = oAnimation.m_jointBounds[i];

			//-- initialize matrices for current animation frame.
			for (uint j = 0; j < oSkeleton.size(); ++j)
			{
				matrices[j] = animSource.m_jointMatrices[j][i];
			}

			//-- compute absolute transformations set for skeleton.
			for (uint j = 1; j < oSkeleton.size(); ++j)
			{
				int8 parentIdx = oSkeleton[j].m_parent;
				matrices[j].postMultiply(matrices[parentIdx]);
			}

			//-- pre-multiply each joint with its invert bind pose matrix.
			for (uint j = 0; j < oSkeleton.size(); ++j)
			{
				matrices[j].preMultiply(oSkeleton[j].m_invBindPose);
			}

			//-- compute local space AABB.
			for (uint j = 0; j < meshes.size(); ++j)
			{
				const Mesh& mesh = meshes[j];

				for (uint k = 0; k < mesh.m_vertices.size(); ++k)
				{
					const Vertex::Common& v = mesh.m_vertices[k].m_common;
					vec3f outPos(0,0,0);
					for (uint h = 0; h < 3; ++h)
					{
						uint16 jId = v.m_joints[h];
						float  w   = v.m_weights[h];

						outPos += matrices[jId].applyToPoint(v.m_pos).scale(w);
					}
					oAABB.include(outPos);
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void saveAnimation(const Skeleton& iSkeleton, const Animation& iAnimation, WOData& oData)
	{
		//-- 1. write header
		{
			SkinnedMeshFormat::Header header;
			memset(&header, 0, sizeof(header));

			strcpy_s(header.m_format, "animation");
			header.m_version = 1;

			oData.write(header);
		}

		//-- 2. write skeleton.
		{
			//-- 2.1. write info.
			{
				SkinnedMeshFormat::Skeleton::Info skelInfo;
				memset(&skelInfo, 0, sizeof(skelInfo));

				skelInfo.m_numJoints = iSkeleton.size();

				oData.write(skelInfo);
			}

			//-- 2.2. write joints.
			{
				for (uint i = 0; i < iSkeleton.size(); ++i)
				{
					const Joint& iJoint = iSkeleton[i];

					SkinnedMeshFormat::Skeleton::Joint oJoint;
					memset(&oJoint, 0, sizeof(oJoint));

					strcpy_s(oJoint.m_name, iJoint.m_name.c_str());
					oJoint.m_parent = iJoint.m_parent;

					oData.write(oJoint);
				}
			}
		}

		//-- 3. write animation info
		{
			SkinnedMeshAnimationFormat::Info info;
			memset(&info, 0, sizeof(info));

			info.m_numFrames = iAnimation.m_numFrames;
			info.m_frameRate = 24;

			oData.write(info);
		}

		//-- 4. write animation bounds.
		for (uint i = 0; i < iAnimation.m_numFrames; ++i)
		{
			const AABB& aabb = iAnimation.m_jointBounds[i];

			SkinnedMeshAnimationFormat::Bound bound;
			memset(&bound, 0, sizeof(bound));
			
			bound.m_aabb[0] = aabb.m_min.x;
			bound.m_aabb[1] = aabb.m_min.y;
			bound.m_aabb[2] = aabb.m_min.z;
			bound.m_aabb[3] = aabb.m_max.x;
			bound.m_aabb[4] = aabb.m_max.y;
			bound.m_aabb[5] = aabb.m_max.z;

			oData.write(bound);
		}

		//-- 5. write animation joints.
		for (uint i = 0; i < iAnimation.m_numFrames; ++i)
		{
			const Animation::Transforms& ts = iAnimation.m_jointTransforms[i];

			for (uint j = 0; j < iSkeleton.size(); ++j)
			{
				const Animation::Transform& t = ts[j];

				SkinnedMeshAnimationFormat::Joint joint;
				memset(&joint, 0, sizeof(joint));

				//-- quat
				joint.m_quat[0] = t.m_quat[0];
				joint.m_quat[1] = t.m_quat[1];
				joint.m_quat[2] = t.m_quat[2];
				joint.m_quat[3] = t.m_quat[3];

				//-- pos
				joint.m_pos[0]  = t.m_pos[0];
				joint.m_pos[1]  = t.m_pos[1];
				joint.m_pos[2]  = t.m_pos[2];

				oData.write(joint);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void saveMeshes(
		const MeshSources& iMeshSources, const Skeleton& iSkeleton, const Meshes& iMeshes, const AABB& aabb, WOData& oData)
	{
		//-- 1. write header
		{
			SkinnedMeshFormat::Header header;
			memset(&header, 0, sizeof(header));

			strcpy_s(header.m_format, "skinned_mesh");
			header.m_version = 1;

			oData.write(header);
		}

		//-- 2. write skeleton.
		{
			//-- 2.1. write info.
			{
				SkinnedMeshFormat::Skeleton::Info skelInfo;
				memset(&skelInfo, 0, sizeof(skelInfo));

				skelInfo.m_numJoints = iSkeleton.size();

				oData.write(skelInfo);
			}

			//-- 2.2. write joints.
			{
				for (uint i = 0; i < iSkeleton.size(); ++i)
				{
					const Joint& iJoint = iSkeleton[i];

					SkinnedMeshFormat::Skeleton::Joint oJoint;
					memset(&oJoint, 0, sizeof(oJoint));

					strcpy_s(oJoint.m_name, iJoint.m_name.c_str());
					oJoint.m_parent      = iJoint.m_parent;

					oData.write(oJoint);
				}
			}
		}

		//-- 3. write skeleton invert bind pose.
		for (uint i = 0; i < iSkeleton.size(); ++i)
		{
			const Joint& iJoint = iSkeleton[i];

			SkinnedMeshFormat::InvBindPose oInvBindPose;
			memcpy(oInvBindPose.m_matrix, &iJoint.m_invBindPose[0], sizeof(mat4f));

			oData.write(oInvBindPose);
		}

		//-- 4. write mesh info.
		{
			SkinnedMeshFormat::Info info;
			memset(&info, 0, sizeof(info));

			strcpy_s(info.m_format, "xyzuvni3w3tb");
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

		//-- 5. write sub-meshes.
		for (uint i = 0; i < iMeshes.size(); ++i)
		{
			const Mesh& mesh = iMeshes[i];

			//-- 5.1. write sub-mesh info.
			SkinnedMeshFormat::SubInfo subInfo;
			memset(&subInfo, 0, sizeof(subInfo));
			{
				strcpy_s(subInfo.m_name, iMeshSources[i].m_name.c_str());
				subInfo.m_numIndices       = mesh.m_indices.size();
				subInfo.m_numVertices      = mesh.m_vertices.size();
				subInfo.m_numVertexStreams = 2;

				oData.write(subInfo);
			}

			//-- 5.2. write each individual vertex stream.
			std::vector<Vertex::Common>  commonVertices(mesh.m_vertices.size());
			std::vector<Vertex::Tangent> tangentVertices(mesh.m_vertices.size());
			for (uint v = 0; v < mesh.m_vertices.size(); ++v)
			{
				commonVertices[v]  = mesh.m_vertices[v].m_common;
				tangentVertices[v] = mesh.m_vertices[v].m_tangent;
			}

			//-- 5.2.1. write common part of vertices.
			{
				SkinnedMeshFormat::VertexStream vStream;
				memset(&vStream, 0, sizeof(vStream));

				vStream.m_elemSize = sizeof(Vertex::Common);

				oData.write(vStream);

				oData.writeBytes(&commonVertices[0], sizeof(Vertex::Common) * mesh.m_vertices.size());
			}

			//-- 5.2.2. write tangent part of vertices.
			{
				SkinnedMeshFormat::VertexStream vStream;
				memset(&vStream, 0, sizeof(vStream));

				vStream.m_elemSize = sizeof(Vertex::Tangent);

				oData.write(vStream);

				oData.writeBytes(&tangentVertices[0], sizeof(Vertex::Tangent) * mesh.m_vertices.size());
			}

			//-- 5.3. write indices.
			oData.writeBytes(&mesh.m_indices[0], sizeof(uint16) * mesh.m_indices.size());
		}
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	void collada2skinnedmesh(const ROData& iData, WOData& oData)
	{
		//-- firstly verify COLLADA document.
		xml_document doc;
		verifyCOLLADA(doc, iData);
		
		//-- gather skinned meshes info.
		MeshesInfo meshesInfo;
		gatherMeshesInfo(doc, meshesInfo);

		//-- gather skeleton source.
		SkeletonSource skeletonSource;
		gatherSkeletonSource(doc, meshesInfo, skeletonSource);

		//-- gather mesh sources.
		MeshSources		meshSources;
		SkinningSources skinningSources;
		gatherMeshSources(doc, meshesInfo, skeletonSource, meshSources, skinningSources);

		//-- now process meshes.
		AABB aabb;
		Meshes meshes;
		Skeleton skeleton;
		processMeshSources(skeletonSource, meshSources, skinningSources, meshes, skeleton, aabb);

		//-- and save them.
		saveMeshes(meshSources, skeleton, meshes, aabb, oData);
	}

	//----------------------------------------------------------------------------------------------
	void collada2animation(const ROData& iData, WOData& oData)
	{
		//-- firstly verify COLLADA document.
		xml_document doc;
		verifyCOLLADA(doc, iData, true);

		//-- gather skinned meshes info.
		MeshesInfo meshesInfo;
		gatherMeshesInfo(doc, meshesInfo);

		//-- gather skeleton source.
		SkeletonSource skeletonSource;
		gatherSkeletonSource(doc, meshesInfo, skeletonSource);

		//-- gather mesh sources.
		MeshSources		meshSources;
		SkinningSources skinningSources;
		gatherMeshSources(doc, meshesInfo, skeletonSource, meshSources, skinningSources);

		//-- gather mesh animation.
		AnimationSource animSource;
		gatherAnimationSource(doc, skeletonSource, animSource);

		//-- now process meshes.
		Skeleton  skeleton;
		Animation animation;
		processAnimationSource(skeletonSource, meshSources, skinningSources, animSource, skeleton, animation);

		//-- and save them.
		saveAnimation(skeleton, animation, oData);
	}

} //-- brUGE