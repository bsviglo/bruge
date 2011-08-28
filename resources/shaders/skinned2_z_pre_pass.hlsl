#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;
	float3 wPos		: TEXCOORD0;
	float2 tc		: TEXCOORD1;
	float3 normal	: TEXCOORD2;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TEXCOORD3;
	float3 binormal : TEXCOORD4;
#endif
};

//--------------------------------------------------------------------------------------------------
tbuffer tb_auto_MatrixPalette
{
	float4x4 g_bones[100];
};

#ifdef _VERTEX_SHADER_

//-------------------------------------------------------------------------------------------------
struct vs_in
{                            
	float3 pos		: POSITION;
	float3 normal	: NORMAL;
	float2 tc		: TEXCOORD0;
	uint3  joints	: TEXCOORD1;
	float3 weights	: TEXCOORD2;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
#endif
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
    
	float3 worldPos	     = float3(0,0,0);
	float3 worldNormal   = float3(0,0,0);

#ifdef PIN_BUMP_MAP
	float3 worldTangent  = float3(0,0,0);
	float3 worldBinormal = float3(0,0,0);
#endif

	for (uint j = 0; j < 3; ++j)
	{
		float4x4 bone   = g_bones[i.joints[j]];
		float    weight = i.weights[j];

		worldPos	  += mul(float4(i.pos, 1.0f), bone).xyz * weight;
		worldNormal   += mul(i.normal, (float3x3)bone) * weight;

#ifdef PIN_BUMP_MAP
		worldTangent  += mul(i.tangent, (float3x3)bone) * weight;
		worldBinormal += mul(i.binormal, (float3x3)bone) * weight;
#endif
	}
    
	o.wPos   = worldPos;
	o.pos    = mul(float4(worldPos, 1.0f), g_viewProjMat);
	o.normal = worldNormal;
	o.tc     = i.tc;
	
#ifdef PIN_BUMP_MAP
	o.tangent  = worldTangent;
	o.binormal = worldBinormal;
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