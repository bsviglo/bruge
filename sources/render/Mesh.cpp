#include "render/Mesh.hpp"

#include "render/mesh_formats.hpp"
#include "render/render_system.hpp"
#include "render/IBuffer.h"
#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"

//-- http://pugixml.org/
#include "pugixml/pugixml.hpp"

using namespace brUGE::render;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::os;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//-- ToDo: reconsider.
	uint g_instancingCounter = 0;

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	Mesh::Mesh() : m_instacingID(g_instancingCounter++)
	{

	}

	//----------------------------------------------------------------------------------------------
	Mesh::~Mesh()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool Mesh::load(const ROData& iData, const std::string& name)
	{
		//-- check header.
		{
			StaticMeshFormat::Header iHeader;
			iData.read(iHeader);
			if (std::string(iHeader.m_format.data()) != "static_mesh")
			{
				ERROR_MSG("Failed to load mesh. Most likely it's not a *.mesh format.");
				return false;
			}
		}

		//-- load common info.
		StaticMeshFormat::Info iInfo;
		iData.read(iInfo);

		//-- load mesh bounds.
		m_aabb = AABB(
			vec3f(iInfo.m_aabb[0], iInfo.m_aabb[1], iInfo.m_aabb[2]),
			vec3f(iInfo.m_aabb[3], iInfo.m_aabb[4], iInfo.m_aabb[5])
			);

		//-- allocate sub-meshes info.
		std::vector<SubMesh::Desc> descs(iInfo.m_numSubMeshes);

		//-- iterate over the whole set of sub-meshes.
		for (uint i = 0; i < iInfo.m_numSubMeshes; ++i)
		{
			StaticMeshFormat::SubInfo iSubInfo;
			iData.read(iSubInfo);

			SubMesh::Desc& oDesc = descs[i];

			oDesc.m_streams.resize(iSubInfo.m_numVertexStreams);
			oDesc.m_name		= iSubInfo.m_name;
			oDesc.m_numVertices = iSubInfo.m_numVertices;

			//-- read each individual vertex stream.
			for (uint j = 0; j < iSubInfo.m_numVertexStreams; ++j)
			{
				StaticMeshFormat::VertexStream iStream;
				iData.read(iStream);

				SubMesh::Desc::Stream& oStream = oDesc.m_streams[j];
				
				//-- read data for vertex buffer.
				oStream.m_elemSize = iStream.m_elemSize;
				oStream.m_vertices.resize(iStream.m_elemSize * iSubInfo.m_numVertices);
				iData.readBytes(&oStream.m_vertices[0], iStream.m_elemSize * iSubInfo.m_numVertices);
			}

			//-- read data for index buffer.
			oDesc.m_indices.resize(iSubInfo.m_numIndices);
			iData.readBytes(&oDesc.m_indices[0], sizeof(uint16) * iSubInfo.m_numIndices);
		}

		//-- Now all needed data has been read and we just allocate GPU resources.
		bool success = true;

		m_submeshes.resize(descs.size());
		for (uint i = 0; i < descs.size(); ++i)
		{
			const SubMesh::Desc& desc = descs[i];
			SubMesh&			 sm   = m_submeshes[i];

			//-- create index buffer.
			sm.m_numIndices = desc.m_indices.size();
			sm.m_IB	= rd()->createBuffer(IBuffer::TYPE_INDEX,  &desc.m_indices[0], desc.m_indices.size(), sizeof(uint16));
			success &= sm.m_IB.get() != nullptr;

			//-- iterate over the whole set of streams and create of all them appropriate vertex buffers.
			sm.m_VBs.resize(desc.m_streams.size());
			sm.m_pVBs.resize(desc.m_streams.size());
			for (uint j = 0; j < desc.m_streams.size(); ++j)
			{
				const SubMesh::Desc::Stream& stream = desc.m_streams[j];

				sm.m_VBs[j] = rd()->createBuffer(
					IBuffer::TYPE_VERTEX, &stream.m_vertices[0], desc.m_numVertices, stream.m_elemSize
					);
				sm.m_pVBs[j] = sm.m_VBs[j].get();
				success &= sm.m_VBs[j].get() != nullptr;
			}
		}

		//-- now retrieve material for each sub-mesh.
		{
			//-- load materials lib for this model.
			//-- Note: materials lib may contain more then one material one for each model
			//--	   submesh. Appropriate material selected by sequential number of the mesh.
			std::string material = name + ".material";
			std::vector<std::shared_ptr<PipelineMaterial>> mtllib;
			RODataPtr mData = FileSystem::instance().readFile("resources/" + material);
			if (!mData.get() || !rs().materials().createPipelineMaterials(mtllib, *mData.get()))
			{
				ERROR_MSG("Can't load materials library %s for model.", material.c_str());
				return false;
			}

			if (mtllib.size() < descs.size())
			{
				ERROR_MSG("Not all sub-meshes of mesh %s has its own material.", name.c_str());
				return false;
			}

			for (uint i = 0; i < descs.size(); ++i)
			{
				m_submeshes[i].m_pMaterial = mtllib[i];
			}
		}

		return success;
	}
	
	//----------------------------------------------------------------------------------------------
	uint Mesh::gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const
	{
		RenderOp op;

		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			const SubMesh& sm = m_submeshes[i];

			if (sm.m_pMaterial)
			{
				op.m_material = sm.m_pMaterial->renderFx(rs().shaderPass(pass), instanced);
			}
			else
			{
				op.m_material = sm.m_sMaterial->renderFx();
			}

			op.m_primTopolpgy	= PRIM_TOPOLOGY_TRIANGLE_LIST;
			op.m_indicesCount	= sm.m_numIndices;
			op.m_IB				= sm.m_IB.get();
			op.m_VBs			= &sm.m_pVBs[0];
			op.m_VBCount		= (op.m_material->m_bumped) ? 2 : 1;

			ops.push_back(op);
		}

		return m_submeshes.size();
	}

	//----------------------------------------------------------------------------------------------
	SkinnedMesh::SkinnedMesh()
	{

	}

	//----------------------------------------------------------------------------------------------
	SkinnedMesh::~SkinnedMesh()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::load(const utils::ROData& iData, const std::string& name)
	{
		//-- check header.
		{
			SkinnedMeshFormat::Header iHeader;
			iData.read(iHeader);
			if (std::string(iHeader.m_format) != "skinned_mesh")
			{
				ERROR_MSG("Failed to load mesh. Most likely it's not a *.skinnedmesh format.");
				return false;
			}
		}

		//-- load skeleton info.
		SkinnedMeshFormat::Skeleton::Info skelInfo;
		{
			iData.read(skelInfo);

			//-- read skeleton.
			m_skeleton.resize(skelInfo.m_numJoints);
			for (uint i = 0; i < skelInfo.m_numJoints; ++i)
			{
				SkinnedMeshFormat::Skeleton::Joint iJoint;

				iData.read(iJoint);

				strcpy_s(m_skeleton[i].m_name, iJoint.m_name);
				m_skeleton[i].m_parent = iJoint.m_parent;
			}
		}

		//-- read invert bind pose.
		m_invBindPose.resize(skelInfo.m_numJoints);
		for (uint i = 0; i < skelInfo.m_numJoints; ++i)
		{
			SkinnedMeshFormat::InvBindPose iInvBindPose;

			iData.read(iInvBindPose);

			m_invBindPose[i].set(iInvBindPose.m_matrix);
		}

		//-- load common info.
		SkinnedMeshFormat::Info iInfo;
		iData.read(iInfo);

		//-- load mesh bounds.
		m_aabb = AABB(
			vec3f(iInfo.m_aabb[0], iInfo.m_aabb[1], iInfo.m_aabb[2]),
			vec3f(iInfo.m_aabb[3], iInfo.m_aabb[4], iInfo.m_aabb[5])
			);

		//-- allocate sub-meshes info.
		std::vector<SubMesh::Desc> descs(iInfo.m_numSubMeshes);

		//-- iterate over the whole set of sub-meshes.
		for (uint i = 0; i < iInfo.m_numSubMeshes; ++i)
		{
			SkinnedMeshFormat::SubInfo iSubInfo;
			iData.read(iSubInfo);

			SubMesh::Desc& oDesc = descs[i];

			oDesc.m_streams.resize(iSubInfo.m_numVertexStreams);
			strcpy_s(oDesc.m_name, iSubInfo.m_name);
			oDesc.m_numVertices = iSubInfo.m_numVertices;

			//-- read each individual vertex stream.
			for (uint j = 0; j < iSubInfo.m_numVertexStreams; ++j)
			{
				SkinnedMeshFormat::VertexStream iStream;
				iData.read(iStream);

				SubMesh::Desc::Stream& oStream = oDesc.m_streams[j];

				//-- read data for vertex buffer.
				oStream.m_elemSize = iStream.m_elemSize;
				oStream.m_vertices.resize(iStream.m_elemSize * iSubInfo.m_numVertices);
				iData.readBytes(&oStream.m_vertices[0], iStream.m_elemSize * iSubInfo.m_numVertices);
			}

			//-- read data for index buffer.
			oDesc.m_indices.resize(iSubInfo.m_numIndices);
			iData.readBytes(&oDesc.m_indices[0], sizeof(uint16) * iSubInfo.m_numIndices);
		}

		//-- Now all needed data has been read and we just allocate GPU resources.
		bool success = true;

		m_submeshes.resize(descs.size());
		for (uint i = 0; i < descs.size(); ++i)
		{
			const auto& desc = descs[i];
			auto&		sm   = m_submeshes[i];

			//-- create index buffer.
			sm.m_numIndices = desc.m_indices.size();
			sm.m_IB	= rd()->createBuffer(IBuffer::TYPE_INDEX,  &desc.m_indices[0], desc.m_indices.size(), sizeof(uint16));
			success &= sm.m_IB.get() != nullptr;

			//-- iterate over the whole set of streams and create of all them appropriate vertex buffers.
			sm.m_VBs.resize(desc.m_streams.size());
			sm.m_pVBs.resize(desc.m_streams.size());
			for (uint j = 0; j < desc.m_streams.size(); ++j)
			{
				const auto& stream = desc.m_streams[j];

				sm.m_VBs[j] = rd()->createBuffer(
					IBuffer::TYPE_VERTEX, &stream.m_vertices[0], desc.m_numVertices, stream.m_elemSize
					);
				sm.m_pVBs[j] = sm.m_VBs[j].get();
				success &= sm.m_VBs[j].get() != nullptr;
			}
		}

		//-- now retrieve material for each sub-mesh.
		{
			//-- load materials lib for this model.
			//-- Note: materials lib may contain more then one material one for each model
			//--	   submesh. Appropriate material selected by sequential number of the mesh.
			std::string material = name + ".material";
			std::vector<std::shared_ptr<PipelineMaterial>> mtllib;
			RODataPtr mData = FileSystem::instance().readFile("resources/" + material);
			if (!mData.get() || !rs().materials().createPipelineMaterials(mtllib, *mData.get()))
			{
				ERROR_MSG("Can't load materials library %s for model.", material.c_str());
				return false;
			}

			if (mtllib.size() < descs.size())
			{
				ERROR_MSG("Not all sub-meshes of mesh %s has its own material.", name.c_str());
				return false;
			}

			for (uint i = 0; i < descs.size(); ++i)
			{
				m_submeshes[i].m_pMaterial = mtllib[i];
			}
		}

		return success;
	}
	
	//----------------------------------------------------------------------------------------------
	uint SkinnedMesh::gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const
	{
		RenderOp op;

		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			const SubMesh& sm = m_submeshes[i];

			if (sm.m_pMaterial)
			{
				op.m_material = sm.m_pMaterial->renderFx(rs().shaderPass(pass), instanced);
			}
			else
			{
				op.m_material = sm.m_sMaterial->renderFx();
			}

			op.m_primTopolpgy	= PRIM_TOPOLOGY_TRIANGLE_LIST;
			op.m_indicesCount	= sm.m_numIndices;
			op.m_IB				= sm.m_IB.get();
			op.m_VBs			= &sm.m_pVBs[0];
			op.m_VBCount		= (op.m_material->m_bumped) ? 2 : 1;

			ops.push_back(op);
		}

		return m_submeshes.size();
	}

} // render
} // brUGE