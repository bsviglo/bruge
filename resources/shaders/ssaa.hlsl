#include "post_processing.hlsl"

#ifdef _VERTEX_SHADER_
#endif

#ifdef _FRAGMENT_SHADER_

cbuffer cb_user
{
	float g_filterWidth;
	float g_threshold;
	float g_debug;
};

texture2D(float4, g_sourceTex)


//-- calculate luma for desired rgb color.
//-------------------------------------------------------------------------------------------------
float lumRGB(float3 color)
{
   return dot(color, float3(0.212f, 0.716f, 0.072f));
}

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
    float w = g_filterWidth;
    float t = lumRGB(sample2D(g_sourceTex, i.tc + float2(+0.0, -1.0) * w * g_screenRes.zw).xyz);
	float l = lumRGB(sample2D(g_sourceTex, i.tc + float2(-1.0, +0.0) * w * g_screenRes.zw).xyz);
	float r = lumRGB(sample2D(g_sourceTex, i.tc + float2(+1.0, +0.0) * w * g_screenRes.zw).xyz);
	float b = lumRGB(sample2D(g_sourceTex, i.tc + float2(+0.0, +1.0) * w * g_screenRes.zw).xyz);
 
    float2 n  = float2(-(t - b), r - l);
    float  nl = length(n);
	
	if (g_debug)
	{
		n /= nl;
		return float4(step(nl, g_threshold) * float3(0.5f + n * 0.5f, 0), 1);
	}
	else
	{
		if	(nl < g_threshold)
		{
			return sample2D(g_sourceTex, i.tc);
		}
		else
		{
			n *= g_screenRes.zw / nl;
		 
			float4 o  = sample2D(g_sourceTex, i.tc);
			float4 t0 = sample2D(g_sourceTex, i.tc + n * 0.5f) * 0.9f;
			float4 t1 = sample2D(g_sourceTex, i.tc - n * 0.5f) * 0.9f;
			float4 t2 = sample2D(g_sourceTex, i.tc + n) * 0.75f;
			float4 t3 = sample2D(g_sourceTex, i.tc - n) * 0.75f;
		 
			return (o + t0 + t1 + t2 + t3) / 4.3f;
		}
	}
}

#endif