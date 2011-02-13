#pragma once

#include "prerequisites.h"
#include "math/Vector4.h"

namespace brUGE
{
namespace render
{

	// Color in format of RGBA as a float4.
	//---------------------------------------------------------------------------------------------
	struct Color
	{
		float r, g, b, a;

		inline explicit		Color	()											: r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
		inline				Color	(float r_, float g_, float b_)				: r(r_), g(g_), b(b_), a(1.0f) {}
		inline				Color	(float r_, float g_, float b_, float a_)	: r(r_), g(g_), b(b_), a(a_) {}
		inline				Color	(uint32 abgr)								{ setABGR(abgr); }

		inline void			set		(float r_, float g_, float b_, float a_)	{ r = r_; g = g_; b = b_; a = a_; }
		inline void			set		(const float* rgba)							{ r = rgba[0]; g = rgba[1]; b = rgba[2]; a = rgba[3]; }				
		inline void			set		(const vec3f& v3)							{ r = v3.x; g = v3.y; b = v3.z; a = 1.0f; }
		inline void			set		(const vec4f& v4)							{ r = v4.x; g = v4.y; b = v4.z; a = v4.w; }
		inline void			setRGBA	(uint32 rgba)
		{
			r = ((rgba>>24) & 0xff) / 255.0f;
			g = ((rgba>>16) & 0xff) / 255.0f;
			b = ((rgba>>8 ) & 0xff) / 255.0f;
			a = ((rgba>>0 ) & 0xff) / 255.0f;
		}
		inline void			setABGR	(uint32 abgr)
		{
			r = ((abgr>>0 ) & 0xff) / 255.0f;
			g = ((abgr>>8 ) & 0xff) / 255.0f;
			b = ((abgr>>16) & 0xff) / 255.0f;
			a = ((abgr>>24) & 0xff) / 255.0f;
		}
		
		inline vec4f		toVec4	() const	{ return vec4f(r, g, b, a); }
		inline vec3f		toVec3	() const	{ return vec3f(r, g, b); }
		inline const float* toPtr	() const	{ return &r; }
	};

} // render
} // brUGE
