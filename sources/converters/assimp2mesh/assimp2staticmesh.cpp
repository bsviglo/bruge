#include "prerequisites.hpp"
#include "math/math_all.hpp"
#include "utils/Data.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "render/mesh_formats.hpp"

namespace brUGE
{

	using namespace utils;

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
	};

#pragma pack(pop)

	//----------------------------------------------------------------------------------------------
	struct Mesh
	{
		std::string						m_name;
		std::vector<uint16>				m_indices;
		std::vector<Vertex::Common>		m_common;
		std::vector<Vertex::Tangent>	m_tangent;
	};
	typedef std::vector<Mesh> Meshes;

	//--------------------------------------------------------------------------------------------------
	void gatherMeshes(const aiScene& scene, Meshes& oMeshes, AABB& oAABB)
	{
		oMeshes.resize(scene.mNumMeshes);

		for (uint m = 0; m < scene.mNumMeshes; ++m)
		{
			const auto& iMesh = *scene.mMeshes[m];
			auto&		oMesh = oMeshes[m];

			//-- copy name
			oMesh.m_name = iMesh.mName.C_Str();

			//-- copy indices
			oMesh.m_indices.resize(iMesh.mNumFaces * 3);

			for (uint i = 0; i < iMesh.mNumFaces; ++i)
			{
				for (uint j = 0; j < 3; ++j)
				{
					oMesh.m_indices[i * 3 + j] = static_cast<uint16>(iMesh.mFaces[i].mIndices[j]);
				}
			}

			//-- copy vertices for each individual stream
			oMesh.m_common.resize(iMesh.mNumVertices);
			oMesh.m_tangent.resize(iMesh.mNumVertices);

			for (uint i = 0; i < iMesh.mNumVertices; ++i)
			{
				auto& c = oMesh.m_common[i];
				auto& t = oMesh.m_tangent[i];

				c.m_pos			= vec3f(&iMesh.mVertices[i][0]);
				c.m_normal		= vec3f(&iMesh.mNormals[i][0]);
				c.m_uv			= vec2f(&iMesh.mTextureCoords[0][i][0]);
					
				t.m_tangent		= vec3f(&iMesh.mTangents[i][0]);
				t.m_binormal	= vec3f(&iMesh.mBitangents[i][0]);
			}

			//-- calculate bounds for the whole mesh.
			for (const auto& v : oMesh.m_common)
			{
				oAABB.include(v.m_pos);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void saveMeshes(const Meshes& iMeshes, const AABB& aabb, WOData& oData)
	{
		//-- 1. write header
		{
			StaticMeshFormat::Header header;

			header.m_format  = { "static_mesh" };
			header.m_version = 1;

			oData.write(header);
		}

		//-- 2. write mesh info.
		{
			StaticMeshFormat::Info info;

			info.m_format		= { "xyzuvntb" };
			info.m_formatSize	= sizeof(Vertex);
			info.m_numSubMeshes = iMeshes.size();
			info.m_aabb			= { aabb.m_min.x, aabb.m_min.y, aabb.m_min.z, aabb.m_max.x, aabb.m_max.y, aabb.m_max.z };

			oData.write(info);
		}

		//-- 3. write sub-meshes.
		{
			for (const auto& mesh : iMeshes)
			{
				//-- 3.1. write sub-mesh info.
				{
					StaticMeshFormat::SubInfo subInfo;

					//-- copy name and clamp it to the size in SubInfo.n_name
					for (uint c = 0; c < subInfo.m_name.size(); ++c)
						subInfo.m_name[c] = mesh.m_name[c];

					subInfo.m_numIndices		= mesh.m_indices.size();
					subInfo.m_numVertices		= mesh.m_common.size();
					subInfo.m_numVertexStreams = 2;

					oData.write(subInfo);
				}

				//-- 3.3. write each individual vertex stream.

				//-- 3.3.1. write common part of vertices.
				{
					StaticMeshFormat::VertexStream vStream;
					vStream.m_elemSize = sizeof(Vertex::Common);
					oData.write(vStream);

					oData.writeBytes(&mesh.m_common[0], sizeof(Vertex::Common) * mesh.m_common.size());
				}

				//-- 3.3.2. write tangent part of vertices.
				{
					StaticMeshFormat::VertexStream vStream;
					vStream.m_elemSize = sizeof(Vertex::Tangent);
					oData.write(vStream);

					oData.writeBytes(&mesh.m_tangent[0], sizeof(Vertex::Tangent) * mesh.m_tangent.size());
				}

				//-- 3.3. write indices.
				oData.writeBytes(&mesh.m_indices[0], sizeof(uint16) * mesh.m_indices.size());
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void assimp2staticmesh(const aiScene& scene, WOData& oData)
	{
		Meshes meshes;
		AABB aabb;
		gatherMeshes(scene, meshes, aabb);
	
		saveMeshes(meshes, aabb, oData);
	}

} //-- brUGE