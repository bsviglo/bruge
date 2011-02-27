#include "common.hlsl"

#ifdef _VERTEX_SHADER_

vs_out_common main(vs_in_common i)
{
	return vs_common(i);
};

#endif


#ifdef _FRAGMENT_SHADER_

texture2D(float4, t_auto_diffuseMap);
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);

float4 main(vs_out_common i) : SV_TARGET
{
	//-- decal calculation stuff.
	float2 ssc		  = i.pos.xy * g_screenRes.zw;
	float4 srcColor   = sample2D(t_auto_diffuseMap, i.texCoord.xy);
	float4 decalColor = sample2D(t_auto_decalsMask, ssc);
	float4 lightsMask = sample2D(t_auto_lightsMask, ssc);
	
	float3 colorRGB = lerp(srcColor.xyz, decalColor.xyz, decalColor.w);

    float3 chrom = lightsMask.rgb / (G_EPS + luminance(lightsMask.rgb));
    float3 spec  = chrom * lightsMask.a;
	
	return float4(lightsMask.rgb * colorRGB + 0.1f * spec, 1.0f);
};
	
#endif