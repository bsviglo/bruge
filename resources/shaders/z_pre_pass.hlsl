#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;
	float3 wPos		: TEXCOORD0;
	float3 wNormal	: TEXCOORD1;
	float2 tc		: TEXCOORD2;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos	  : POSITION;
	float2 tc	  : TEXCOORD0;
	float3 normal : NORMAL;
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

	float4 wPos		= mul(float4(i.pos, 1),    g_worldMat);
	float4 wNormal	= mul(float4(i.normal, 0), g_worldMat);

	o.wNormal = wNormal.xyz;
	o.wPos	  = wPos.xyz;
	o.pos	  = mul(float4(wPos.xyz, 1.0f), g_viewProjMat);
	o.tc	  = i.tc;
	
	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_diffuseMap);

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float  dist    = length(i.wPos - g_cameraPos.xyz);
	float3 wNormal = normalize(i.wNormal);
	return float4(wNormal.x, wNormal.y, 0.0f, dist);
};
	
#endif