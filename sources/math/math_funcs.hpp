#pragma once

#include "math_types.hpp"
#include "Vector4.hpp"

namespace brUGE
{
namespace math
{

	//----------------------------------------------------------------------------------------------
	template<typename T>
	inline const T& max(const T &a, const T &b) { return (a > b) ? a : b; }

	//----------------------------------------------------------------------------------------------
	template<typename T>
	inline const T& min(const T &a, const T &b) { return (a < b) ? a : b; }

	//-- check is a float value almost zero or not?
	//----------------------------------------------------------------------------------------------
	inline bool almostZero(float val, float epsilon = 0.004f)
	{
		return (fabs(val) < epsilon) ? true : false;
	}

	//-- clamp a generic value to the range [minVal, maxVal].
	//----------------------------------------------------------------------------------------------
	template<typename T>
	inline T clamp(T minVal, T curVal, T maxVal)
	{
		curVal = max(minVal, curVal);
		curVal = min(maxVal, curVal);
		return curVal;
	}

	//-- swaps its arguments.
	//----------------------------------------------------------------------------------------------
	inline void swapf(float& left, float& right)
	{
		float tmp = left;
		left  = right;
		right = tmp;
	}

	//-- from deg/rad to rad/deg conversion functions.
	//----------------------------------------------------------------------------------------------
	inline float degToRad(float angle) { return angle * static_cast<float>(DEG_to_RAD); }
	inline float radToDeg(float angle) { return angle * static_cast<float>(RAD_to_DEG); }

	//-- make collision against two parallel planes (called slab) of the complex collision object
	//-- like OBB or AABB or any other convex hull.
	//-- See for more details: http://www.gamedev.net/community/forums/topic.asp?topic_id=346956
	//----------------------------------------------------------------------------------------------
	inline bool raySlabIntersect(
		float start, float dir, float min, float max, float& tfirst, float& tlast)
	{
		if (almostZero(dir, EPSILON))
		{
			return (start < max && start > min);
		}

		float tmin = (min - start) / dir;
		float tmax = (max - start) / dir;
		if (tmin > tmax) swapf(tmin, tmax);

		//-- ray is parallel to slab. No hit if origin not within slab.
		if (tmax < tfirst || tmin > tlast)
			return false;

		if (tmin > tfirst) tfirst = tmin;
		if (tmax < tlast)  tlast  = tmax;

		return true;
	}

} //-- math
} //-- brUGE