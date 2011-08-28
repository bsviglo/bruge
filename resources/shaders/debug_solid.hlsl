#include "common.hlsl"

//-------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;
	float4 color	: TEXCOORD2;
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
	o.color		= inst.m_color;	
	
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{	
	return i.color;
}
#endif
