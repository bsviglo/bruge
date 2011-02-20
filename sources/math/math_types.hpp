#pragma once

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
	//-- this type represents an outcode that is used for clipping.
	//----------------------------------------------------------------------------------------------
	typedef unsigned int Outcode;

	const Outcode OUTCODE_LEFT		= 0x01;	//-- beyond left plane.
	const Outcode OUTCODE_RIGHT		= 0x02;	//-- beyond right plane.
	const Outcode OUTCODE_BOTTOM	= 0x04;	//-- beyond bottom plane.
	const Outcode OUTCODE_TOP		= 0x08;	//-- beyond top plane.
	const Outcode OUTCODE_NEAR		= 0x10;	//-- beyond near plane.
	const Outcode OUTCODE_FAR		= 0x20;	//-- beyond far plane.
	const Outcode OUTCODE_MASK		= OUTCODE_LEFT | OUTCODE_RIGHT | OUTCODE_BOTTOM | OUTCODE_TOP | OUTCODE_NEAR | OUTCODE_FAR;	//-- combination of all outcodes.


	class Plane;
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

	using math::Plane;
	using math::OBB;
	using math::AABB;
	using math::quat;

} // brUGE
