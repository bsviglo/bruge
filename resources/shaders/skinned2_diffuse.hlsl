#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos	: SV_POSITION;
	float2 tc	: TEXCOORD0;
};

//--------------------------------------------------------------------------------------------------
tbuffer tb_auto_MatrixPalette
{
	float4x4 g_bones[100];
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{            
	float3 pos		: POSITION;
	float3 normal	: NORMAL;
	float2 tc		: TEXCOORD0;
	uint3  joints	: TEXCOORD1;
	float3 weights	: TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
    
    float3 worldPos = float3(0,0,0);
    for (uint j = 0; j < 3; ++j)
    {
		float4x4 bone   = g_bones[i.joints[j]];
		float    weight = i.weights[j];

		worldPos += mul(float4(i.pos, 1.0f), bone).xyz * weight;
    }
    
	o.pos = mul(float4(worldPos, 1.0f), g_viewProjMat);
	o.tc  = i.tc;

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

texture2D(float4, diffuseMap);
texture2D(float4, t_auto_decalsMask);
texture2D(float4, t_auto_lightsMask);
texture2D(float4, t_auto_shadowsMask);

//-------------------------------------------------------------------------------------------------
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
}

#endif
