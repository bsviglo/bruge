#include "decal_manager.hpp"
#include "game_world.hpp"
#include "os/FileSystem.h"
#include "loader/ResourcesManager.h"

using namespace brUGE::os;
using namespace brUGE::math;
using namespace brUGE::utils;

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	DecalManager::DecalManager()
		: m_updateStatic(false), m_updateDynamic(false)
	{

	}

	//----------------------------------------------------------------------------------------------
	DecalManager::~DecalManager()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool DecalManager::init()
	{
		//-- create geometry instancing buffers.
		{
			m_staticTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 1024 * sizeof(GPUDecal) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			m_dynamicTB = rd()->createBuffer(
				IBuffer::TYPE_TEXTURE, NULL, 1024 * sizeof(GPUDecal) / sizeof(vec4f),
				sizeof(vec4f), IBuffer::USAGE_DYNAMIC, IBuffer::CPU_ACCESS_WRITE
				);

			if (!m_staticTB || !m_dynamicTB)
			{
				ERROR_MSG("Can't create texture buffers.");
				return false;
			}
		}

		//-- load decals textures.
		std::unique_ptr<TextureProperty> prop;
		{
			auto tex = ResourcesManager::instance().loadTexture("textures/decal.dds");

			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_TRILINEAR_ANISO;
			sDesc.wrapR			= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.wrapS			= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.wrapT			= SamplerStateDesc::ADRESS_MODE_BORDER;
			sDesc.maxAnisotropy = 16;
			sDesc.borderColour	= Color(0,0,0,0);

			SamplerStateID stateS = rd()->createSamplerState(sDesc);

			prop.reset(new TextureProperty(tex, stateS));
		}

		//-- load decals material.
		{
			RODataPtr file = FileSystem::instance().readFile("resources/materials/decals.mtl");
			if (!file || !(m_material = rs().materials().createMaterial(*file)))
			{
				return false;
			}

			m_material->addProperty("diffuse", prop.release());
		}

		//-- load standard system unit cube mesh.
		{
			m_unitCube = ResourcesManager::instance().loadMesh("system/meshes/box.mesh");

			if (!m_unitCube)
			{
				ERROR_MSG("Can't load system unit cube mesh.");
				return false;
			}
			
			//-- create rops for drawing.
			RenderOps rops;
			//-- ToDo: reconsider.
			m_unitCube->gatherROPs(RenderSystem::PASS_Z_ONLY, false, rops);
			{
				RenderOp& op  = rops[0];

				//-- setup common params.
				op.m_material     = m_material->renderFx();
				op.m_instanceSize = sizeof(GPUDecal);
				
				//-- create ROP for static decals.
				op.m_instanceTB = m_staticTB.get();
				m_ROPs.push_back(op);

				//-- create ROP for dynamic decals.
				op.m_instanceTB = m_dynamicTB.get();
				m_ROPs.push_back(op);
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void DecalManager::update(float /*dt*/)
	{
		//-- 1. update static decals.
		if (m_updateStatic)
		{
			m_staticDecalsGPU.clear();
			m_staticDecalsGPU.resize(m_staticDecalDescs.size());

			for (uint i = 0; i < m_staticDecalDescs.size(); ++i)
			{
				const DecalDesc& desc = m_staticDecalDescs[i];
				GPUDecal& gpu		  = m_staticDecalsGPU[i];

				gpu.m_dir   = desc.m_dir.toVec4();
				gpu.m_pos   = desc.m_pos.toVec4();
				gpu.m_up    = desc.m_up.toVec4();
				gpu.m_scale = desc.m_scale.toVec4();
			}
		}

		//-- 2. update dynamic decals.
		if (m_updateDynamic)
		{
			m_dynamicDecalsGPU.clear();
			m_dynamicDecalsGPU.resize(m_dynamicDecalDescs.size());

			for (uint i = 0; i < m_dynamicDecalDescs.size(); ++i)
			{
				const DynamicDecalDesc& desc = m_dynamicDecalDescs[i];
				GPUDecal& gpu				 = m_dynamicDecalsGPU[i];
				const mat4f& world			 = desc.second->matrix();

				gpu.m_dir   = world.applyToVector(desc.first.m_dir).toVec4();
				gpu.m_pos   = world.applyToPoint(desc.first.m_pos).toVec4();
				gpu.m_up    = world.applyToVector(desc.first.m_up).toVec4();
				gpu.m_scale = desc.first.m_scale.toVec4();
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	uint DecalManager::gatherRenderOps(RenderOps& ops) const
	{
		uint count = 0;

		//-- setup static decals ROP.
		if (!m_staticDecalsGPU.empty())
		{
			ops.push_back(m_ROPs[0]);
			ops.back().m_instanceCount = m_staticDecalsGPU.size();
			ops.back().m_instanceData  = &m_staticDecalsGPU.front();

			++count;
		}

		//-- setup dynamic decals ROP.
		if (!m_dynamicDecalsGPU.empty())
		{
			ops.push_back(m_ROPs[1]);
			ops.back().m_instanceCount = m_dynamicDecalsGPU.size();
			ops.back().m_instanceData  = &m_dynamicDecalsGPU.front();

			++count;
		}

		return count;
	}

	//----------------------------------------------------------------------------------------------
	void DecalManager::addStaticDecal(const mat4f& orient, const vec3f& scale)
	{
		m_updateStatic = true;

		vec3f dir = orient.applyToUnitAxis(2);
		vec3f up  = orient.applyToUnitAxis(1);
		vec3f pos = orient.applyToOrigin();

		DecalDesc decal(dir, pos, up, scale);
		m_staticDecalDescs.push_back(decal);
	}

	//----------------------------------------------------------------------------------------------
	void DecalManager::addDynamicDecal(const mat4f& orient, const vec3f& scale, const Node* node)
	{
		m_updateDynamic = true;

		vec3f dir = orient.applyToUnitAxis(2);
		vec3f up  = orient.applyToUnitAxis(1);
		vec3f pos = orient.applyToOrigin();

		DecalDesc decal(dir, pos, up, scale);
		m_dynamicDecalDescs.push_back(std::make_pair(decal, node));
	}

} //-- render
} //-- brUGE