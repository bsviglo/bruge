#include "animation_engine.hpp"
#include "os/FileSystem.h"
#include "console/Console.h"
#include "math/math_all.hpp"
#include "game_world.hpp"
#include "render_world.hpp"
#include "mesh_manager.hpp"
#include "DebugDrawer.h"

using namespace brUGE::os;
using namespace brUGE::utils;
using namespace brUGE::math;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//-- converts one transformation presentation (quat, vec3f) to another mat4f.
	//----------------------------------------------------------------------------------------------
	inline mat4f quatToMat4x4(const quat& q, const vec3f& pos)
	{
		mat4f out;
		
		//-- 1. rotation part.
		out = q.toMat4();
		out.transpose();

		//-- 2. translation part.
		out.m30 = pos.x; out.m31 = pos.y; out.m32 = pos.z;

		return out;
	}

	//----------------------------------------------------------------------------------------------
	bool g_drawSkeletons = false;
	bool g_drawNodes = false;

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

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
		for (uint i = 0; i < m_animCtrls.size(); ++i)
			delete m_animCtrls[i];

		m_animCtrls.clear();
	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::init()
	{
		//-- register console functions and values.
		REGISTER_CONSOLE_VALUE("anim_drawSkeletons", bool, g_drawSkeletons);
		REGISTER_CONSOLE_VALUE("anim_drawNodes",	 bool, g_drawNodes);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::animate(float dt)
	{
		for (uint i = 0; i < m_animCtrls.size(); ++i)
		{
			AnimationData* data = m_animCtrls[i];
			if (data)
			{
				MeshInstance* mesh = data->m_meshInst;
				for (uint j = 0; j < data->m_animLayers.size(); ++j)
				{
					AABB aabb;
					AnimationData::AnimLayer& layer = data->m_animLayers[j];

					//-- increment timing.
					layer.m_time += dt;

					//-- extract local transformations of the node.
					layer.m_anim->tick(
						layer.m_time, mesh->m_skinnedMesh->skeleton(),
						data->m_localPositions, aabb, 24, layer.m_isLooped, true
						);

					//-- update local and world bounds.
					mesh->m_transform->m_localBounds = aabb;
					mesh->m_transform->m_worldBounds = aabb.getTranformed(mesh->m_transform->m_worldMat);

					//-- ToDo: blend skeleton.
				}

				//-- translate it to the world space. Needed for physics.
				Nodes& nodes = mesh->m_transform->m_nodes;
				for (uint i = 0; i < nodes.size() - 1; ++i)
				{
					Joint::Transform& transf = data->m_localPositions[i];
					Node&			  node   = *nodes[i + 1];
					
					mat4f worldTransf = quatToMat4x4(transf.orient, transf.pos);
					worldTransf.postMultiply(mesh->m_transform->m_worldMat);

					node.matrix(worldTransf);
				}

				//-- ToDo: draw skeleton.
				if (g_drawNodes || g_drawSkeletons)
				{
					//const mat4f&  worldMat = mesh->m_transform->m_worldMat;
					const Joints& joints = mesh->m_skinnedMesh->skeleton();
					for (uint i = 0; i < joints.size(); ++i)
					{
						const vec3f& startPos = nodes[i + 1]->matrix().applyToOrigin();
						if (g_drawNodes)
							DebugDrawer::instance().drawText2D(joints[i].m_name.c_str(), startPos, Color(1,1,0,1));

						if (joints[i].m_parentIdx != -1)
						{
							//const vec3f& startPos = worldMat.applyToPoint(data->m_localPositions[i].pos);
							//const vec3f& endPos   = worldMat.applyToPoint(data->m_localPositions[joints[i].m_parentIdx].pos);
						
							const vec3f& endPos = nodes[joints[i].m_parentIdx + 1]->matrix().applyToOrigin();
							if (g_drawSkeletons)
								DebugDrawer::instance().drawLine(startPos, endPos, Color(1,1,1,1));
						}
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::postAnimate()
	{

	}

	//----------------------------------------------------------------------------------------------
	Handle AnimationEngine::addAnimDef(AnimationData::Desc& desc)
	{
		std::unique_ptr<AnimationData> animData(new AnimationData(desc));
		if (desc.m_idleAnim)
		{
			AnimationData::AnimLayer layer;
			layer.m_anim	 = getAnim(desc.m_idleAnim);
			layer.m_blend	 = 1.0f;
			layer.m_time	 = 0.0f;
			layer.m_isLooped = true;
			
			animData->m_animLayers.push_back(layer);
		}

		m_animCtrls.push_back(animData.release());
		return m_animCtrls.size() - 1;
	}

	//----------------------------------------------------------------------------------------------
	bool AnimationEngine::delAnimDef(Handle id)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		//-- reset to empty.
		delete m_animCtrls[id];
		m_animCtrls[id] = nullptr;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::playAnim(Handle id, const char* name, bool isLooped, uint /*rate*/)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));
		
		AnimationData* data = m_animCtrls[id];
		assert(data);
		
		AnimationData::AnimLayer layer;
		layer.m_anim	 = getAnim(name);
		layer.m_blend	 = 1.0f;
		layer.m_time	 = 0.0f;
		layer.m_isLooped = isLooped;

		data->m_animLayers.push_back(layer);
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::stopAnim(Handle id)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));

		//-- reset to empty.
		if (m_animCtrls[id])
			m_animCtrls[id]->m_animLayers.clear();
	}

	//----------------------------------------------------------------------------------------------
	void AnimationEngine::blendAnim(
		Handle id, float /*srcBlend*/, float /*dstBlend*/, const char* /*name*/,
		bool /*isLooped*/, uint /*rate*/)
	{
		assert(id != CONST_INVALID_HANDLE && id < static_cast<int>(m_animCtrls.size()));
		id;

		//-- ToDo:
	}

	//----------------------------------------------------------------------------------------------
	Ptr<Animation> AnimationEngine::getAnim(const char* name)
	{
		AnimationsMap::iterator iter = m_animations.find(name);
		if (iter != m_animations.end())
		{
			return iter->second;
		}
		else
		{
			RODataPtr data = FileSystem::instance().readFile("resources/models/" + std::string(name) + ".md5anim");
			if (!data.get())
			{
				return NULL;
			}

			Ptr<Animation> result = new Animation();
			if (result->load(*data))
			{
				m_animations[name] = result;
			}
			else
			{
				result = NULL;
			}
			return result;
		}
	}

	//----------------------------------------------------------------------------------------------
	Animation::Animation()
		:	m_animComponents(0), m_jointCount(0), m_frameCount(0)
	{

	}

	//----------------------------------------------------------------------------------------------
	Animation::~Animation()
	{

	}

	//----------------------------------------------------------------------------------------------
	void Animation::setupFrame(Joint::Transform& transf, uint frame, uint joint)
	{
		//-- set base transformation.
		transf = m_baseFrame[joint];

		FrameData&	data  = m_frames[frame];
		int			flags = m_hierarchy[joint].flags;
		int			pos   = m_hierarchy[joint].startIdx;

		//-- update transformation specific for current frame.
		if (flags & 1 )	transf.pos.x	= data[pos++] * 0.1f;
		if (flags & 2 )	transf.pos.y	= data[pos++] * 0.1f;
		if (flags & 4 ) transf.pos.z	= data[pos++] * 0.1f;
		if (flags & 8 ) transf.orient.x = data[pos++];
		if (flags & 16) transf.orient.y = data[pos++];
		if (flags & 32) transf.orient.z = data[pos++];

		renormalize(transf.orient);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateJoints(Joint::Transforms& skeleton, uint _1st, uint _2nd, float blend)
	{
		Joint::Transform first, second;

		for (uint i = 0; i < m_jointCount; ++i)
		{
			//-- find joint transformation at the fist and the second frames.
			setupFrame(first,  _1st, i);
			setupFrame(second, _2nd, i);

			//-- blend results using blend factor.
			skeleton[i].orient = slerp(first.orient, second.orient, blend);
			skeleton[i].pos    = lerp (first.pos, second.pos, blend);
		}
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateBounds(AABB& bound, uint _1st, uint _2nd, float blend)
	{
		//-- smoothly change one AABB to another.
		bound.m_min = lerp(m_bounds[_1st].m_min, m_bounds[_2nd].m_min, blend);
		bound.m_max = lerp(m_bounds[_1st].m_max, m_bounds[_2nd].m_max, blend);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::tick(
		float time, const Joints& skeleton, Joint::Transforms& oSkeleton, AABB& oBound,
		uint frameRate, bool isLooped, bool isLocal)
	{
		//-- calculate total time for the desired frame rate and frame count.
		float totalTime = (m_frameCount - 1) / static_cast<float>(frameRate);
		
		//-- adjust animation time in case the looped animation.
		if (isLooped)
		{
			while (time > totalTime)
				time -= totalTime;
		}
		else
		{
			time = math::min(time, totalTime);
		}

		//-- calculate blend frame parameters.
		float _blendFrame  = frameRate * time;
		uint  _1st   	   = floorf(_blendFrame);
		uint  _2nd   	   = ceilf(_blendFrame);
		float _blendFactor = _blendFrame - _1st;

		//--ConPrint("Animation: time = %f; frame1 = %d; frame2 = %d; blend = %f",
		//--	time, _1st, _2nd, _blendFactor
		//--	);

		//-- calculate blended results.
		updateJoints(oSkeleton, _1st, _2nd, _blendFactor);		
		updateBounds(oBound   , _1st, _2nd, _blendFactor);

		//-- reconstruct absolute transformation of the skeleton nodes.
		buildAbsoluteTransforms(oSkeleton, skeleton, isLocal);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::buildAbsoluteTransforms(
		Joint::Transforms& oTransforms, const Joints& skeleton, bool isLocal)
	{
		//-- set position of the root node to zero if isLocal flag specified.
		if (isLocal)
		{
			oTransforms[0].pos.setZero();
		}
		
		//-- iterate over the all nodes and apply parent's transformation for every node.
		for (uint i = 1; i < skeleton.size(); ++i)
		{
			int						idx			 = skeleton[i].m_parentIdx;
			Joint::Transform&		transf		 = oTransforms[i];
			const Joint::Transform& parentTransf = oTransforms[idx];

			transf.pos    = parentTransf.pos + parentTransf.orient.rotate(transf.pos);
			transf.orient = parentTransf.orient * transf.orient;
		}
	}

	//----------------------------------------------------------------------------------------------
	void Animation::goToFrame(uint frame, Joint::Transforms& skeleton, AABB& bound)
	{
		for (uint i = 0; i < m_jointCount; ++i)
		{
			setupFrame(skeleton[i], frame, i);
		}

		bound = m_bounds[frame];
	}

	//----------------------------------------------------------------------------------------------
	AnimationData::AnimationData(AnimationData::Desc& desc)
		: m_transform(desc.m_transform), m_meshInst(desc.m_meshInst)
	{
		m_localPositions.resize(m_meshInst->m_skinnedMesh->skeleton().size());
	}

} //-- render
} //-- brUGE