#pragma once

#include "math_types.hpp"
#include "Quaternion.hpp"
#include "Matrix4x4.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

//-- ToDo: reconsider and delete.
#ifdef max
#	undef max
#endif
#ifdef min
#	undef min
#endif

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

	//----------------------------------------------------------------------------------------------
	inline bool almostEqual(float val0, float val1, float epsilon = 0.001f)
	{
		return (fabs(val0 - val1) < epsilon) ? true : false;
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
	template<typename T>
	inline void swap(T& left, T& right)
	{
		T tmp = left;
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
		if (tmin > tmax) swap(tmin, tmax);

		//-- ray is parallel to slab. No hit if origin not within slab.
		if (tmax < tfirst || tmin > tlast)
			return false;

		if (tmin > tfirst) tfirst = tmin;
		if (tmax < tlast)  tlast  = tmax;

		return true;
	}

	//-- decomposes matrix into quaternion as rotation matrix, scale vector and translation.
	//----------------------------------------------------------------------------------------------
	inline void decomposeMatrix(quat& oQuat, vec3f& oScale, vec3f& oTranslate, const mat4f& iMat)
	{
		const vec3f& row0 = iMat.getRowAsVec3(0);
		const vec3f& row1 = iMat.getRowAsVec3(1);
		const vec3f& row2 = iMat.getRowAsVec3(2);
		const vec3f& row3 = iMat.getRowAsVec3(3);

		oScale.x = row0.length();
		oScale.y = row1.length();
		oScale.z = row2.length();

		oTranslate = row3;

		mat4f m = iMat;
		/*
		m.preScale(1.0f / oScale.x, 1.0f / oScale.y, 1.0f / oScale.z);

		vec3f in = row0.cross(row1);
		if (in.dot(row2) < 0)
		{
			row2.scale(-1.0f);
			oScale.z *= -1;
		}
		*/
		m.transpose();
		oQuat.set(m);
	}

	//-- combine matrix from quaternion and translation.
	//----------------------------------------------------------------------------------------------
	inline mat4f combineMatrix(const quat& q, const vec3f& t)
	{
		mat4f o;

		//-- rotation.
		o = q.toMat4();
		o.transpose();

		//-- translation.
		o.m30 = t.x; o.m31 = t.y; o.m32 = t.z;

		return o;
	}

} //-- math
} //-- brUGE