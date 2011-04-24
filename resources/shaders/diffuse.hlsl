#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 tc		: TEXCOORD0;
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
	float3 pos		: POSITION;
	float2 tc		: TEXCOORD0;
	float3 normal	: NORMAL;
#ifdef PIN_INSTANCED
	uint   instID	: SV_InstanceID;
#endif
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

#ifdef PIN_INSTANCED
	float4x4 worldMat = g_instances[i.instID];
#else
	float4x4 worldMat = g_worldMat;
#endif

	float4 wPos = mul(float4(i.pos, 1), worldMat);
	o.pos		= mul(wPos, g_viewProjMat); 

	o.tc  = i.tc;

	return o;
}

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, diffuseMap);
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);
texture2D(float4, t_auto_shadowsMask);

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float2 ssc		   = i.pos.xy * g_screenRes.zw;
	float4 srcColor    = sample2D(diffuseMap,  i.tc);
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