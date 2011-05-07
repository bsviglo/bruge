#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos : SV_POSITION;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos	    : POSITION;
	float2 tc	    : TEXCOORD0;
	float3 normal   : NORMAL;
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;
	o.pos = mul(float4(i.pos, 1), g_MVPMat);
	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	discard;
	return float4(1,1,1,1);
};
	
#endif