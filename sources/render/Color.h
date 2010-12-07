#pragma once

#include "prerequisites.h"
#include "math/Vector4.h"

namespace brUGE
{
namespace render
{

	// Color in format of RGBA float[4].
	//---------------------------------------------------------------------------------------------
	struct Color
	{
		union
		{
			float color[4];
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};
		};

		inline Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f)									{}
		inline Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_), a(1.0f)			{}
		inline Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_)	{}

		inline void set(float r_, float g_, float b_, float a_)								{ r = r_; g = g_; b = b_; a = a_; }
		inline void set(const float* rgba)													{ r = rgba[0]; g = rgba[1]; b = rgba[2]; a = rgba[3]; }				
		
		inline operator const vec4f() const													{ return vec4f(color); }
		inline operator vec4f()																{ return vec4f(color); }
		inline Color&	 operator = (const vec4f& rt)										{ set(rt.x, rt.y, rt.z, rt.w); return *this; }
		inline float&	 operator [] (int i)												{ return color[i]; }
		inline const float& operator [] (int i) const										{ return color[i]; }
	};

} // render
} // brUGE
