#pragma once

#include "math_types.hpp"
#include "Vector3.hpp"
#include "AABB.hpp"

namespace brUGE
{
namespace math
{
	
	//----------------------------------------------------------------------------------------------
	class Plane
	{
	public:
		enum EClassify
		{
			CLASSIFLY_IN_FRONT = 1,
			CLASSIFLY_IN_BACK  = 2,
			CLASSIFLY_ON_PLANE = 3
		};

	public:
		explicit Plane () : m_normal(0, 0, 1), m_dist(0)
		{
		}

		//-- build plane from plane equation.
		Plane(const vec3f& normal, float d) : m_normal(normal), m_dist(d)
		{
			m_normal.normalize();
		}

		//-- build plane from plane equation.
		Plane(float nx, float ny, float nz, float d) : m_normal(nx, ny, nz), m_dist(d)
		{
			m_normal.normalize();
		}

		//-- build plane from normal and point on plane.
		Plane(const vec3f& normal, const vec3f& point) : m_normal(normal), m_dist(0)
		{
			m_normal.normalize();
			m_dist = -point.dot(m_normal);
		}

		//-- build plane from 3 points.
		Plane(const vec3f& p1, const vec3f& p2, const vec3f& p3)
		{
			m_normal = vec3f(p2 - p1).cross(vec3f(p3 - p1)).getNormalized();
			m_dist = -p1.dot(m_normal);
		}

		inline float signedDistanceTo(const vec3f& point) const
		{
			return (m_normal.dot(point) + m_dist);
		}

		//-- classify point.
		inline EClassify classify(const vec3f& point) const
		{
			float v = signedDistanceTo(point);

			if		(v > +EPSILON)	return CLASSIFLY_IN_FRONT;
			else if (v < -EPSILON)	return CLASSIFLY_IN_BACK;
			else					return CLASSIFLY_ON_PLANE;
		}

	public:
		vec3f m_normal;
		float m_dist;
	};

} // math
} // brUGE
