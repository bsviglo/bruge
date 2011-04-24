#include "render/Mesh.hpp"

#include "render/render_system.hpp"
#include "render/IBuffer.h"
#include "loader/ResourcesManager.h"

//-- http://pugixml.org/
#include "pugixml/pugixml.hpp"

//-- http://developer.nvidia.com/object/nvtristrip_library.html
#include "nvTriStrip/NvTriStrip.h"

using namespace brUGE::render;
using namespace brUGE::math;
using namespace brUGE::utils;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//----------------------------------------------------------------------------------------------
	void buildTangentBasic(
		const vec3f& pos1, const vec3f& pos2, const vec3f& pos3,
		const vec2f& tex1, const vec2f& tex2, const vec2f& tex3,
		vec3f& tangent,	vec3f& binormal
		)
	{
		vec3f e1 = pos2 - pos1;
		vec3f e2 = pos3 - pos1;

		vec2f et1 = tex2 - tex1;
		vec2f et2 = tex3 - tex1;

		tangent.setZero();
		binormal.setZero();

		float r = et1.y * et2.x - et1.x * et2.y;

		if(r != 0.0f)
		{
			float d = 1.0f / r;
			tangent  = (e2.scale(et1.y) - e1.scale(et2.y)).scale(d);
			binormal = (e2.scale(et1.x) - e1.scale(et2.x)).scale(d);
			tangent.normalize();
			binormal.normalize();
		}
	}

	// http://www.gamedev.ru/code/forum/?id=34454
	//----------------------------------------------------------------------------------------------
	bool optimizeForStrips(
		std::vector<Mesh::Vertex>& outVertices,
		std::vector<Mesh::Tangent>& outTangents,
		std::vector<uint16>& indices,
		const std::vector<Mesh::Vertex>& vertices,
		const std::vector<Mesh::Tangent>& tangents,
		const std::vector<Mesh::Face>& faces
		)
	{
		//::SetStitchStrips(false);

		::PrimitiveGroup *prim_groups;
		::PrimitiveGroup *remapped_groups;
		uint16	prim_groups_cnt;
		uint	indices_cnt;

		bool result = ::GenerateStrips(
			faces[0].index.data,
			static_cast<uint>(faces.size() * 3),
			&prim_groups,
			&prim_groups_cnt
			);

		if (!result || prim_groups_cnt != 1 || prim_groups->type != PT_STRIP)
		{
			ERROR_MSG("NvTriStrip - !GenerateStrips(PT_STRIP) failed");
			delete[] prim_groups;
			return false;
		}

		indices_cnt = prim_groups->numIndices;

		::RemapIndices(prim_groups, 1, static_cast<uint16>(vertices.size()), &remapped_groups);

		if (indices_cnt != remapped_groups->numIndices)
		{
			ERROR_MSG("NvTriStrip - !RemapIndices() failed");
			delete[] remapped_groups;
			delete[] prim_groups;
			return false;
		}

		//--  Validation for corrected NvTriStrip: Incorrect NvTriStrip will probably fail here.
		for (uint i = 0; i < indices_cnt; i++)
		{
			if (prim_groups->indices[i] == 0xffff && remapped_groups->indices[i] != 0xffff)
			{
				ERROR_MSG("NvTriStrip - !RemapIndices() failed");
				delete[] remapped_groups;
				delete[] prim_groups;
				return false;
			}
		}

		indices.resize(indices_cnt);
		memcpy(&indices.front(), remapped_groups->indices, indices_cnt * sizeof(uint16));

		outVertices.resize(vertices.size());
		outTangents.resize(vertices.size());

		for (uint i = 0; i < indices_cnt; ++i)
		{
			uint16 new_index = remapped_groups->indices[i];
			uint16 old_index = prim_groups->indices[i];

			if (old_index == 0xffff)
				continue;

			outVertices[new_index] = vertices[old_index];
			outTangents[new_index] = tangents[old_index];
		}

		delete[] remapped_groups;
		delete[] prim_groups;

		return true;
	}

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	Mesh::Mesh()
	{

	}

	//----------------------------------------------------------------------------------------------
	Mesh::~Mesh()
	{
		for (auto iter = m_submeshes.cbegin(); iter != m_submeshes.cend(); ++iter)
		{
			delete *iter;
		}
	}
	
	//----------------------------------------------------------------------------------------------
	bool Mesh::build()
	{
		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			SubMesh* submesh = m_submeshes[i];

			//-- resize tangents vector.
			submesh->tangents.resize(submesh->vertices.size());

			submesh->buildTangents();
			submesh->buildNormals();
	
			//-- compute AABB.
			const std::vector<Vertex>& vts = submesh->vertices;
			for (uint i = 0; i < vts.size(); ++i)
			{
				m_aabb.include(vts[i].pos);
			}

			if (!submesh->buildBuffers(false))
				return false;
		}
		
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool Mesh::SubMesh::buildBuffers(bool useNVTriStipOptimization/* = false*/)
	{
		if (useNVTriStipOptimization)
		{
			std::vector<Vertex>  oVertices;
			std::vector<Tangent> oTangents;
			std::vector<uint16>  oIndices;

			if (!optimizeForStrips(oVertices, oTangents, oIndices, vertices, tangents, faces))
				return false;

			indicesCount = oIndices.size();
			primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;

			mainVB	  = rd()->createBuffer(IBuffer::TYPE_VERTEX, &oVertices[0], vertices.size(), sizeof(Vertex));
			tangentVB = rd()->createBuffer(IBuffer::TYPE_VERTEX, &oTangents[0], vertices.size(), sizeof(Tangent));
			IB		  = rd()->createBuffer(IBuffer::TYPE_INDEX,  &oIndices[0],  oIndices.size(), sizeof(uint16));
		}
		else
		{
			mainVB	  = rd()->createBuffer(IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(Vertex));
			tangentVB = rd()->createBuffer(IBuffer::TYPE_VERTEX, &tangents[0], vertices.size(),  sizeof(Tangent));
			IB		  = rd()->createBuffer(IBuffer::TYPE_INDEX,  &faces[0],	   faces.size() * 3, sizeof(uint16));

			primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_LIST;
			indicesCount = faces.size() * 3;
		}

		if (!mainVB || !tangentVB || !IB)
		{
			return false;
		}

		return true;
	}

	//------------------------------------------
	void Mesh::SubMesh::buildNormals()
	{
		for (uint i = 0; i < faces.size(); ++i)
		{
			uint16 a = faces[i].index[0];
			uint16 b = faces[i].index[1];
			uint16 c = faces[i].index[2];

			vec3f pos1 = vertices[a].pos;
			vec3f pos2 = vertices[b].pos;
			vec3f pos3 = vertices[c].pos;

			vec3f normal(0.0f, 0.0f, 0.0f);

			//calculate normal vector and normalize it
			normal = (pos2 - pos1).cross(pos3 - pos1);
			normal.normalize();

			//calculate average value of normal vector
			vertices[a].normal += normal;
			vertices[b].normal += normal;
			vertices[c].normal += normal;
		}

		//normalize vertex components
		for(uint i = 0; i < vertices.size(); ++i)
			vertices[i].normal.normalize();
	}


	//----------------------------------------------------------------------------------------------
	void Mesh::SubMesh::buildTangents()
	{
		for (uint i = 0; i < faces.size(); ++i)
		{
			uint16 a = faces[i].index[0];
			uint16 b = faces[i].index[1];
			uint16 c = faces[i].index[2];

			vec3f pos1 = vertices[a].pos;
			vec3f pos2 = vertices[b].pos;
			vec3f pos3 = vertices[c].pos;

			vec2f tex1 = vertices[a].texCoord;
			vec2f tex2 = vertices[b].texCoord;
			vec2f tex3 = vertices[c].texCoord;

			vec3f tangent, binormal;

			buildTangentBasic(pos1, pos2, pos3, tex1, tex2, tex3, tangent, binormal);

			//calculate average value of binormal vector
			tangents[a].binormal += binormal;
			tangents[b].binormal += binormal;
			tangents[c].binormal += binormal;

			//calculate average value of tangent vector
			tangents[a].tangent += tangent;
			tangents[b].tangent += tangent;
			tangents[c].tangent += tangent;
		}

		//normalize vertex components
		for (uint i = 0; i < vertices.size(); ++i)
		{
			tangents[i].binormal.normalize();
			tangents[i].tangent.normalize();
		}
	}

	//----------------------------------------------------------------------------------------------
	uint Mesh::gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const
	{
		RenderOp op;

		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			const Mesh::SubMesh& sm = *m_submeshes[i];

			op.m_primTopolpgy	= PRIM_TOPOLOGY_TRIANGLE_LIST;
			op.m_IB				= &*sm.IB;
			op.m_mainVB			= &*sm.mainVB;
			op.m_tangentVB		= &*sm.tangentVB;
			op.m_indicesCount	= sm.indicesCount;

			if (sm.pMaterial)
			{
				op.m_material = sm.pMaterial->renderFx(rs().shaderPass(pass), instanced);
			}
			else
			{
				op.m_material = sm.sMaterial->renderFx();
			}

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
		for (auto iter = m_submeshes.cbegin(); iter != m_submeshes.cend(); ++iter)
		{
			delete *iter;
		}
	}
	

	//----------------------------------------------------------------------------------------------
/*
	void SkinnedMesh::computeSkeleton(Joint::Transforms& skeleton, bool / *isLocal* /) const
	{
		for (uint i = 0; i < m_joints.size(); ++i)
		{
			Joint::Transform& transf = skeleton[i];
			int				  idx    = m_joints[i].m_parentIdx;

			if (idx != -1)
			{
				const Joint::Transform& parentTransf = skeleton[idx];

				transf.pos    = parentTransf.pos + parentTransf.orient.rotate(transf.pos);
				transf.orient = parentTransf.orient * transf.orient;
			}
		}
	}
*/
	

	//----------------------------------------------------------------------------------------------
/*
	void SkinnedMesh::setOriginSkeleton(Joint::Transforms& skeleton, bool / *isLocal* /) const
	{
		for (uint i = 0; i < m_joints.size(); ++i)
		{
			skeleton[i] = m_joints[i].m_transform;
		}
	}
*/
	
	//----------------------------------------------------------------------------------------------
	uint SkinnedMesh::gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const
	{
		RenderOp op;

		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			const SkinnedMesh::SubMesh& sm = *m_submeshes[i];

			op.m_primTopolpgy	= PRIM_TOPOLOGY_TRIANGLE_LIST;
			op.m_IB				= &*sm.IB;
			op.m_mainVB			= &*sm.mainVB;
			op.m_tangentVB		= &*sm.tangentVB;
			op.m_weightsTB		= &*sm.weightsTB;
			op.m_indicesCount	= sm.indicesCount;
			op.m_material		= sm.material->renderFx(rs().shaderPass(pass), instanced);

			ops.push_back(op);
		}

		return m_submeshes.size();
	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::build()
	{
		Positions positions;

		for (uint i = 0; i < m_submeshes.size(); ++i)
		{
			SubMesh*  submesh = m_submeshes[i];
			
			//-- resize tangents vector.
			submesh->tangents.resize(submesh->vertices.size());
			
			submesh->buildPositions(positions, m_joints);
			submesh->buildTangents(positions);
			submesh->buildNormals(positions);
	
			//-- compute AABB.
			for (uint i = 0; i < positions.size(); ++i)
			{
				m_originAABB.include(positions[i]);
			}

			if (!submesh->buildBuffers())
				return false;
		}
		
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedMesh::SubMesh::buildBuffers()
	{
		mainVB	  = rd()->createBuffer(IBuffer::TYPE_VERTEX, &vertices[0], vertices.size(),  sizeof(Vertex));
		tangentVB = rd()->createBuffer(IBuffer::TYPE_VERTEX, &tangents[0], vertices.size(),  sizeof(Tangent));
		IB		  = rd()->createBuffer(IBuffer::TYPE_INDEX,  &faces[0],	   faces.size() * 3, sizeof(uint16));

		//-- Note: Be careful with texture buffer, because it has to be 16 bytes aligned.
		{
			std::vector<GPUWeight> tmp(weights.size());
			for (uint i = 0; i < weights.size(); ++i)
			{
				tmp[i].joint.x = weights[i].joint;
				tmp[i].weight  = vec4f(weights[i].pos, weights[i].weight);
			}

			weightsTB = rd()->createBuffer(IBuffer::TYPE_TEXTURE, &tmp[0],
				weights.size() * 2, sizeof(vec4f)
				);
		}

		primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_LIST;
		indicesCount = faces.size() * 3;

		if (!mainVB || !tangentVB || !IB || !weightsTB)
		{
			return false;
		}

		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	void SkinnedMesh::SubMesh::buildPositions(Positions& positions, const Joints& joints)
	{
		positions.resize(vertices.size());

		for (uint i = 0; i < vertices.size(); ++i)
		{
			const Vertex& v   = vertices[i];
			vec3f&		  pos = positions[i];

			for (uint k = 0; k < v.weightCount; ++k)
			{
				const Weight& weight = weights[v.weightIdx + k];
				const Joint&  joint  = joints [weight.joint];

				//-- transform weight.pos by bone with weight
				pos += joint.transform(weight.pos).scale(weight.weight);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedMesh::SubMesh::buildNormals(const Positions& positions)
	{
		for (uint i = 0; i < faces.size(); ++i)
		{
			uint16 a = faces[i].index[0];
			uint16 b = faces[i].index[1];
			uint16 c = faces[i].index[2];

			vec3f pos1 = positions[a];
			vec3f pos2 = positions[b];
			vec3f pos3 = positions[c];

			vec3f normal(0.0f, 0.0f, 0.0f);

			//calculate normal vector and normalize it
			normal = (pos2 - pos1).cross(pos3 - pos1);
			normal.normalize();

			//calculate average value of normal vector
			vertices[a].normal += normal;
			vertices[b].normal += normal;
			vertices[c].normal += normal;
		}

		//normalize vertex components
		for(uint i = 0; i < vertices.size(); ++i)
			vertices[i].normal.normalize();
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedMesh::SubMesh::buildTangents(const Positions& positions)
	{
		for (uint i = 0; i < faces.size(); ++i)
		{
			uint16 a = faces[i].index[0];
			uint16 b = faces[i].index[1];
			uint16 c = faces[i].index[2];

			vec3f pos1 = positions[a];
			vec3f pos2 = positions[b];
			vec3f pos3 = positions[c];

			vec2f tex1 = vertices[a].texCoord;
			vec2f tex2 = vertices[b].texCoord;
			vec2f tex3 = vertices[c].texCoord;

			vec3f tangent, binormal;

			buildTangentBasic(pos1, pos2, pos3, tex1, tex2, tex3, tangent, binormal);

			//calculate average value of binormal vector
			tangents[a].binormal += binormal;
			tangents[b].binormal += binormal;
			tangents[c].binormal += binormal;

			//calculate average value of tangent vector
			tangents[a].tangent += tangent;
			tangents[b].tangent += tangent;
			tangents[c].tangent += tangent;
		}

		//normalize vertex components
		for (uint i = 0; i < vertices.size(); ++i)
		{
			tangents[i].binormal.normalize();
			tangents[i].tangent.normalize();
		}
	}


	//-- auxiliary function helps us to create BSP from mesh.
	//----------------------------------------------------------------------------------------------
	Ptr<BSPTree> createBSPFromMesh(const Ptr<Mesh>& /*mesh*/)
	{
		return NULL;

		/*
		Ptr<BSPTree> result = new BSPTree();

		//-- calculate collision data.
		for (auto iter = mesh->begin(); iter != mesh->end(); ++iter)
		{
			Mesh::SubMesh& submesh = *(*iter);

			for (uint i = 0; i < submesh.faces.size(); ++i)
			{
				const vec3us&					 idx  = submesh.faces[i].index;
				const std::vector<Mesh::Vertex>& vrts = submesh.vertices;

				result->addTriangle(vrts[idx.x].pos, vrts[idx.y].pos, vrts[idx.z].pos);
			}
		}
		result->build();

		return result;
		*/
	}
	
} // render
} // brUGE