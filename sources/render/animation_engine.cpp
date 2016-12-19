#include "animation_engine.hpp"
#include "os/FileSystem.h"
#include "math/math_all.hpp"
#include "scene/game_world.hpp"
#include "render_world.hpp"
#include "mesh_manager.hpp"
#include "mesh_formats.hpp"
#include "DebugDrawer.h"
#include <algorithm>

using namespace brUGE::os;
using namespace brUGE::utils;	
using namespace brUGE::math;

//--------------------------------------------------------------------------------------------------
namespace
{
	bool g_drawSkeletons = false;
	bool g_drawNodeNames = false;
	bool g_drawJoints    = false;
}


namespace brUGE
{
namespace render
{
	//----------------------------------------------------------------------------------------------
	AnimationEngine::AnimationEngine()
	{

	}

	//----------------------------------------------------------------------------------------------
	AnimationEngine::~AnimationEngine()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::init()
	{
		//-- register console functions and values.
		REGISTER_CONSOLE_VALUE("anim_drawSkeletons", bool, g_drawSkeletons);
		REGISTER_CONSOLE_VALUE("anim_drawNodeNames", bool, g_drawNodeNames);
		REGISTER_CONSOLE_VALUE("anim_drawJoints",    bool, g_drawJoints);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::preAnimate(float dt)
	{
		for (auto animCtrl : m_activeAnimCtrls)
		{
			//-- stop ticking and calculating world transforms for physics driven controllers.
			if (animCtrl->m_physicsDriven)
				continue;

			auto& transform = *animCtrl->m_meshInst->m_transform;

			//-- tick animation.
			m_animBlender.tick(dt, animCtrl->m_animLayers);

			//-- calculate local bound.
			m_animBlender.blendBounds(animCtrl->m_animLayers, transform.m_localBounds);

			//-- calculate world bound.
			transform.m_worldBounds = transform.m_localBounds.getTranformed(transform.m_worldMat);
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::animate()
	{
		for (auto animCtrl : m_activeAnimCtrls)
		{
			if (!animCtrl->m_wantsWorldPalette)
				continue;

			const auto& world			= animCtrl->m_transform->m_worldMat;
			const auto& skeleton		= animCtrl->m_meshInst->m_skinnedMesh->skeleton();
			auto&		worldPalette	= animCtrl->m_meshInst->m_worldPalette;

			//-- calculate local matrix palette.
			m_animBlender.blendPalette(animCtrl->m_animLayers, skeleton, animCtrl->m_tranformPalette);

			//-- transform (quat, pos) -> mat4f and calculate world space palette.
			for (uint i = 0; i < animCtrl->m_tranformPalette.size(); ++i)
			{
				auto&		mat = worldPalette[i];
				const auto& tp  = animCtrl->m_tranformPalette[i];
 
				mat = combineMatrix(tp.m_orient, tp.m_pos);
				mat.postMultiply(world);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::postAnimate()
	{
		for (auto animCtrl : m_activeAnimCtrls)
		{
			if (!animCtrl->m_wantsWorldPalette)
				continue;

			const auto& invBindPose	= animCtrl->m_meshInst->m_skinnedMesh->invBindPose();
			const auto& skeleton	= animCtrl->m_meshInst->m_skinnedMesh->skeleton();
			auto&		palette		= animCtrl->m_meshInst->m_worldPalette;

			//-- draw skeleton.
			if (g_drawNodeNames || g_drawSkeletons || g_drawJoints)
			{
				for (uint k = 0; k < skeleton.size(); ++k)
				{
					const vec3f& startPos = palette[k].applyToOrigin();

					if (g_drawJoints)
					{
						//DebugDrawer::instance().drawSphere(0.025f, palette[k], Color(1,0,0,1), DebugDrawer::DRAW_OVERRIDE);
						DebugDrawer::instance().drawCoordAxis(palette[k], 0.01f);
					}

					if (g_drawNodeNames)
					{
						DebugDrawer::instance().drawText2D(skeleton[k].m_name, startPos, Color(1, 1, 0, 1));
					}

					if (g_drawSkeletons)
					{
						if (skeleton[k].m_parent != -1)
						{
							const vec3f& endPos = palette[skeleton[k].m_parent].applyToOrigin();

							DebugDrawer::instance().drawLine(startPos, endPos, Color(1, 1, 1, 1));
						}
					}
				}
			}

			//-- calculate world space palette.
			for (uint j = 0; j < palette.size(); ++j)
			{
				palette[j].preMultiply(invBindPose[j]);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	Handle AnimationEngine::createAnimationController(AnimationController::Desc& desc)
	{
		auto animCtrl = std::make_unique<AnimationController>(desc);
		
		//-- try to find free slot.
		for (uint i = 0; i < m_animCtrls.size(); ++i)
		{
			if (!m_animCtrls[i])
			{
				m_animCtrls[i] = std::move(animCtrl);
				return i;
			}
		}

		m_animCtrls.push_back(std::move(animCtrl));
		return m_animCtrls.size() - 1;
	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::removeAnimationController(Handle id)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		delFromActive(m_animCtrls[id].get());

		//-- reset to empty.
		m_animCtrls[id].reset();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::playAnim(Handle id, const char* name, bool looped)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));
		
		const auto& animCtrl = m_animCtrls[id];
		assert(animCtrl);

		auto anim = getAnim(name);
		if (!anim)
		{
			WARNING_MSG("Can't load animation '%s'.", name);
			return;
		}
		
		AnimLayer layer;
		layer.m_anim   = anim;
		layer.m_blend  = 1.0f;
		layer.m_looped = looped;
		layer.m_paused = false;

		animCtrl->m_tranformPalette.resize(anim->numJoints());
		animCtrl->m_animLayers.push_back(layer);

		addToActive(animCtrl.get());
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::pauseAnim(Handle id, int layerIdx)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		const auto& animCtrl = m_animCtrls[id];
		if (animCtrl && !animCtrl->m_animLayers.empty())
		{
			if (layerIdx == -1)
			{
				for (auto& layer : animCtrl->m_animLayers)
					layer.m_paused = false;
			}
			else
			{
				animCtrl->m_animLayers[layerIdx].m_paused = true;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::continueAnim(Handle id, int layerIdx)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		const auto& animCtrl = m_animCtrls[id];
		if (animCtrl && !animCtrl->m_animLayers.empty())
		{
			if (layerIdx == -1)
			{
				for (auto& layer : animCtrl->m_animLayers)
					layer.m_paused = false;
			}
			else
			{
				animCtrl->m_animLayers[layerIdx].m_paused = false;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::goToAnim(Handle id, uint frame, int layerIdx)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		const auto& animCtrl = m_animCtrls[id];
		if (animCtrl && !animCtrl->m_animLayers.empty())
		{
			if (layerIdx == -1)
			{
				for (auto& layer : animCtrl->m_animLayers)
				{
					layer.m_paused = true;
					layer.m_anim->goTo(frame, layer.m_time);
				}
			}
			else
			{
				auto& layer = animCtrl->m_animLayers[layerIdx];

				layer.m_paused = true;
				layer.m_anim->goTo(frame, layer.m_time);
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::stopAnim(Handle id)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		//-- reset to empty.
		if (const auto& animCtrl = m_animCtrls[id])
		{
			animCtrl->m_animLayers.clear();
			delFromActive(animCtrl.get());
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::blendAnim(Handle id, float /*srcBlend*/, float /*dstBlend*/, const char* /*name*/)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));
		id;

		//-- ToDo:
	}

	//----------------------------------------------------------------------------------------------
	std::shared_ptr<Animation> AnimationEngine::getAnim(const char* name)
	{
		auto iter = m_animations.find(name);
		if (iter != m_animations.end())
		{
			return iter->second;
		}
		else
		{
			auto data = FileSystem::instance().readFile("resources/models/" + std::string(name) + ".animation");
			if (!data)
			{
				return nullptr;
			}

			auto result = std::make_shared<Animation>();
			if (result->load(*data))
			{
				m_animations[name] = result;
			}
			else
			{
				result = nullptr;
			}
			return result;
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::addToActive(AnimationController* data)
	{
		//-- add to active list.
		auto item = std::find(m_activeAnimCtrls.begin(), m_activeAnimCtrls.end(), data);
		if (item == m_activeAnimCtrls.end())
		{
			m_activeAnimCtrls.push_back(data);
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::delFromActive(AnimationController* data)
	{
		//-- remove from active list.
		auto item = std::find(m_activeAnimCtrls.begin(), m_activeAnimCtrls.end(), data);
		if (item != m_activeAnimCtrls.end())
		{
			m_activeAnimCtrls.erase(item);
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::debugDraw()
	{

	}

	//----------------------------------------------------------------------------------------------
	Animation::Animation()	:	m_numJoints(0), m_numFrames(0)
	{

	}

	//----------------------------------------------------------------------------------------------
	Animation::~Animation()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool Animation::load(const utils::ROData& iData)
	{
		//-- check header.
		{
			SkinnedMeshAnimationFormat::Header iHeader;
			iData.read(iHeader);
			if (std::string(iHeader.m_format) != "animation")
			{
				ERROR_MSG("Failed to load mesh. Most likely it's not a *.animation format.");
				return false;
			}
		}

		//-- load skeleton info.
		SkinnedMeshAnimationFormat::Skeleton::Info skelInfo;
		{
			iData.read(skelInfo);

			//-- read skeleton.
			//m_skeleton.resize(skelInfo.m_numJoints);
			for (uint i = 0; i < skelInfo.m_numJoints; ++i)
			{
				SkinnedMeshAnimationFormat::Skeleton::Joint iJoint;

				iData.read(iJoint);

				//strcpy_s(m_skeleton[i].m_name, iJoint.m_name);
				//m_skeleton[i].m_parent = iJoint.m_parent;
			}
		}

		//-- load info.
		SkinnedMeshAnimationFormat::Info info;
		iData.read(info);

		m_numFrames = info.m_numFrames;
		m_frameRate = info.m_frameRate;
		m_numJoints = skelInfo.m_numJoints;

		//-- read bounds.
		m_bounds.resize(m_numFrames);
		for (uint i = 0; i < m_numFrames; ++i)
		{
			SkinnedMeshAnimationFormat::Bound bound;
			iData.read(bound);

			m_bounds[i] = AABB(
				vec3f(bound.m_aabb[0], bound.m_aabb[1], bound.m_aabb[2]),
				vec3f(bound.m_aabb[3], bound.m_aabb[4], bound.m_aabb[5])
				);
		}

		//-- load joints transformations.
		m_frames.resize(m_numFrames);
		for (uint i = 0; i < m_numFrames; ++i)
		{
			m_frames[i].resize(m_numJoints);
			for (uint j = 0; j < m_numJoints; ++j)
			{
				SkinnedMeshAnimationFormat::Joint joint;
				iData.read(joint);

				m_frames[i][j].m_orient = quat(joint.m_quat);
				m_frames[i][j].m_pos    = vec3f(joint.m_pos);
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void Animation::goTo(uint frame, Time& oTime) const
	{
		//-- update blending parameters.
		oTime.m_1st   = min(frame, m_numFrames - 1);
		oTime.m_2nd   = min(frame, m_numFrames - 1);
		oTime.m_blend = 0.0f;
	}

	//----------------------------------------------------------------------------------------------
	void Animation::tick(float dt, Time& oTime, bool looped) const
	{
		//-- increment timing.
		oTime.m_time += dt;

		//-- calculate total time for desired frame rate and frame count.
		float totalTime = (m_numFrames - 1) / static_cast<float>(m_frameRate);

		//-- adjust animation time in case the looped animation.
		if (looped)
		{
			while (oTime.m_time > totalTime)
				oTime.m_time -= totalTime;
		}
		else
		{
			oTime.m_time = math::clamp(0.0f, oTime.m_time, totalTime);
		}

		//-- calculate blend frame parameters.
		float blendFrame  = m_frameRate * oTime.m_time;

		//-- update blending parameters.
		oTime.m_1st   = floorf(blendFrame);
		oTime.m_2nd   = ceilf(blendFrame);
		oTime.m_blend = blendFrame - oTime.m_1st;

		//--ConPrint("Animation: time = %f; frame1 = %d; frame2 = %d; blend = %f",
		//--	oTime.m_time, oTime.m_1st, oTime.m_2nd, oTime.m_blend
		//--	);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateJoints(TransformPalette& oPalette, uint _1st, uint _2nd, float blend) const
	{
		for (uint i = 0; i < m_numJoints; ++i)
		{
			//-- find joint transformation at the fist and the second frames.
			const auto& first  = m_frames[_1st][i];
			const auto& second = m_frames[_2nd][i];

			//-- blend results using blend factor.
			oPalette[i].m_orient = slerp(first.m_orient, second.m_orient, blend);
			oPalette[i].m_pos    = lerp (first.m_pos, second.m_pos, blend);
		}
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateBounds(AABB& oBound, const Time& time) const
	{
		//-- smoothly change one AABB to another.
		oBound.m_min = lerp(m_bounds[time.m_1st].m_min, m_bounds[time.m_2nd].m_min, time.m_blend);
		oBound.m_max = lerp(m_bounds[time.m_1st].m_max, m_bounds[time.m_2nd].m_max, time.m_blend);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updatePalette(TransformPalette& oPalette, const Time& time, const Skeleton& skeleton) const
	{
		//-- calculate blended results.
		updateJoints(oPalette, time.m_1st, time.m_2nd, time.m_blend);

		//-- reconstruct absolute transformation of the skeleton nodes.
		buildAbsoluteTransforms(oPalette, skeleton);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::buildAbsoluteTransforms(TransformPalette& oPalette, const Skeleton& skeleton) const
	{
		//-- set position of the root node to zero if isLocal flag specified.
		oPalette[0].m_pos.setZero();
		
		//-- iterate over the all nodes and apply parent's transformation for every node.
		for (uint i = 1; i < skeleton.size(); ++i)
		{
			int			idx			 = skeleton[i].m_parent;
			auto&		transf		 = oPalette[i];
			const auto& parentTransf = oPalette[idx];

			transf.m_pos    = parentTransf.m_pos + parentTransf.m_orient.rotate(transf.m_pos);
			transf.m_orient = parentTransf.m_orient * transf.m_orient;
		}
	}

	//----------------------------------------------------------------------------------------------
	AnimationBlender::AnimationBlender()
	{

	}

	//----------------------------------------------------------------------------------------------
	AnimationBlender::~AnimationBlender()
	{

	}

	//----------------------------------------------------------------------------------------------
	void AnimationBlender::tick(float dt, AnimLayers& layers)
	{
		for (auto& layer : layers)
		{
			if (layer.m_paused)
				continue;

			layer.m_anim->tick(dt, layer.m_time, layer.m_looped);
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationBlender::blendBounds(const AnimLayers& layers, AABB& bound)
	{
		//-- if we doesn't have any layer.
		if (layers.empty())
			return;

		if (layers.size() == 1)
		{
			layers[0].m_anim->updateBounds(bound, layers[0].m_time);
			return;
		}

		//-- ToDo: implement blending
	}

	//----------------------------------------------------------------------------------------------
	void AnimationBlender::blendPalette(const AnimLayers& layers, const Skeleton& skeleton, TransformPalette& oPalette)
	{
		//-- if we doesn't have any layer.
		if (layers.empty())
			return;

		if (layers.size() == 1)
		{
			layers[0].m_anim->updatePalette(oPalette, layers[0].m_time, skeleton);
			return;
		}

		//-- ToDo: implement blending
	}

} //-- render
} //-- brUGE