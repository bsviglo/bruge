#pragma once

#include "math_types.hpp"
#include "math_funcs.hpp"
#include "Vector3.hpp"
#include "Matrix4x4.hpp"

namespace brUGE
{
namespace math
{

	//-- AABB stands for Axis Aligned Bounding Box.
	//----------------------------------------------------------------------------------------------
	class AABB
	{
	public:
		inline AABB() { setEmpty(); }
		inline AABB(const vec3f& min, const vec3f& max) { set(min, max); }
		inline AABB(const AABB& rt) : m_min(rt.m_min), m_max(rt.m_max) { }
		inline ~AABB() {}

		AABB& operator = (const AABB& rt)
		{
			set(rt.m_min, rt.m_max);
			return *this;
		}

		inline void setEmpty()
		{
			set( INFINITY,  INFINITY,  INFINITY,
				-INFINITY, -INFINITY, -INFINITY);
		}

		inline void setInfinite()
		{
			set(-INFINITY, -INFINITY, -INFINITY,
				 INFINITY,  INFINITY,  INFINITY);
		}

		inline bool isEmpty() const
		{
			if (m_min.x < m_max.x)	return false;
			if (m_min.y < m_max.y)	return false;
			if (m_min.z < m_max.z)	return false;
			return true;
		}

		inline Vector3<float> getCenter() const
		{
			return (m_min + m_max).scale(0.5f);
		}

		inline Vector3<float> getDimensions() const
		{
			return m_max - m_min;
		}

		inline void set(float minx, float miny, float minz, float maxx, float maxy, float maxz)
		{
			m_min.set(minx, miny, minz);
			m_max.set(maxx, maxy, maxz);
		}

		inline void set(const vec3f& min, const vec3f& max)
		{
			m_min = min;
			m_max = max;
		}

		inline void include(const vec3f& point)
		{
			if (point.x < m_min.x) m_min.x = point.x;
			if (point.x > m_max.x) m_max.x = point.x;
			if (point.y < m_min.y) m_min.y = point.y;
			if (point.y > m_max.y) m_max.y = point.y;
			if (point.z < m_min.z) m_min.z = point.z;
			if (point.z > m_max.z) m_max.z = point.z;
		}

		inline void combine(const AABB& b2)
		{
			if (b2.m_min.x < m_min.x) m_min.x = b2.m_min.x;
			if (b2.m_min.y < m_min.y) m_min.y = b2.m_min.y;
			if (b2.m_min.z < m_min.z) m_min.z = b2.m_min.z;
			if (b2.m_max.x > m_max.x) m_max.x = b2.m_max.x;
			if (b2.m_max.y > m_max.y) m_max.y = b2.m_max.y;
			if (b2.m_max.z > m_max.z) m_max.z = b2.m_max.z;
		}

		//-- Intersect ray R(t)=origin + t * dir against AABB a. When intersecting,
		//-- return intersection distance dist and point intersecPoint of intersection.
		//-- Note: this function from book "Real-Time Collision Detection" by Christer Ericson.
		bool intersectRay(
			const vec3f& origin, const vec3f& dir, float& dist, vec3f& coll) const
		{
			float tfirst = 0.0f, tlast  = 1.0f;

			if (!raySlabIntersect(origin.x, dir.x, m_min.x, m_max.x, tfirst, tlast)) return false;
			if (!raySlabIntersect(origin.y, dir.y, m_min.y, m_max.y, tfirst, tlast)) return false;
			if (!raySlabIntersect(origin.z, dir.z, m_min.x, m_max.z, tfirst, tlast)) return false;

			coll = origin + dir.scale(tfirst);
			dist = tfirst * dir.length();

			return true;
		}
		
		//-- test if segment specified by points p0 and p1 intersects AABB b.
		//-- Note: this function from book "Real-Time Collision Detection" by Christer Ericson.
		bool intesectsSegment(const vec3f& p0, const vec3f& p1) const
		{
			vec3f c = getCenter();				//-- box center-point.
			vec3f e = m_max - c;					//-- box half length extents.
			vec3f m = (p0 + p1).scale(0.5f);	//-- segment midpoint.
			vec3f d = p1 - m;					//-- segment half length vector.
			m = m - c;							//-- translate box and segment to origin.

			//-- try world coordinate axes as separating axes.
			float adx = fabs(d.x);
			if (fabs(m.x) > e.x + adx) return false;
			float ady = fabs(d.y);
			if (fabs(m.y) > e.y + ady) return false;
			float adz = fabs(d.z);
			if (fabs(m.z) > e.z + adz) return false;

			//-- add in an epsilon term to counteract arithmetic errors when segment is
			//-- (near) parallel to a coordinate axis (see text for detail).
			adx += EPSILON; ady += EPSILON; adz += EPSILON;

			// Try cross products of segment direction vector with coordinate axes.
			if (fabs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return false;
			if (fabs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) return false;
			if (fabs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return false;

			//-- no separating axis found; segment must be overlapping AABB.
			return true;
		}

		inline void transform(const mat4f& transform)
		{
			assert(!isEmpty());

			//-- need to remember to save the size that we'll use to scale the unit axes.
			vec3f size = m_max - m_min;

			//-- calculate the new location of the minimum point.
			m_min = transform.applyToPoint(m_min);
			m_max = m_min;

			//-- go through each axis. Each component of each axis either adds to the
			//-- max or takes from the min.
			//-- here X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2
			for (uint axis = 0; axis <= 2; ++axis)
			{
				vec3f transformedAxis = transform.applyToUnitAxis(axis).scale(size[axis]);

				for (uint resultDirection = 0; resultDirection <= 2; ++resultDirection)
				{
					//-- go through each coordinate of the axis and add to the
					//-- appropriate point.
					if (transformedAxis[resultDirection] > 0)
					{
						m_max[resultDirection] += transformedAxis[resultDirection];
					}
					else
					{
						m_min[resultDirection] += transformedAxis[resultDirection];
					}
				}
			}
		}

		inline AABB getTranformed(const mat4f& tranform) const
		{
			AABB tmp(*this);
			tmp.transform(tranform);
			return tmp;
		}

		inline Outcode calculateOutcode(const mat4f& VP) const
		{
			Outcode oc = OUTCODE_MASK;

			vec4f vx[2];
			vec4f vy[2];
			vec4f vz[2];

			vx[0] = VP.getRow(0);
			vx[1] = vx[0];
			vx[0] *= m_min[0];
			vx[1] *= m_max[0];

			vy[0] = VP.getRow(1);
			vy[1] = vy[0];
			vy[0] *= m_min[1];
			vy[1] *= m_max[1];

			vz[0] = VP.getRow(2);
			vz[1] = vz[0];
			vz[0] *= m_min[2];
			vz[1] *= m_max[2];

			const vec4f& vw = VP.getRow(3);

			for (uint i = 0 ; i < 8 ; ++i)
			{
				vec4f v = vw;
				v += vx[i & 1];
				v += vy[(i >> 1) & 1];
				v += vz[(i >> 2) & 1];

				oc &= v.calculateOutcode();
			}
			return oc;
		}
	
	public:
		vec3f m_min, m_max;
	};

} // math
} // brUGE