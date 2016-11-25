#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "render/Mesh.hpp"

#include <memory>
#include <vector>

namespace brUGE
{
	struct Transform;

namespace render
{
	struct MeshInstance;
	

	//-- For each particular animation we have only one instance of this class. It contains only
	//-- static information of animation.
	//----------------------------------------------------------------------------------------------
	class Animation : public RefCount
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

		bool					load			(const utils::ROData& data);
		void					tick			(float dt, Time& oTime, bool looped);
		void					goTo			(uint frame, Time& oTime);

		//-- update bounds and matrix palette.
		const AABB&				updateBounds	(const Time& time);
		const TransformPalette&	updatePalette	(const Time& time, const Skeleton& skeleton);

		uint					numJoints		() const { return m_numJoints; }
		uint					numFrames		() const { return m_numFrames; }

	private:
		void updateJoints(TransformPalette& palette, uint _1st, uint _2nd, float blend);
		void buildAbsoluteTransforms(TransformPalette& palette, const Skeleton& skeleton);

	private:
		typedef std::vector<float> BlendAlphaMask;
		typedef std::vector<AABB>  Bounds;
		typedef std::vector<TransformPalette> AnimationFrames;

		uint			 m_numJoints;
		uint			 m_numFrames;
		uint			 m_frameRate;
		BlendAlphaMask	 m_blendAlphaMask;
		AnimationFrames	 m_frames;
		Bounds			 m_bounds;
		AABB			 m_tempBound;
		TransformPalette m_tempPalette;
	};

	//-- Represents one particular layer of the animation.
	//------------------------------------------------------------------------------------------
	struct AnimLayer
	{
		AnimLayer() : m_looped(false), m_paused(false), m_blend(0.0f) { }

		bool			m_looped;
		bool			m_paused;
		float			m_blend;
		Animation::Time	m_time;
		Ptr<Animation>	m_anim;
	};
	typedef std::vector<AnimLayer> AnimLayers;

	//-- Consists of the arbitrary number of the animation layers. The all layers is blended 
	//-- together and makes the final animation of the mesh. Each layer may be configured with the
	//-- own list of options.
	//----------------------------------------------------------------------------------------------
	struct AnimationData
	{
		struct Desc
		{
			Desc() : m_transform(nullptr), m_meshInst(nullptr) { }

			Transform*	   m_transform;
			MeshInstance*  m_meshInst;
		};

		AnimationData(Desc& desc)
			:	m_transform(desc.m_transform), m_meshInst(desc.m_meshInst),	m_wantsWorldPalette(true) { }

	public:
		bool			m_wantsWorldPalette;
		AnimLayers		m_animLayers;
		Transform*		m_transform;
		MeshInstance*	m_meshInst;
	};


	//-- ToDo: Maybe it will be better to create one AnimationBlender per skeleton.
	//-- Blends two or more animation together and produce a new blended animation on the output.
	//----------------------------------------------------------------------------------------------
	class AnimationBlender
	{
	public:
		AnimationBlender();
		~AnimationBlender();

		void tick			(float dt, AnimLayers& layers);
		void blendBounds	(AnimLayers& layers, AABB& bound);
		void blendPalette	(AnimLayers& layers, const Skeleton& skeleton, MatrixPalette& localPalette);

	private:
		AABB			 m_blendBound;
		TransformPalette m_blendPalette;
	};


	//-- The main purpose of the AnimationEngine is to provide uniform and optimal way of doing
	//-- animation on the arbitrary geometry set and to hide complexity of this task inside his
	//-- implementation.
	//----------------------------------------------------------------------------------------------
	class AnimationEngine : public NonCopyable
	{
	public:
		AnimationEngine();
		~AnimationEngine();

		bool			init();

		//-- calculate only bounds for the animated model, but not matrix local palette.
		void			preAnimate	(float dt);
		//-- calculate matrix world palette only for visible or desired models.
		void			animate		();
		//-- pre-multiply each joint matrix for correct skinning. This method called after physics 
		//-- update step.
		void			postAnimate	();

		//-- animation controllers.
		Handle			addAnimDef	(AnimationData::Desc& desc);
		bool			delAnimDef	(Handle handle);
		
		//-- some animation controlling functions.
		void			playAnim	(Handle id, const char* name, bool looped = false);
		void			pauseAnim	(Handle id, int layer = -1);
		void			continueAnim(Handle id, int layer = -1);
		void			goToAnim	(Handle id, uint frame, int layer = -1);
		void			stopAnim	(Handle id);
		void			blendAnim	(Handle id, float srcBlend, float dstBlend, const char* name);

		Ptr<Animation>	getAnim		(const char* name);

	private:
		void		   addToActive	(AnimationData* data);
		void		   delFromActive(AnimationData* data);

	private:
		typedef std::vector<AnimationData*>				AnimationInsts;
		typedef std::map<std::string, Ptr<Animation>>	AnimationsMap;

		AnimationInsts	 m_animCtrls;
		AnimationsMap	 m_animations;
		AnimationInsts	 m_activeAnimCtrls;
		AnimationBlender m_animBlender;
	};

} //-- render
} //-- brUGE