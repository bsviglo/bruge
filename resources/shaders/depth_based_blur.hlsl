#include "common.hlsl"

//-- vertex 2 fragment.
//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos	: SV_POSITION;	
	float2 tc	: TEXCOORD0;
};


#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{
	float3 pos	: POSITION;
	float2 tc	: TEXCOORD0;
};


//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;
	o.pos = float4(i.pos, 1.0f);
	o.tc  = i.tc;
	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_depthMap);
texture2D(float4, g_srcMap);

//-- Gaussian 3x3 filter.
static float  g_depthThreshold = 0.05f;
static float  g_blurRadius = 2.5f;
static uint   g_samplesCount = 9;
static float3 g_samples[] = 
{
	float3(+0, +0, 0.25f),
	float3(-1, -1, 0.0625f),
	float3(-1, +0, 0.125f),
	float3(-1, +1, 0.0625f),
	float3(+0, -1, 0.125f),
	float3(+0, +1, 0.125f),
	float3(+1, -1, 0.0625f),
	float3(+1, +0, 0.125f),
	float3(+1, +1, 0.0625f)
};

//--------------------------------------------------------------------------------------------------
float4 main(vs_out input) : SV_TARGET
{
	float ret = 0;

	//-- retrieve origin sample's depth.
	float  originDepth  = sample2D(t_auto_depthMap, input.tc).w;
	float4 originSample = sample2D(g_srcMap, input.tc);

	for (uint i = 0; i < g_samplesCount; ++i)
	{
		float3 sample = g_samples[i];
		float2 uv	  = input.tc + sample.xy * g_screenRes.zw * g_blurRadius;
		float  weight = sample.z;
		
		//-- do depth comparision.
		float sampleDepth = sample2D(t_auto_depthMap, uv).w;

		//-- don't take into account samples which are failed depth comparison test.
		if (abs(sampleDepth - originDepth) > g_depthThreshold)
		{
			ret += originSample * weight;
		}
		else
		{
			ret += sample2D(g_srcMap, uv) * weight;
		}
	}

	return ret;
};
	
#endif