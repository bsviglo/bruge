#pragma once

#include "math_types.h"
#include "Vector3.h"
#include "Matrix4x4.h"

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
		inline AABB(const AABB& rt) : min(rt.min), max(rt.max) { }
		inline ~AABB() {}

		AABB& operator = (const AABB& rt)
		{
			set(rt.min, rt.max);
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
			if (min.x < max.x)	return false;
			if (min.y < max.y)	return false;
			if (min.z < max.z)	return false;
			return true;
		}

		inline Vector3<float> getCenter() const
		{
			return (min + max).scale(0.5f);
		}

		inline Vector3<float> getDimensions() const
		{
			return max - min;
		}

		inline void set(float minx, float miny, float minz, float maxx, float maxy, float maxz)
		{
			this->min.set(minx, miny, minz);
			this->max.set(maxx, maxy, maxz);
		}

		inline void set(const vec3f& min, const vec3f& max)
		{
			this->min = min;
			this->max = max;
		}

		inline void include(const vec3f& point)
		{
			if (point.x < min.x) min.x = point.x;
			if (point.x > max.x) max.x = point.x;
			if (point.y < min.y) min.y = point.y;
			if (point.y > max.y) max.y = point.y;
			if (point.z < min.z) min.z = point.z;
			if (point.z > max.z) max.z = point.z;
		}

		inline void combine(const AABB& b2)
		{
			if (b2.min.x < min.x) min.x = b2.min.x;
			if (b2.min.y < min.y) min.y = b2.min.y;
			if (b2.min.z < min.z) min.z = b2.min.z;
			if (b2.max.x > max.x) max.x = b2.max.x;
			if (b2.max.y > max.y) max.y = b2.max.y;
			if (b2.max.z > max.z) max.z = b2.max.z;
		}

		//-- Intersect ray R(t)=origin + t * dir against AABB a. When intersecting,
		//-- return intersection distance dist and point intersecPoint of intersection.
		//-- Note: this function from book "Real-Time Collision Detection" by Christer Ericson.
		bool intersectRay(
			const vec3f& origin, const vec3f& dir, float& dist, vec3f& coll) const
		{
			float tfirst = 0.0f, tlast  = 1.0f;

			if (!raySlabIntersect(origin.x, dir.x, min.x, max.x, tfirst, tlast)) return false;
			if (!raySlabIntersect(origin.y, dir.y, min.y, max.y, tfirst, tlast)) return false;
			if (!raySlabIntersect(origin.z, dir.z, min.x, max.z, tfirst, tlast)) return false;

			coll = origin + dir.scale(tfirst);
			dist = tfirst * dir.length();

			return true;
		}
		
		//-- test if segment specified by points p0 and p1 intersects AABB b.
		//-- Note: this function from book "Real-Time Collision Detection" by Christer Ericson.
		bool intesectsSegment(const vec3f& p0, const vec3f& p1) const
		{
			vec3f c = getCenter();				//-- box center-point.
			vec3f e = max - c;					//-- box half length extents.
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
			assert(!this->isEmpty());

			//-- need to remember to save the size that we'll use to scale the unit axes.
			vec3f size = max - min;

			//-- calculate the new location of the minimum point.
			min = transform.applyToPoint(min);
			max = min;

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
						max[resultDirection] += transformedAxis[resultDirection];
					}
					else
					{
						min[resultDirection] += transformedAxis[resultDirection];
					}
				}
			}
		}

		inline AABB getTranformed(const mat4f& tranform)
		{
			AABB tmp(*this);
			tmp.transform(tranform);
			return tmp;
		}
	
	public:
		vec3f min, max;
	};

} // math
} // brUGE