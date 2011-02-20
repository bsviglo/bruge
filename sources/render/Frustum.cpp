#include "Frustum.h"
#include "math/Matrix4x4.h"

namespace brUGE
{
	using namespace math;

namespace render
{
	
	//----------------------------------------------------------------------------------------------
	Frustum::Frustum()
	{

	}

	//----------------------------------------------------------------------------------------------
	Frustum::~Frustum()
	{

	}

	//----------------------------------------------------------------------------------------------
	void Frustum::update(const mat4f& VP)
	{
		//-- right plane.
		m_planes[0].m_normal.x	= VP[ 3] - VP[ 0];
		m_planes[0].m_normal.y	= VP[ 7] - VP[ 4];
		m_planes[0].m_normal.z	= VP[11] - VP[ 8];
		m_planes[0].m_dist		= VP[15] - VP[12];

		//-- left plane.
		m_planes[1].m_normal.x	= VP[ 3] + VP[ 0];
		m_planes[1].m_normal.y	= VP[ 7] + VP[ 4];
		m_planes[1].m_normal.z	= VP[11] + VP[ 8];
		m_planes[1].m_dist		= VP[15] + VP[12];

		//-- bottom plane.
		m_planes[2].m_normal.x	= VP[ 3] + VP[ 1];
		m_planes[2].m_normal.y	= VP[ 7] + VP[ 5];
		m_planes[2].m_normal.z	= VP[11] + VP[ 9];
		m_planes[2].m_dist		= VP[15] + VP[13];

		//-- top plane.
		m_planes[3].m_normal.x	= VP[ 3] - VP[ 1];
		m_planes[3].m_normal.y	= VP[ 7] - VP[ 5];
		m_planes[3].m_normal.z	= VP[11] - VP[ 9];
		m_planes[3].m_dist		= VP[15] - VP[13];

		//-- far plane.
		m_planes[4].m_normal.x	= VP[ 3] - VP[ 2];
		m_planes[4].m_normal.y	= VP[ 7] - VP[ 6];
		m_planes[4].m_normal.z	= VP[11] - VP[10];
		m_planes[4].m_dist		= VP[15] - VP[14];

		//-- near plane.
		m_planes[5].m_normal.x	= VP[ 3] + VP[ 2];
		m_planes[5].m_normal.y	= VP[ 7] + VP[ 6];
		m_planes[5].m_normal.z	= VP[11] + VP[10];
		m_planes[5].m_dist		= VP[15] + VP[14];
		
		for (uint i = 0; i < 6; ++i)
		{
			float length = m_planes[i].m_normal.length();

			if (!almostZero(length))
			{
				m_planes[i].m_normal.normalize();
				m_planes[i].m_dist /= length;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------------
	bool Frustum::testAABB(const AABB& aabb) const
	{
		for (uint i = 0; i < 6; ++i)
		{
			if (m_planes[i].classify(aabb) == Plane::CLASSIFLY_IN_BACK)
			{
				return false;
			}
		}
		return true;
	}

} // render
} // brUGE