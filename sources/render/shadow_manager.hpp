#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "materials.hpp"
#include "vertex_format.hpp"
#include <memory>

namespace brUGE
{
	class Camera;

namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	class CSMShadowComponent : public IComponent
	{
	public:
		struct Parameters
		{
			Parameters() : m_autoSplitSheme(true), m_shadowMapRes(2048), m_splitShemeLambda(0.85f), m_bias(1.0f), m_slopeScaleBias(4.0f), m_splitCount(4) { }

			bool	m_autoSplitSheme;
			uint	m_shadowMapRes;
			float	m_splitShemeLambda;
			float	m_bias;
			float	m_slopeScaleBias;
			uint	m_splitCount;
		};

	public:
		CSMShadowComponent(::Handle owner) : IComponent(owner) { }
		virtual ~CSMShadowComponent() override { }

		static TypeID		typeID() { return m_typeID; }
		const Parameters&	params() const { return m_params; }

	private:
		Parameters			m_params;
		::Handle			m_instance;
		static const TypeID	m_typeID;
	};


	//------------------------------------------------------------------------------------------------------------------
	class ShadowSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public IWorld
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
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public IContext
		{
		public:
			virtual ~Context() override { }

			virtual bool init() override;
		private:
			RenderOps									m_rops;
			std::unordered_map<ISystem::TypeID, Handle>	m_contexts;

			//-- information for rendering.
			bool										m_autoSplitSheme;
			float										m_splitShemeLambda;
			vec2f										m_cameraFarNearDist;
			uint										m_splitCount;
			std::shared_ptr<ITexture>					m_shadowMaps;
			std::vector<RenderCamera>					m_shadowCameras;
			std::vector<vec4ui>							m_shadowViewPorts;
			std::vector<float>							m_splitPlanes;
		};

	public:
		ShadowSystem() { }
		virtual ~ShadowSystem() { }

		virtual bool		init() override;

		//-- update the global state of the world
		virtual void		update(Handle world, const DeltaTime& dt) const override;

		//-- perform work while the world is the constant state. Here we may have multiple Context are working
		//-- separately (even different threads) on the same constant world.
		virtual void		process(Handle context) const override;

		//--
		virtual Handle		createWorld(const pugi::xml_node& cfg = pugi::xml_node()) override;
		virtual void		removeWorld(Handle handle) override;

		virtual Handle		createContext(Handle world) override;
		virtual void		removeContext(Handle handle) override;

	private:
	};


	//-- ToDo: Legacy

	class  LightsManager;
	class  MeshManager;
	struct DirectionLight;

	//-- Class is responsible for calculating different kinds of shadows and displaying it on the
	//-- screen. For now it uses only direction light shadows, but in the near future it will be able
	//-- to cast shadows from the point light and spot light.
	//-- ToDo:
	//----------------------------------------------------------------------------------------------
	class ShadowManager : public NonCopyable
	{
	public:
		ShadowManager();
		~ShadowManager();

		bool init();
		void update(float dt);
		
		void castShadows(const RenderCamera& cam, LightsManager& lightManager, MeshManager& meshManager);
		void receiveShadows(const RenderCamera* rCam);

	private:
		uint						m_shadowMapRes;
		std::shared_ptr<IBuffer>	m_fsQuadVB;
		IBuffer*					m_pVB;
		std::shared_ptr<Material>	m_shadowResolveMaterial;
		std::shared_ptr<Material>	m_shadowBlurMaterial;
		SamplerStateID				m_shadowMapSml;
		std::shared_ptr<ITexture>	m_noiseMap;
		SamplerStateID				m_noiseMapSml;
		std::shared_ptr<ITexture>	m_blurMap;
		SamplerStateID				m_blurMapSml;
		RenderOps					m_receiveROPs;
		RenderOps					m_castROPs;
		RenderOps					m_blurROPs;

		//-- bias parameters.
		float m_bias;
		float m_slopeScaleBias;

		//-- for every split.
		bool						m_autoSplitSheme;
		float						m_splitShemeLambda;
		vec2f						m_cameraFarNearDist;
		uint						m_splitCount;
		std::shared_ptr<ITexture>	m_shadowMaps;
		std::vector<RenderCamera>	m_shadowCameras;
		std::vector<vec4ui>			m_shadowViewPorts;
		std::vector<float>			m_splitPlanes;

	private:

		//-- UI represents visual acces to the many configuration parameters of shadow manager.
		//-- It does that throught direct access to the ShadowManager class.
		//------------------------------------------------------------------------------------------
		class UI : public NonCopyable
		{
		public:
			UI(ShadowManager& sm);
			~UI();

			void update();

		private:
			int			   m_scroll;
			ShadowManager& m_self;
		};
		typedef std::unique_ptr<UI> UIPtr;
		friend class UI;

		//-- optional UI interface.
		//-- Note: It was designed to be as much non-invasiv as possible.
		bool  m_uiEnabled;
		UIPtr m_ui;
	};

} //-- render
} //-- brUGE