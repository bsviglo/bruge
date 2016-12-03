#include "common.hlsl"

float4x4 g_transform;

struct vs_out
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float2 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	float4 color	: COLOR0;
};

vs_out main(vs_in i)
{
    vs_out o;
	o.pos   = mul(float4(i.pos.xy, 0, 1), g_transform);
	o.uv    = i.uv;
	o.color	= i.color;
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

bool g_useTexture;
texture2D(float4, g_texture);

float4 main(vs_out i) : SV_TARGET
{	
    return i.color * sample2D(g_texture, i.uv);
}

#endif