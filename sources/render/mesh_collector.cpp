#include "mesh_collector.hpp"
#include "mesh_manager.hpp"
#include "scene/game_world.hpp"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	MeshCollector::MeshCollector()
	{
		m_instances.reserve(25);
	}

	//----------------------------------------------------------------------------------------------
	MeshCollector::~MeshCollector()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool MeshCollector::init()
	{
		//-- create geometry instancing buffer.
		{
			m_instanceTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 128 * sizeof(mat4f) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			if (!m_instanceTB)
			{
				ERROR_MSG("Can't create texture buffers.");
				return false;
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void MeshCollector::begin(RenderSystem::EPassType pass)
	{
		m_pass = pass;

		for (auto iter = m_instances.begin(); iter != m_instances.end(); ++iter)
			iter->m_buffer.clear();
	}

	//----------------------------------------------------------------------------------------------
	bool MeshCollector::addMeshInstance(const MeshInstance& instance)
	{
		//-- ToDo: reconsider.

		if (!instance.m_mesh || instance.m_mesh->instancingID() == -1)
			return false;

		int id = instance.m_mesh->instancingID();

		if (id >= static_cast<int>(m_instances.size()))
		{
			m_instances.resize(id + 1);
		}
	
		Instance& inst = m_instances[id];

		if (!inst.m_mesh)
			inst.m_mesh = instance.m_mesh;

		inst.m_buffer.push_back(instance.m_transform->m_worldMat);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void MeshCollector::end()
	{
		//-- pass
	}

	//----------------------------------------------------------------------------------------------
	uint MeshCollector::gatherROPs(RenderOps& rops)
	{
		uint totalCount = 0;

		for (auto iter = m_instances.begin(); iter != m_instances.end(); ++iter)
		{
			if (!iter->m_buffer.empty())
			{
				uint count = iter->m_mesh->gatherROPs(m_pass, true, rops);

				for (uint i = rops.size() - count; i < rops.size(); ++i)
				{
					RenderOp& rop = rops[i];
					rop.m_worldMat		= nullptr;
					rop.m_instanceTB    = m_instanceTB.get();
					rop.m_instanceData	= &iter->m_buffer[0];
					rop.m_instanceSize  = sizeof(mat4f);
					rop.m_instanceCount = iter->m_buffer.size();
				}

				totalCount += count;
			}
		}

		return totalCount;
	}

} //-- render
} //-- brUGE