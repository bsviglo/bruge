#pragma once

#include "math.h"

#define	PI			 3.141592653589793238462643
#define	PI_2		 6.283185307179586476925287
#define RAD_to_DEG	 57.29577951308232087679815
#define DEG_to_RAD	 0.017453292519943295769237
#define E			 2.718281828458563411277850
#define INFINITY	 1e7f
#define ONE_DIV_255  0.003921568627450980392156
#define EPSILON		 0.0001f

namespace brUGE
{
namespace math
{
	class OBB;
	class AABB;
	class Quaternion;

	template<class T>
	class Matrix4x4;

	template<class T>
	class Vector2;

	template<class T>
	class Vector3;

	template<class T>
	class Vector4;

	typedef Matrix4x4<float>		mat4f;
	typedef Matrix4x4<int>			mat4i;
	typedef Matrix4x4<unsigned int>	mat4ui;
	
	typedef Vector4<float>			vec4;
	typedef Vector3<float>			vec3;
	typedef Vector2<float>			vec2;

	typedef Vector4<float> 			vec4f;
	typedef Vector3<float> 			vec3f;
	typedef Vector2<float> 			vec2f;

	typedef Vector4<unsigned int> 	vec4ui;
	typedef Vector3<unsigned int> 	vec3ui;
	typedef Vector2<unsigned int> 	vec2ui;

	typedef Vector4<unsigned short> vec4us;
	typedef Vector3<unsigned short> vec3us;
	typedef Vector2<unsigned short> vec2us;

	typedef Vector4<int> 			vec4i;
	typedef Vector3<int> 			vec3i;
	typedef Vector2<int> 			vec2i;

	typedef Vector4<float> 			color4f;
	typedef Vector3<float> 			color3f;
	typedef Vector4<unsigned int> 	color4ui;
	typedef Vector3<unsigned int> 	color3ui;

	typedef Vector4<unsigned char>	vec4b;
	typedef Vector3<unsigned char>	vec3b;
	typedef Vector2<unsigned char>	vec2b;

	typedef Quaternion				quat;

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
		
} // math
	
	// most used math types.
	using math::vec2f;
	using math::vec3f;
	using math::vec4f;

	using math::vec2i;
	using math::vec3i;
	using math::vec4i;

	using math::vec2ui;
	using math::vec3ui;
	using math::vec4ui;

	using math::vec2us;
	using math::vec3us;
	using math::vec4us;

	using math::vec2b;
	using math::vec3b;
	using math::vec4b;

	using math::mat4f;
	using math::mat4i;
	using math::mat4ui;

	using math::OBB;
	using math::AABB;
	using math::quat;

} // brUGE
