#include "terrain_common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 maskUV	: TEXCOORD0;
	float2 layerUV	: TEXCOORD1;
};


#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

	//-- restore real position from i.xy and i.z
	float4 combinedPos = float4(i.xz.x, i.y, i.xz.y, 1.0f);
	combinedPos.xz += g_posOffset.xy;

	o.pos = mul(combinedPos, g_viewProjMat);
	//-- ToDo: implement.
	o.maskUV  = (i.tc + g_texOffset.xy) * g_texOffset.zw;
	o.layerUV = i.tc * 4.0f;

	return o;
}

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, maskMap);
texture2D(float4, layerMap1);
texture2D(float4, layerMap2);
texture2D(float4, layerMap3);
texture2D(float4, layerMap4);
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);
texture2D(float4, t_auto_shadowsMask);

//--------------------------------------------------------------------------------------------------
float4 blendLayers(in float2 maskUV, in float2 layerUV)
{
	float4 oColor = float4(0,0,0,0);
	float4 mask   = sample2D(maskMap, maskUV);

	oColor += sample2D(layerMap1, layerUV) * mask.x;
	oColor += sample2D(layerMap2, layerUV) * mask.y;
	oColor += sample2D(layerMap3, layerUV) * mask.z;
	oColor += sample2D(layerMap4, layerUV) * mask.w;

	return oColor;
}

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float2 ssc		   = i.pos.xy * g_screenRes.zw;

	//-- ToDo: replace with real texturing.
	//float4 srcColor    = sample2D(maskMap,  i.tc);
	float4 srcColor    = blendLayers(i.maskUV, i.layerUV);

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