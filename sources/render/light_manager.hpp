#pragma once

#include "prerequisites.hpp"
#include "engine/ISystem.hpp"
#include "engine/IComponent.hpp"
#include "render_common.h"
#include "Color.h"
#include "materials.hpp"
#include "vertex_format.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

namespace brUGE
{
namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	class DirectionalLightComponent : public IComponent
	{
	public:
		struct Parameters
		{
			Color m_color;
		};

	public:
		DirectionalLightComponent(::Handle owner) : IComponent(owner) { }
		virtual ~DirectionalLightComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		::Handle			m_light;
		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class SpotLightComponent : public IComponent
	{
	public:
		struct Parameters
		{
			vec2f m_intoutRadius;
			Color m_color;
		};

	public:
		SpotLightComponent(::Handle owner) : IComponent(owner) { }
		virtual ~SpotLightComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		::Handle			m_light;
		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class OmniLightComponent : public IComponent
	{
	public:
		struct Parameters
		{
			vec2f m_inoutCosAngle;
			vec2f m_startEndFading;
			Color m_color;
		};

	public:
		OmniLightComponent(::Handle owner) : IComponent(owner) { }
		virtual ~OmniLightComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		::Handle			m_light;
		static const TypeID	m_typeID;
	};

	//------------------------------------------------------------------------------------------------------------------
	class LightSystem : public IRenderSystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World() { }
			virtual ~World() override { }


			virtual bool				init() override;

			virtual void				activate() override;
			virtual void				deactivate() override;

			virtual IComponent::Handle	createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::Handle	createComponent(Universe::World& world, Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::Handle	cloneComponent (Universe::World& world, Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual bool				removeComponent(Universe::World& world, Handle gameObj, IComponent::Handle component) override;

		private:
			std::vector<std::unique_ptr<DirectionalLightComponent>> m_directLightComponents;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		public:
			Context() { }
			virtual ~Context() override { }

			virtual bool init(ISystem* system, IWorld* world) override;
		};

	public:

		virtual bool checkRequiredComponents(Handle handle) const override
		{ 
			GameObject* gameObj = nullptr;

			return	gameObj->hasComponentByType(IComponent::TYPE_TRANSFORM) && (
					gameObj->hasComponentByType(IComponent::TYPE_DIRECTIONAL_LIGHT) ||
					gameObj->hasComponentByType(IComponent::TYPE_SPOT_LIGHT) ||
					gameObj->hasComponentByType(IComponent::TYPE_OMNI_LIGHT)
				);
		}

	private:

	};


	class Mesh;

	//------------------------------------------------------------------------------------------------------------------
	struct DirectionLight
	{
		vec3f m_dir;
		Color m_color;
	};

	//----------------------------------------------------------------------------------------------
	struct PointLight
	{
		vec3f m_pos;
		vec2f m_intoutRadius;
		Color m_color;
	};

	//----------------------------------------------------------------------------------------------
	struct SpotLight
	{
		vec3f m_pos;
		vec3f m_dir;
		vec2f m_inoutCosAngle;
		vec2f m_startEndFading;
		Color m_color;
	};


	//-- Controls life time of the all light at the scene. Provides simple interface to access,
	//-- modify, add and delete any light at the scene by its descriptor.
	//-- Does culling lights against view frustum and performs account of the impact every light
	//-- in the final rendered picture.
	//----------------------------------------------------------------------------------------------
	class LightsManager
	{
	public:
		LightsManager();
		~LightsManager();

		bool					init();
		void					update(float dt);
		uint					gatherROPs(RenderOps& ops) const;

		Handle					addDirLight		(const DirectionLight& light);
		void					updateDirLight	(Handle id, const DirectionLight& light);
		void					delDirLight		(Handle id);
		const DirectionLight&	getDirLight		(Handle id);

		Handle					addPointLight	(const PointLight& light);
		void					updatePointLight(Handle id, const PointLight& light);	
		void					delPointLight	(Handle id);
		const PointLight&		getPointLight	(Handle id);

		Handle					addSpotLight	(const SpotLight& light);
		void					updateSpotLight	(Handle id, const SpotLight& light);
		void					delSpotLight	(Handle id);
		const SpotLight&		getSpotLight	(Handle id);

	private:
		std::vector<std::pair<bool, DirectionLight>>	m_dirLights;
		std::vector<std::pair<bool, PointLight>>		m_pointLights;
		std::vector<std::pair<bool, SpotLight>>			m_spotLights;

		//------------------------------------------------------------------------------------------
		struct GPUDirLight
		{
			GPUDirLight(const DirectionLight& light)
			{
				m_dir	= light.m_dir.toVec4();
				m_color = light.m_color.toVec4();
			}

			vec4f m_dir;	//-- .w - padding
			vec4f m_color;
		};

		//------------------------------------------------------------------------------------------
		struct GPUPointLight
		{
			GPUPointLight(const PointLight& light)
			{
				m_pos			= light.m_pos.toVec4();
				m_inoutRadius	= light.m_intoutRadius.toVec4();
				m_color			= light.m_color.toVec4();
			}

			vec4f m_pos;		 //-- .w  - padding	
			vec4f m_inoutRadius; //-- .zw - padding
			vec4f m_color;
		};

		//------------------------------------------------------------------------------------------
		struct GPUSpotLight
		{
			GPUSpotLight(const SpotLight& light)
			{
				m_pos	   = light.m_pos.toVec4();
				m_dir	   = light.m_dir.toVec4();
				m_fading.x = light.m_inoutCosAngle.x;
				m_fading.y = light.m_inoutCosAngle.y;
				m_fading.z = light.m_startEndFading.x;
				m_fading.w = light.m_startEndFading.y;
				m_color	   = light.m_color.toVec4();
			}

			vec4f m_pos;
			vec4f m_dir;
			vec4f m_fading;
			vec4f m_color;
		};

		std::vector<GPUDirLight>	m_gpuDirLights;
		std::vector<GPUPointLight>	m_gpuPointLights;
		std::vector<GPUSpotLight>	m_gpuSpotLights;

		std::shared_ptr<IBuffer>	m_dirLightsTB;
		std::shared_ptr<IBuffer>	m_pointLightsTB;
		std::shared_ptr<IBuffer>	m_spotLightsTB;
		std::shared_ptr<Mesh>		m_unitCube;
		std::shared_ptr<IBuffer>	m_fsQuadVB;
		IBuffer*					m_pVB;
		std::shared_ptr<Material>	m_dirLightMaterial;
		std::shared_ptr<Material>	m_spotLightMaterial;
		std::shared_ptr<Material>	m_pointLightMaterial;
		RenderOps					m_ROPs;
	};

} //-- render
} //-- brUGE