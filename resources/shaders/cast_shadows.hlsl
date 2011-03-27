#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos : SV_POSITION;
	float2 tc  : TEXCOORD0;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos	    : POSITION;
	float2 tc	    : TEXCOORD0;
	float3 normal   : NORMAL;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
#endif
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;
	o.pos = mul(float4(i.pos, 1), g_MVPMat);
	o.tc  = i.tc;
	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
#ifdef PIN_ALPHA_TEST
	texture2D(float4, t_auto_diffuseMap);
#endif

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
#ifdef PIN_ALPHA_TEST
	float alpha = sample2D(t_auto_diffuseMap, i.tc).a;
	if (g_alphaRef >= alpha)
		discard;
#endif

	//-- shader is not used, so don't worry about output data.
	return float4(1,1,1,1);
};
	
#endif