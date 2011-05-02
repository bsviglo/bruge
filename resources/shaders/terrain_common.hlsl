#include "common.hlsl"

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
cbuffer cb_PerTerrainSector
{
	float4 g_posOffset;
	float4 g_texOffset;
};

//--------------------------------------------------------------------------------------------------
struct vs_in
{
	float  y		: POSITION;
	float3 normal	: NORMAL;

	float2 xz		: TEXCOORD0;
	float2 tc		: TEXCOORD1;
};

#endif