#include "animation_system.hpp"
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

	//------------------------------------------------------------------------------------------------------------------
	AnimationSystem::AnimationSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	AnimationSystem::~AnimationSystem()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	bool AnimationSystem::init(const pugi::xml_node& cfg)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------------------------
	void AnimationSystem::preAnimate(const DeltaTime& dt)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void AnimationSystem::animate()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void AnimationSystem::postAnimate()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	AnimationSystem::World::World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	AnimationSystem::World::~World()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle AnimationSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle AnimationSystem::World::createComponent(Handle gameObj, IComponent::TypeID typeID, const pugi::xml_node& cfg)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	IComponent::Handle AnimationSystem::World::cloneComponent(Handle srcGameObj, Handle dstGameObj, IComponent::TypeID typeID)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	void AnimationSystem::World::removeComponent(IComponent::Handle component)
	{

	}


	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------------


	//-- ToDo: legacy



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

			//-- calculate palette for GPU execution.
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

} //-- brUGE