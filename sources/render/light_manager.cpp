#include "light_manager.hpp"
#include "Mesh.hpp"
#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"

using namespace brUGE::os;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//-- template version of the add*Light function to minimize overhead of the duplicating code.
	//----------------------------------------------------------------------------------------------
	template<typename T>
	Handle addLight(std::vector<std::pair<bool, T>>& container, const T& light)
	{
		for (uint i = 0; i != container.size(); ++i)
		{
			if (!container[i].first)
			{
				container[i] = std::make_pair(true, light);
				return i;
			}
		}

		container.push_back(std::make_pair(true, light));
		return container.size() - 1;
	}

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	LightsManager::LightsManager() : m_pVB(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	LightsManager::~LightsManager()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool LightsManager::init()
	{
		//-- setup full-screen quad.
		{
			VertexXYZUV vertices[4];

			//-- left-top
			vertices[0].m_pos.set(-1.f, 1.f, 0.0f);
			vertices[0].m_tc.set (0.f, 0.f);

			//-- left-bottom		
			vertices[1].m_pos.set(-1.f, -1.f, 0.0f);
			vertices[1].m_tc.set (0.f, 1.f);

			//-- right-top
			vertices[2].m_pos.set(1.f, 1.f, 0.0f);
			vertices[2].m_tc.set (1.f, 0.f);

			//-- right-bottom
			vertices[3].m_pos.set(1.f, -1.f, 0.0f);
			vertices[3].m_tc.set (1.f, 1.f);

			if (!(m_fsQuadVB = rd()->createBuffer(IBuffer::TYPE_VERTEX, vertices, 4, sizeof(VertexXYZUV))))
				return false;

			m_pVB = m_fsQuadVB.get();
		}

		//-- load unit cube.
		{
			m_unitCube = ResourcesManager::instance().loadMesh("system/meshes/box.mesh");

			if (!m_unitCube)
			{
				ERROR_MSG("Can't load system unit cube mesh.");
				return false;
			}
		}

		//-- load material.
		{
			std::vector<std::shared_ptr<Material>> mtllib;
			RODataPtr file = FileSystem::instance().readFile("resources/materials/lights.mtl");
			if (!file || !rs().materials().createMaterials(mtllib, *file))
			{
				return false;
			}

			m_dirLightMaterial	 = mtllib[0];
			//m_pointLightMaterial = mtllib[1];
			//m_spotLightMaterial	 = mtllib[2];
		}

		//-- create geometry instancing buffers.
		{
			m_dirLightsTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 16 * sizeof(GPUDirLight) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			m_pointLightsTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 128 * sizeof(GPUPointLight) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			m_spotLightsTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 32 * sizeof(GPUSpotLight) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			if (!m_dirLightsTB || !m_pointLightsTB || !m_spotLightsTB)
			{
				ERROR_MSG("Can't create texture buffers.");
				return false;
			}
		}


		//-- create rops for drawing.
		{
			RenderOp op;
			op.m_indicesCount = 4;
			op.m_primTopolpgy = PRIM_TOPOLOGY_TRIANGLE_STRIP;
			op.m_VBs		  = &m_pVB;
			op.m_VBCount	  = 1;
			op.m_instanceTB	  = m_dirLightsTB.get();
			op.m_material	  = m_dirLightMaterial->renderFx();
			op.m_instanceSize = sizeof(GPUDirLight);

			m_ROPs.push_back(op);

			//RenderOps rops;
			//m_unitCube->gatherRenderOps(rops);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle LightsManager::addDirLight(const DirectionLight& light)
	{
		return addLight(m_dirLights, light);
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::updateDirLight(Handle id, const DirectionLight& light)
	{
		m_dirLights[id].second = light;
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::delDirLight(Handle id)
	{
		m_dirLights[id].first = false;
	}

	//----------------------------------------------------------------------------------------------
	const DirectionLight& LightsManager::getDirLight(Handle id)
	{
		return m_dirLights[id].second;
	}

	//----------------------------------------------------------------------------------------------
	Handle LightsManager::addPointLight(const PointLight& light)
	{
		return addLight(m_pointLights, light);
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::updatePointLight(Handle id, const PointLight& light)
	{
		m_pointLights[id].second = light;
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::delPointLight(Handle id)
	{
		m_pointLights[id].first = false;
	}

	//----------------------------------------------------------------------------------------------
	const PointLight& LightsManager::getPointLight(Handle id)
	{
		return m_pointLights[id].second;
	}

	//----------------------------------------------------------------------------------------------
	Handle LightsManager::addSpotLight(const SpotLight& light)
	{
		return addLight(m_spotLights, light);
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::updateSpotLight(Handle id, const SpotLight& light)
	{
		m_spotLights[id].second = light;
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::delSpotLight(Handle id)
	{
		m_spotLights[id].first = false;
	}

	//----------------------------------------------------------------------------------------------
	const SpotLight& LightsManager::getSpotLight(Handle id)
	{
		return m_spotLights[id].second;
	}

	//----------------------------------------------------------------------------------------------
	void LightsManager::update(float /*dt*/)
	{
		//-- update direction lights.
		m_gpuDirLights.clear();

		for (auto i = m_dirLights.begin(); i != m_dirLights.end(); ++i)
		{
			if (i->first)
			{
				m_gpuDirLights.push_back(GPUDirLight(i->second));
			}
		}

		//-- ToDo: update another light's types.
	}

	//----------------------------------------------------------------------------------------------
	uint LightsManager::gatherROPs(RenderOps& ops) const
	{
		uint count = 0;

		//-- setup direction lights ROP.
		if (!m_gpuDirLights.empty())
		{
			ops.push_back(m_ROPs[0]);
			ops.back().m_instanceCount = m_gpuDirLights.size();
			ops.back().m_instanceData  = &m_gpuDirLights.front();

			++count;
		}

		//-- ToDo: setup another light's types.

		return count;
	}

} //-- render
} //-- brUGE