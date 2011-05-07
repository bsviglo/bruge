#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos	: SV_POSITION;	
	float2 tc	: TEXCOORD0;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos		: POSITION;
	float2 tc		: TEXCOORD0;
	float3 normal	: NORMAL;
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

	i.pos.y -= 0.0f;
	
	o.pos = mul(float4(i.pos, 1), g_envTransform).xyww;
	o.tc  = i.tc;

    return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

texture2D(float4, skyMap);

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	return sample2D(skyMap, i.tc);
};
	
#endif