#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "render/Mesh.hpp"

#include <unordered_map>
#include <vector>

namespace brUGE
{
	struct Transform;

namespace render
{

	//------------------------------------------------------------------------------------------------------------------
	class AnimationControllerComponent : public IComponent
	{
	public:
		AnimationControllerComponent(Handle owner) : IComponent(owner), m_animCtrl(animCtrl) { }
		virtual ~AnimationControllerComponent() override { }

	private:
		Handle m_animCtrl;
	};

	//------------------------------------------------------------------------------------------------------------------
	class AnimationSystem : public ISystem
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		class World : public ISystem::IComponentWorld
		{
		public:
			World() { }
			virtual ~World() override { }

			virtual bool			init() override;

			virtual void			activate() override;
			virtual void			deactivate() override;

			virtual IComponent::ID	createComponent(SceneSystem::Scene& scene, Handle gameObj, IComponent::TypeID typeID) override;
			virtual IComponent::ID	createComponent(SceneSystem::Scene& scene, Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg) override;
			virtual IComponent::ID	cloneComponent (SceneSystem::Scene& scene, Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID) override;
			virtual bool			removeComponent(SceneSystem::Scene& scene, Handle gameObj, IComponent::ID component) override;

		private:
			std::vector<std::unique_ptr<AnimationController>>				m_animCtrls;
			std::unordered_map<std::string, std::shared_ptr<Animation>>		m_animations;
			std::vector<AnimationController*>								m_activeAnimCtrls;
			AnimationBlender												m_animBlender;

			std::vector<std::unique_ptr<AnimationControllerComponent>>		m_components;
		};

		//--------------------------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		public:


		};

	public:

		virtual bool checkRequiredComponents(Handle handle) const override
		{ 
			GameObject* gameObj = nullptr;

			return	gameObj->hasComponentByType(IComponent::TYPE_TRANSFORM) &&
					gameObj->hasComponentByType(IComponent::TYPE_SKINNED_MESH) &&
					gameObj->hasComponentByType(IComponent::TYPE_ANIMATION_CONTROLLER);
		}

	private:
		std::vector<std::unique_ptr<World>>		m_worlds;
		std::vector<std::unique_ptr<Context>>	m_contexts;
	};



	struct MeshInstance;
	

	//-- For each particular animation we have only one instance of this class. It contains only
	//-- static information of animation (multi threading friendly)
	//------------------------------------------------------------------------------------------------------------------
	class Animation
	{
	public:

		//------------------------------------------------------------------------------------------
		struct Time
		{
			Time() : m_1st(0), m_2nd(0), m_blend(0.0f), m_time(0.0f) { }

			uint16 m_1st;
			uint16 m_2nd;
			float  m_blend;
			float  m_time;
		};

	public:
		Animation();
		virtual ~Animation();

		bool	load(const utils::ROData& data);
		void	tick(float dt, Time& oTime, bool looped) const;
		void	goTo(uint frame, Time& oTime) const;

		//-- update bounds and matrix palette.
		void	updateBounds(AABB& oBound, const Time& time) const;
		void	updatePalette(TransformPalette& oPalette, const Time& time, const Skeleton& skeleton) const;

		uint	numJoints() const { return m_numJoints; }
		uint	numFrames() const { return m_numFrames; }

	private:
		void updateJoints(TransformPalette& oPalette, uint _1st, uint _2nd, float blend) const;
		void buildAbsoluteTransforms(TransformPalette& oPalette, const Skeleton& skeleton) const;

	private:
		uint							m_numJoints;
		uint							m_numFrames;
		uint							m_frameRate;
		std::vector<float>				m_blendAlphaMask;
		std::vector<TransformPalette>	m_frames;
		std::vector<AABB>				m_bounds;
	};

	//-- Represents one particular layer of an animation.
	//------------------------------------------------------------------------------------------------------------------
	struct AnimLayer
	{
		AnimLayer() : m_looped(false), m_paused(false), m_blend(0.0f) { }

		bool						m_looped;
		bool						m_paused;
		float						m_blend;
		Animation::Time				m_time;
		std::shared_ptr<Animation>	m_anim;
	};
	typedef std::vector<AnimLayer> AnimLayers;

	//-- Consists of the arbitrary number of the animation layers. The all layers are blended 
	//-- together and make the final animation of the mesh. Each layer may be configured with the
	//-- own list of options.
	//------------------------------------------------------------------------------------------------------------------
	struct AnimationController
	{
		struct Desc
		{
			Desc() : m_transform(nullptr), m_meshInst(nullptr) { }

			Transform*	   m_transform;
			MeshInstance*  m_meshInst;
		};

		AnimationController(Desc& desc)
			:	m_transform(desc.m_transform), m_meshInst(desc.m_meshInst),	m_wantsWorldPalette(true), m_physicsDriven(false) { }

	public:
		//-- do we need to calculate world space palette.
		bool				m_wantsWorldPalette;
		//-- do this controller driven by physics. In this case stop animating it and use world transform given by the physics.
		bool				m_physicsDriven;
		AnimLayers			m_animLayers;
		Transform*			m_transform;
		MeshInstance*		m_meshInst;
		TransformPalette	m_tranformPalette;
	};


	//-- ToDo: Maybe it will be better to create one AnimationBlender per skeleton.
	//-- Blends two or more animation together and produce a new blended animation on the output.
	//------------------------------------------------------------------------------------------------------------------
	class AnimationBlender
	{
	public:
		AnimationBlender();
		~AnimationBlender();

		void tick(float dt, AnimLayers& layers);
		void blendBounds(const AnimLayers& layers, AABB& bound);
		void blendPalette(const AnimLayers& layers, const Skeleton& skeleton, TransformPalette& localPalette);

	private:
		AABB			 m_blendBound;
		TransformPalette m_blendPalette;
	};


	//-- The main purpose of the AnimationEngine is to provide uniform and optimal way of doing
	//-- animation on the arbitrary geometry set and to hide complexity of this task inside his
	//-- implementation.
	//------------------------------------------------------------------------------------------------------------------
	class AnimationEngine : public NonCopyable
	{
	public:
		AnimationEngine();
		~AnimationEngine();

		bool			init();

		//-- calculate only bounds for the animated model, but not matrix local palette.
		void			preAnimate(float dt);
		//-- calculate matrix world palette only for visible or desired models.
		void			animate();
		//-- pre-multiply each joint matrix for correct skinning. This method called after physics 
		//-- update step.
		void			postAnimate();

		//-- animation controllers.
		Handle			createAnimationController(AnimationController::Desc& desc);
		bool			removeAnimationController(Handle handle);
		
		//-- some animation controlling functions.
		void			playAnim(Handle id, const char* name, bool looped = false);
		void			pauseAnim(Handle id, int layerIdx = -1);
		void			continueAnim(Handle id, int layerIdx = -1);
		void			goToAnim(Handle id, uint frame, int layerIdx = -1);
		void			stopAnim(Handle id);
		void			blendAnim(Handle id, float srcBlend, float dstBlend, const char* name);
		void			physicsDriven(Handle id, bool flag);

		std::shared_ptr<Animation>	getAnim(const char* name);

	private:
		void		   addToActive(AnimationController* data);
		void		   delFromActive(AnimationController* data);
		void		   debugDraw();

	private:
		std::vector<std::unique_ptr<AnimationController>>				m_animCtrls;
		std::unordered_map<std::string, std::shared_ptr<Animation>>		m_animations;
		std::vector<AnimationController*>								m_activeAnimCtrls;
		AnimationBlender												m_animBlender;
	};

} //-- render
} //-- brUGE