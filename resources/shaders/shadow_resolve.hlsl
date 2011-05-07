#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
cbuffer g_shadowConstants
{
	float2   g_shadowMapRes;
	float2   g_invShadowMapRes;
	float4   g_splitPlanes;
	float4x4 g_shadowMatrices[4];
};

//-- vertex 2 fragment.
//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos	: SV_POSITION;	
	float2 tc	: TEXCOORD0;
	float3 vDir	: TEXCOORD1;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{
	float3 pos	: POSITION;
	float2 tc	: TEXCOORD0;
};

//--------------------------------------------------------------------------------------------------
vs_out main(in vs_in i)
{
	vs_out o;
	o.pos = float4(i.pos, 1.0f);
	o.tc  = i.tc;

	//-- calculate world space camera to vertex direction.
	float4 wPos = mul(float4(i.pos.xy, g_farNearPlane.x, 1.0f), g_invViewProjMat);
	wPos.xyz /= wPos.w;

	o.vDir = (wPos.xyz - g_cameraPos);

	return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_depthMap);
texture2D(float,  g_shadowMap);
texture2D(float4, g_noiseMap);

static uint PCF_NUM_SAMPLES = 16;

static float2 poissonDisk[] =
{ 
   float2( -0.94201624, -0.39906216 ), 
   float2( 0.94558609, -0.76890725 ), 
   float2( -0.094184101, -0.92938870 ),
   float2( 0.34495938, 0.29387760 ), 
   float2( -0.91588581, 0.45771432 ), 
   float2( -0.81544232, -0.87912464 ), 
   float2( -0.38277543, 0.27676845 ), 
   float2( 0.97484398, 0.75648379 ), 
   float2( 0.44323325, -0.97511554 ), 
   float2( 0.53742981, -0.47373420 ), 
   float2( -0.26496911, -0.41893023 ), 
   float2( 0.79197514, 0.19090188 ), 
   float2( -0.24188840, 0.99706507 ), 
   float2( -0.81409955, 0.91437590 ), 
   float2( 0.19984126, 0.78641367 ), 
   float2( 0.14383161, -0.14100790 ) 
};

//-- do PCF filter based on poisson disk with 16 samples.
//-------------------------------------------------------------------------------------------------
float PCF_filter(float2 shadowUV, float zReceiver, float filterRadiusUV) 
{ 
    float  sum	  = 0.0f; 
	float2 adjust = g_invShadowMapRes.xy * filterRadiusUV * float2(0.25f, 1.0f);

    for (int i = 0; i < PCF_NUM_SAMPLES; ++i) 
    { 
		sum += (zReceiver > sample2D(g_shadowMap, shadowUV + poissonDisk[i] * adjust).x);
    } 
    return sum / PCF_NUM_SAMPLES; 
}

//-------------------------------------------------------------------------------------------------
float customFilter(in float2 shadowUV, in float2 screenUV, in float zReceiver, in float filterRadiusUV)
{
	//-- get noise from texture.
	float2 noiseUV = screenUV * (g_screenRes.xy / float2(256,256));
	float2 noise   = 2.0 * sample2D(g_noiseMap, noiseUV).xy - float2(1,1);

	float2 adjust =  g_invShadowMapRes.xy * filterRadiusUV * float2(0.25f, 1.0f);

	float2 dx	  = float2(noise.x, 0.0f);
	float2 dy	  = float2(0.0f, noise.y);
	float2 dxdy_p = (dx + dy);
	float2 dxdy_n = (dx - dy);

	float result =
		(zReceiver > sample2D(g_shadowMap, shadowUV + dx * adjust).x) + 
		(zReceiver > sample2D(g_shadowMap, shadowUV - dx * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV + dy * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV - dy * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV + dxdy_p * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV - dxdy_p * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV + dxdy_n * adjust).x) +
		(zReceiver > sample2D(g_shadowMap, shadowUV - dxdy_n * adjust).x);

	return result * 0.125f;
}

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	//-- recustruct world space position of the pixel.
	float  viewZ = sample2D(t_auto_depthMap, i.tc).w;
	float3 vDir  = normalize(i.vDir);
	float3 pixelWorldPos = g_cameraPos + vDir * viewZ;

	//-- find view space z value for current pixel.
	float zDist = mul(float4(pixelWorldPos, 1.0f), g_viewMat).z;

	//-- based on the distance to the camera find appropriate shadow map split.
	int split = 3;
	if		(zDist < g_splitPlanes.x)	split = 0;
	else if (zDist < g_splitPlanes.y)	split = 1;
	else if (zDist < g_splitPlanes.z)	split = 2;

	//-- transform it to the light projection space.
	float4 pixelLightSpace = mul(float4(pixelWorldPos, 1.0f), g_shadowMatrices[split]);
	pixelLightSpace.xyz /= pixelLightSpace.w;

	//-- calculate shadow space texture coordinates and pixel's depth.
	float2 shadowTC = CS2TS(pixelLightSpace.xy) * float2(0.25f, 1.0f) + float2(0.25f, 0.0f) * split;
	float  shadowD  = pixelLightSpace.z;

	//-- lets compare our calculated depth with the depth saved in the shadow map.
#if 0
	float isInShadow = (shadowD > sample2D(g_shadowMap, shadowTC).x);
#endif

	//-- calculate PCF filter width based on the distance to the camera and distance in light space
	//-- between stored in shadow map depth and current pixel depth.
#if 1
	float filterFadeSpeed = 1.5f;
	float filterWidth = 1.0f * abs(g_farNearPlane.y - filterFadeSpeed * zDist) / g_farNearPlane.y;

	float isInShadow = PCF_filter(shadowTC, shadowD, filterWidth);
#endif

	//-- custom filter with helps of noise texture.
#if 0
	float isInShadow = customFilter(shadowTC, i.tc, shadowD, 1.5f);
#endif

	return float4(isInShadow, 0, 0, 0);
}
	
#endif	