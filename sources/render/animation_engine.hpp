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
	//-- static information for every particular animation.
	//----------------------------------------------------------------------------------------------
	class Animation : public RefCount
	{
	public:
		Animation();
		virtual ~Animation();

		bool load(const utils::ROData& data);

		void goToFrame(
			uint frame, Joint::Transforms& skeleton, AABB& bound
			);
		void tick(
			float time, const Joints& skeleton, Joint::Transforms& oSkeleton, AABB& oBound,
			uint frameRate = 24, bool isLooped = false, bool isLocal = true
			);

	private:
		void updateJoints(Joint::Transforms& skeleton, uint _1stFrame, uint _2ndFrame, float blend);
		void updateBounds(AABB& bound, uint _1stFrame, uint _2ndFrame, float blend);
		void setupFrame	 (Joint::Transform& transf, uint frame, uint joint);
		void buildAbsoluteTransforms(Joint::Transforms& oTransforms, const Joints& skeleton, bool isLocal);

		//-- load operations.
		bool loadHierarchy(const utils::ROData& data);
		bool loadBounds	  (const utils::ROData& data);
		bool loadBaseFrame(const utils::ROData& data);
		bool loadFrame	  (const utils::ROData& data, uint idx);

	private:
		struct HierarchyItem
		{
			int	flags, startIdx;
		};
		typedef std::vector<HierarchyItem>	Hierarchy;
		typedef std::vector<float>			FrameData;
		typedef std::vector<FrameData>		FramesData;
		typedef std::vector<AABB>			Bounds;

		//-- ToDo: Seems useless because we always use custom frame rate. Maybe delete?
		//float			  m_animTime;
		//uint			  m_frameRate;

		uint			  m_animComponents;
		uint			  m_jointCount;
		uint			  m_frameCount;
		
		Hierarchy		  m_hierarchy;
		FramesData		  m_frames;
		Bounds			  m_bounds;
		Joint::Transforms m_baseFrame;
	};


	//-- Consists of the arbitrary number of the animation layers. The all layers is blended 
	//-- together and makes the final animation of the mesh. Each layer may be configured with
	//-- own list of options.
	//----------------------------------------------------------------------------------------------
	struct AnimationData
	{
		struct Desc
		{
			Desc() : m_idleAnim(nullptr), m_transform(nullptr), m_meshInst(nullptr) { }

			const char*	   m_idleAnim;
			Transform*	   m_transform;
			MeshInstance*  m_meshInst;
		};

		AnimationData(Desc& desc);

		//-- Represents one particular layer of the animation.
		//------------------------------------------------------------------------------------------
		struct AnimLayer
		{
			AnimLayer() : m_isLooped(false), m_time(0), m_blend(0.0f) { }

			bool		   m_isLooped;
			float		   m_time;
			float		   m_blend;
			Ptr<Animation> m_anim;
		};
		typedef std::vector<AnimLayer> AnimLayers;

		Joint::Transforms m_localPositions;
		AnimLayers		  m_animLayers;
		Transform*		  m_transform;
		MeshInstance*	  m_meshInst;
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

		//-- animate all the requested animations.
		void			animate		(float dt);
		//-- calculate all the world palettes for appropriate meshes.
		void			postAnimate	();

		//-- animation controllers.
		Handle			addAnimDef	(AnimationData::Desc& desc);
		bool			delAnimDef	(Handle handle);
		
		//-- some animation controlling functions.
		void			playAnim	(Handle id, const char* name, bool isLooped = false, uint rate = 24);
		void			stopAnim	(Handle id);
		void			blendAnim	(Handle id, float srcBlend, float dstBlend, const char* name, bool isLooped = false, uint rate = 24);

	private:
		Ptr<Animation> getAnim(const char* name);

	private:
		typedef std::vector<AnimationData*>				AnimationInsts;
		typedef std::map<std::string, Ptr<Animation>>	AnimationsMap;

		AnimationInsts	m_animCtrls;
		AnimationsMap	m_animations;
	};

} //-- render
} //-- brUGE