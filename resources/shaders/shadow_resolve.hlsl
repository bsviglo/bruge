#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
cbuffer constants
{
	float4   g_splitPlanes;
	float4   g_splitCount;
	float4x4 g_shadowMat0;
	float4x4 g_shadowMat1;
	float4x4 g_shadowMat2;
	float4x4 g_shadowMat3;
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
texture2D(float, g_shadowMap0);
texture2D(float, g_shadowMap1);
texture2D(float, g_shadowMap2);
texture2D(float, g_shadowMap3);

static float g_shadowBias = 0.0025f;

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	//-- recustruct world space position of the pixel.
	float  viewZ = sample2D(t_auto_depthMap, i.tc).w;
	float3 vDir  = normalize(i.vDir);
	float3 pixelWorldPos = g_cameraPos + vDir * viewZ;

	float zDist = mul(float4(pixelWorldPos, 1.0f), g_viewMat).z;
	float isInShadow = 0;

	if (zDist < g_splitPlanes.x)
	{
		//-- transform it to the light projection space.
		float4 pixelLightSpace = mul(float4(pixelWorldPos, 1.0f), g_shadowMat0);
		pixelLightSpace.xyz /= pixelLightSpace.w;

		//-- calculate shadow space texture coordinates and pixel's depth.
		float2 shadowTC = CS2TS(pixelLightSpace.xy);
		float  shadowD  = pixelLightSpace.z - g_shadowBias;

		//-- lets compare our calculated depth with the depth saved in the shadow map.
		isInShadow = (shadowD > sample2D(g_shadowMap0, shadowTC).x);
	}
	else if (zDist < g_splitPlanes.y)
	{
		//-- transform it to the light projection space.
		float4 pixelLightSpace = mul(float4(pixelWorldPos, 1.0f), g_shadowMat1);
		pixelLightSpace.xyz /= pixelLightSpace.w;

		//-- calculate shadow space texture coordinates and pixel's depth.
		float2 shadowTC = CS2TS(pixelLightSpace.xy);
		float  shadowD  = pixelLightSpace.z - g_shadowBias;

		//-- lets compare our calculated depth with the depth saved in the shadow map.
		isInShadow = (shadowD > sample2D(g_shadowMap1, shadowTC).x);
	}
	else if (zDist < g_splitPlanes.z)
	{
		//-- transform it to the light projection space.
		float4 pixelLightSpace = mul(float4(pixelWorldPos, 1.0f), g_shadowMat2);
		pixelLightSpace.xyz /= pixelLightSpace.w;

		//-- calculate shadow space texture coordinates and pixel's depth.
		float2 shadowTC = CS2TS(pixelLightSpace.xy);
		float  shadowD  = pixelLightSpace.z - g_shadowBias;

		//-- lets compare our calculated depth with the depth saved in the shadow map.
		isInShadow = (shadowD > sample2D(g_shadowMap2, shadowTC).x);
	}
	else
	{
		//-- transform it to the light projection space.
		float4 pixelLightSpace = mul(float4(pixelWorldPos, 1.0f), g_shadowMat3);
		pixelLightSpace.xyz /= pixelLightSpace.w;

		//-- calculate shadow space texture coordinates and pixel's depth.
		float2 shadowTC = CS2TS(pixelLightSpace.xy);
		float  shadowD  = pixelLightSpace.z - g_shadowBias;

		//-- lets compare our calculated depth with the depth saved in the shadow map.
		isInShadow = (shadowD > sample2D(g_shadowMap3, shadowTC).x);
	}

	return float4(isInShadow, 0, 0, 0);
}
	
#endif