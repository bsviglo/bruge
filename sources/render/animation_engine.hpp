#pragma once

#include "prerequisites.h"
#include "utils/Data.hpp"
#include "render/Mesh.hpp"

namespace brUGE
{
	class Transform;

namespace render
{
	class MeshInstance;
	

	//-- For each particular animation we have only one instance of this class. It contains only
	//-- static information for every particular animation.
	//----------------------------------------------------------------------------------------------
	class Animation : public RefCount
	{
	public:
		Animation();
		~Animation();

		bool load(const utils::ROData& data);

		void goToFrame(
			uint frame, Joint::Transforms& skeleton, AABB& bound
			);
		void tick(
			float time, const Joints& skeleton, Joint::Transforms& oSkeleton, AABB& oBound,
			bool isLooped = false, bool isLocal = true
			);

	private:
		void updateJoints(Joint::Transforms& skeleton, uint _1stFrame, uint _2ndFrame, float blend);
		void updateBounds(AABB& bound, uint _1stFrame, uint _2ndFrame, float blend);
		void setupFrame	 (Joint::Transform& transf, uint frame, uint joint);

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

		float			  m_animTime;
		uint			  m_animComponents;
		uint			  m_jointCount;
		uint			  m_frameCount;
		uint			  m_frameRate;

		Hierarchy		  m_hierarchy;
		FramesData		  m_frames;
		Bounds			  m_bounds;
		Joint::Transforms m_baseFrame;

	private:
		static uint m_forceFrameRate;
		static int _setFrameRate(uint frameRate);
	};


	//----------------------------------------------------------------------------------------------
	struct AnimationData
	{
		AnimationData() : m_totalTime(0) { }

		struct AnimLayer
		{
			AnimLayer() : m_time(0), m_blend(0.0f) { }

			bool		   m_isLooped;
			uint		   m_time;
			float		   m_blend;
			Ptr<Animation> m_anim;
		};
		typedef std::vector<AnimLayer> AnimLayers;

		uint			  m_totalTime;
		Joint::Transforms m_localPositions;
		AnimLayers		  m_animLayers;
		Transform*		  m_transform;
		MeshInstance*	  m_meshInst;
	};


	//----------------------------------------------------------------------------------------------
	class AnimationEngine
	{
	public:
		bool init();
		bool fini();

		//-- animate all the requested animations.
		void	animate		(float dt);
		//-- calculate all the world palettes for appropriate meshes.
		void	postAnimate	();

		//-- animation controllers.
		Handle	addAnimDef	(const AnimationData* animCtrl);
		bool	delAnimDef	(Handle handle);
		
		//-- some animation controlling functions.
		void	playAnim	(Handle id, const char* name, bool isLooped = false, uint rate = 24);
		void	stopAnim	(Handle id);
		void	blendAnim	(Handle id, float srcBlend, float dstBlend, const char* name, bool isLooped = false, uint rate = 24);

	private:
		Ptr<Animation> getAnim(const char* name);

	private:
		typedef std::vector<AnimationData*>				AnimationInsts;
		typedef std::map<std::string, Ptr<Animation> >	AnimationsMap;

		AnimationInsts	m_animCtrls;
		AnimationsMap	m_animations;
	};

} //-- render
} //-- brUGE