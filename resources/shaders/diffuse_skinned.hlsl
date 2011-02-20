#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos	: SV_POSITION;
	float2 tc	: TEXCOORD0;
};

//-- 16 byte aligned.
struct Weight
{
	float4 joint; //-- .x   - joint index .yzw - unused (padding).
	float4 pos;   //-- .xyz - position .w - weight 
};

//-- 4 byte aligned.
tbuffer tb_auto_Weights
{
	Weight g_weights[2048];
};

//-- 64 byte aligned.
tbuffer tb_auto_MatrixPalette
{
	float4x4 g_bones[256];
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
float3 boneTransf(in uint idx, in float3 pos)
{
	float4x4 bone = g_bones[idx];
    return mul(float4(pos, 1.0f), bone).xyz;
}

struct vs_in
{                                           
	float3 normal		: NORMAL;
	float2 tc			: TEXCOORD0;
	uint   weightIdx	: TEXCOORD1;
	uint   weightCount	: TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in input)
{
    vs_out o;
    
    float3 worldPos = float3(0,0,0);
    for (uint i = 0; i < input.weightCount; ++i)
    {
		Weight weight = g_weights[i + input.weightIdx];
		worldPos += weight.pos.w * boneTransf(weight.joint.x, weight.pos.xyz); 
    }
    
	o.pos = mul(float4(worldPos, 1.0f), g_viewProjMat);
	o.tc  = input.tc;

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

texture2D(float4, t_auto_diffuseMap);
texture2D(float4, t_auto_decalsMask);

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float2 ssc		  = i.pos.xy * g_screenRes.zw;
	float4 srcColor   = sample2D(t_auto_diffuseMap, i.tc.xy);
	float4 decalColor = sample2D(t_auto_decalsMask, ssc);
	
	float3 outRGB     = lerp(srcColor.xyz, decalColor.xyz, decalColor.w);
	
	return float4(outRGB, srcColor.w);
}

#endif
