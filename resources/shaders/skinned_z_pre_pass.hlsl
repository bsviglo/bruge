#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos	: SV_POSITION;
	float3 wPos	: TEXCOORD0;
	float2 tc	: TEXCOORD1;
	float3 normal	: TEXCOORD2;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TEXCOORD3;
	float3 binormal : TEXCOORD4;
#endif
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

//-------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 normal		: NORMAL;
	float2 tc		: TEXCOORD0;
	uint   weightIdx	: TEXCOORD1;
	uint   weightCount	: TEXCOORD2;
#ifdef PIN_BUMP_MAP
	float3 tangent  	: TANGENT;
	float3 binormal 	: BINORMAL;
#endif
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
    
	o.pos    = mul(float4(worldPos, 1.0f), g_viewProjMat);
	o.wPos   = worldPos;
	o.normal = mul(float4(input.normal, 0.0f), g_worldMat);
	o.tc     = input.tc;
	
#ifdef PIN_BUMP_MAP
	o.tangent  = mul(float4(input.tangent, 0),  g_worldMat);
	o.binormal = mul(float4(input.binormal, 0), g_worldMat);
#endif

	return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

//-------------------------------------------------------------------------------------------------
#ifdef PIN_ALPHA_TEST
	texture2D(float4, diffuseMap);
#endif

#ifdef PIN_BUMP_MAP
	texture2D(float4, bumpMap);
#endif

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
#ifdef PIN_ALPHA_TEST
	float alpha = sample2D(diffuseMap, i.tc).a;
	if (g_alphaRef >= alpha)
		discard;
#endif

	float  dist = length(i.wPos - g_cameraPos.xyz);

#ifdef PIN_BUMP_MAP
	float3 nn   = (2.0f * sample2D(bumpMap, i.tc).xyz - float3(1,1,1));
	float3 norm = normalize(nn.x * i.tangent + nn.y * i.binormal + nn.z * i.normal);
#else
	float3 norm = normalize(i.normal);
#endif

	return float4(norm, dist);
};
	
#endif