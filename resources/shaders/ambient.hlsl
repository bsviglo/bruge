#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 tc		: TEXCOORD0;
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
	
	o.pos = mul(float4(i.pos, 1), g_MVPMat);
	o.tc  = i.tc;

    return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);

//--------------------------------------------------------------------------------------------------
cbuffer cb_user_defined
{
	float4 g_color;
};

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
		//-- decal calculation stuff.
	float2 ssc		  = i.pos.xy * g_screenRes.zw;
	float4 decalColor = sample2D(t_auto_decalsMask, ssc);
	float4 lightsMask = sample2D(t_auto_lightsMask, ssc);
	
	float3 colorRGB = lerp(g_color.xyz, decalColor.xyz, decalColor.w);

    float3 chrom = lightsMask.rgb / (G_EPS + luminance(lightsMask.rgb));
    float3 spec  = chrom * lightsMask.a;
	
	return float4(lightsMask.rgb * colorRGB + 0.1f * spec, g_color.w);
};
	
#endif