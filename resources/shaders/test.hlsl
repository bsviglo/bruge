#include "post_processing.hlsl"

#ifdef _VERTEX_SHADER_
#endif

#ifdef _FRAGMENT_SHADER_

cbuffer cb_user
{
	float g_blendFactor;
};

texture2D(float4, g_sourceTex);

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	return float4(lerp(sample2D(g_sourceTex, i.tc).xyz, float4(1,0,0,0), g_blendFactor).xyz, 1.0f);
}

#endif