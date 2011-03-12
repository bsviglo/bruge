#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

//-- some common defines.
#define texture1D(type, name) 	 	\
	sampler 		  name##_sml;	\
	Texture1D<type>   name##_tex;

#define texture2D(type, name) 	 	\
	sampler 		  name##_sml;	\
	Texture2D<type>   name##_tex;

#define sample1D(name, tc) name##_tex.Sample(name##_sml, tc)	
#define sample2D(name, tc) name##_tex.Sample(name##_sml, tc)

//-- Available pins.
//#define PIN_ALPHA_TEST
//#define PIN_BUMP_MAP

//-- static variables.
static float G_EPS = 0.0001f;

//-- global auto variables.
cbuffer cb_auto_Global
{
	float4 g_screenRes;
	float4 g_farNearPlane;
};

//-- per frame auto variables.
cbuffer cb_auto_PerFrame
{
	float4   g_cameraPos;
	float4x4 g_viewMat;
	float4x4 g_invViewMat;
	float4x4 g_viewProjMat;
	float4x4 g_invViewProjMat;
	float4x4 g_lastViewProjMat;
	float4x4 g_invLastViewProjMat;
};

//-- per instance auto variables.
cbuffer cb_auto_PerInstance
{
	float4x4 g_worldMat;
	float4x4 g_MVPMat;
	float4x4 g_MVMat;
	float    g_alphaRef;
};

#ifdef _VERTEX_SHADER_


#endif

#ifdef _FRAGMENT_SHADER_

//-- compute luminance value for input RGB color.
//--------------------------------------------------------------------------------------------------
float luminance(in float3 rgb)
{
	return dot(rgb, float3(0.3f, 0.59f, 0.11f));
}

#endif

#endif //-- _COMMON_HLSL_