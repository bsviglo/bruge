#include "common.hlsl"

//-------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;
	float3 normal	: TEXCOORD0;
	float4 color	: TEXCOORD1;
};

#ifdef _VERTEX_SHADER_

//-------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos		  : POSITION;
	float2 tc		  : TEXCOORD0;
	float3 normal	  : NORMAL;
	uint   instanceId : SV_InstanceID;
};

//-------------------------------------------------------------------------------------------------
struct Instance
{
	float4x4 m_worldMat;
	float4	 m_color;
};

//-- instancing auto variable.
tbuffer tb_auto_Instancing
{
	Instance g_instances[128];
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
	
	Instance inst = g_instances[i.instanceId];
	
	float4 wPos	= mul(float4(i.pos, 1.0f), inst.m_worldMat);
	o.pos		= mul(wPos, g_viewProjMat);
	o.normal	= mul(float4(i.normal, 0.0f), inst.m_worldMat);
	o.color		= inst.m_color;	
	
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

static const float3 g_sunLight = float3(1.0f, 1.0f, 1.0f);

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{	
	float shade = max(dot(normalize(g_sunLight), normalize(i.normal)), 0.f) + 0.25f;

	return float4(i.color.rgb * shade, i.color.a);
}
#endif
