#include "common.hlsl"

struct vs_out
{
	float4 pos		: SV_POSITION;	
	float4 color	: COLOR0;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float3 pos		: POSITION;
	float4 color	: TEXCOORD0;
};

vs_out main(vs_in i)
{
    vs_out o;
	o.pos.x = (2.0f * i.pos.x * g_screenRes.z) - 1.0f;
	o.pos.y = (2.0f * i.pos.y * g_screenRes.w) - 1.0f;
	o.pos.z = 0.0f;
	o.pos.w = 1.0f;
	o.color	   = i.color;
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

float4 main(vs_out i) : SV_TARGET
{	
    return i.color;
}

#endif
