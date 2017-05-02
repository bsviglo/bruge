#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "render/Mesh.hpp"

namespace brUGE
{
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
		void	updatePalette(render::TransformPalette& oPalette, const Time& time, const render::Skeleton& skeleton) const;

		uint	numJoints() const { return m_numJoints; }
		uint	numFrames() const { return m_numFrames; }

	private:
		void updateJoints(render::TransformPalette& oPalette, uint _1st, uint _2nd, float blend) const;
		void buildAbsoluteTransforms(render::TransformPalette& oPalette, const render::Skeleton& skeleton) const;

	private:
		uint									m_numJoints;
		uint									m_numFrames;
		uint									m_frameRate;
		std::vector<float>						m_blendAlphaMask;
		std::vector<render::TransformPalette>	m_frames;
		std::vector<AABB>						m_bounds;
	};
}
