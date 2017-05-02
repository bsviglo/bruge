#include "animation.hpp"

namespace brUGE
{
	//------------------------------------------------------------------------------------------------------------------
	Animation::Animation()	:	m_numJoints(0), m_numFrames(0)
	{

	}

	//------------------------------------------------------------------------------------------------------------------
	Animation::~Animation()
	{

	}

	//------------------------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------------------------
	void Animation::goTo(uint frame, Time& oTime) const
	{
		//-- update blending parameters.
		oTime.m_1st   = min(frame, m_numFrames - 1);
		oTime.m_2nd   = min(frame, m_numFrames - 1);
		oTime.m_blend = 0.0f;
	}

	//------------------------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------------------------------------------
	void Animation::updateBounds(AABB& oBound, const Time& time) const
	{
		//-- smoothly change one AABB to another.
		oBound.m_min = lerp(m_bounds[time.m_1st].m_min, m_bounds[time.m_2nd].m_min, time.m_blend);
		oBound.m_max = lerp(m_bounds[time.m_1st].m_max, m_bounds[time.m_2nd].m_max, time.m_blend);
	}

	//------------------------------------------------------------------------------------------------------------------
	void Animation::updatePalette(TransformPalette& oPalette, const Time& time, const Skeleton& skeleton) const
	{
		//-- calculate blended results.
		updateJoints(oPalette, time.m_1st, time.m_2nd, time.m_blend);

		//-- reconstruct absolute transformation of the skeleton nodes.
		buildAbsoluteTransforms(oPalette, skeleton);
	}

	//------------------------------------------------------------------------------------------------------------------
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
}