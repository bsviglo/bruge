#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define texture1D(type, name) 	 	\
	sampler 		  name##_sml;	\
	Texture1D<type>   name##_tex;

#define texture2D(type, name) 	 	\
	sampler 		  name##_sml;	\
	Texture2D<type>   name##_tex;

#define sample1D(name, tc) name##_tex.Sample(name##_sml, tc)	
#define sample2D(name, tc) name##_tex.Sample(name##_sml, tc)

//-- ToDo: reconsider.
static const float3 g_worldLightPos = {4.0f, 30.0f, 50.0f};

//-- per frame auto variables.
cbuffer cb_auto_PerFrame
{
	float4x4 g_viewMat;
	float4x4 g_viewProjMat;
	float4x4 g_invViewProjMat;
	float4x4 g_lastViewProjMat;
	float4x4 g_invLastViewProjMat;
	float4	 g_screenRes;
};

//-- per instance auto variables.
cbuffer cb_auto_PerInstance
{
	float4x4 g_worldMat;
	float4x4 g_MVPMat;
};

//-- vertex 2 fragment.
struct vs_out_common
{
	float4 pos		: SV_POSITION;	
	float2 texCoord	: TEXCOORD0;
	float3 wPos		: TEXCOORD1;
	float3 normal	: TEXCOORD2;
};

#ifdef _VERTEX_SHADER_

struct vs_in_common
{                                           
	float3 pos		: POSITION;
	float2 texCoord	: TEXCOORD0;
	float3 normal	: NORMAL;
};

vs_out_common vs_common(vs_in_common i)
{
    vs_out_common o;
	
	float4 wPos	= mul(float4(i.pos, 1), g_worldMat);
	o.wPos		= wPos.xyz;
	o.pos		= mul(wPos, g_viewProjMat);
	o.normal	= mul(float4(i.normal, 0), g_worldMat).xyz;
	o.texCoord	= i.texCoord;

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

	//-- pass
	
#endif

#endif //-- _COMMON_HLSL_