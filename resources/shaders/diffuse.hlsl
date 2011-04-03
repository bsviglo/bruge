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
#ifdef PIN_BUMP_MAP
	float3 tangent	: TANGENT;
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
}

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_diffuseMap);
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);
texture2D(float4, t_auto_shadowsMask);

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float2 ssc		   = i.pos.xy * g_screenRes.zw;
	float4 srcColor    = sample2D(t_auto_diffuseMap,  i.tc);
	float4 decalColor  = sample2D(t_auto_decalsMask,  ssc);
	float4 lightsMask  = sample2D(t_auto_lightsMask,  ssc);
	float4 shadowsMask = sample2D(t_auto_shadowsMask, ssc);
		
	//-- calculate decals factor.
	float3 colorRGB = lerp(srcColor.xyz, decalColor.xyz, decalColor.w);

	//-- calculate lighting factor.
	float3 chrom = lightsMask.rgb / (G_EPS + luminance(lightsMask.rgb));
	float3 spec  = chrom * lightsMask.a;

	colorRGB = lightsMask.rgb * colorRGB + 0.1f * spec;

	//-- calculate shadows factor.
	colorRGB *= max(0.25f, 1.0f - shadowsMask.x);
	
	return float4(colorRGB, 1.0f);
};
	
#endif