#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos : SV_POSITION;

#ifdef PIN_ALPHA_TEST
	float2 tc  : TEXCOORD0;
#endif
};

#ifdef _VERTEX_SHADER_

#ifdef PIN_INSTANCED
	//-- instancing auto variable.
	tbuffer tb_auto_Instancing
	{
		float4x4 g_instances[128];
	};
#endif

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos	    : POSITION;
	float2 tc	    : TEXCOORD0;
	float3 normal   : NORMAL;
#ifdef PIN_INSTANCED
	uint   instID	: SV_InstanceID;
#endif
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

#ifdef PIN_INSTANCED
	float4 wPos = mul(float4(i.pos, 1), g_instances[i.instID]);
	o.pos		= mul(wPos, g_viewProjMat); 
#else
	o.pos = mul(float4(i.pos, 1), g_MVPMat);
#endif

#ifdef PIN_ALPHA_TEST
	o.tc  = i.tc;
#endif

	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
#ifdef PIN_ALPHA_TEST
	texture2D(float4, diffuseMap);
#endif

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
#ifdef PIN_ALPHA_TEST
	float alpha = sample2D(diffuseMap, i.tc).a;
	if (g_alphaRef >= alpha)
		discard;
#endif

	//-- shader is not used, so don't worry about output data.
	return float4(1,1,1,1);
};
	
#endif