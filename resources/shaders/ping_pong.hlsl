#include "post_processing.hlsl"

#ifdef _VERTEX_SHADER_

#endif

#ifdef _FRAGMENT_SHADER_

texture2D(float4, g_sourceTex)

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	return sample2D(g_sourceTex, i.tc);
}

#endif