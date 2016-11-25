#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos	: SV_POSITION;
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
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
    
    float3 worldPos = float3(0,0,0);

	[unroll]
    for (uint j = 0; j < 3; ++j)
    {
		float4x4 bone   = g_bones[i.joints[j]];
		float    weight = i.weights[j];

		if (weight == 0.0f) continue;

		worldPos += mul(float4(i.pos, 1.0f), bone).xyz * weight;
    }
    
	o.pos = mul(float4(worldPos, 1.0f), g_viewProjMat);

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	//-- shader is not used, so don't worry about output data.
	return float4(1,1,1,1);
};
	
#endif