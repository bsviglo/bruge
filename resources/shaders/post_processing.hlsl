#include "common.hlsl"

struct vs_out
{
	float4 pos	: SV_POSITION;
	float2 tc	: TEXCOORD0;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float3 pos	: POSITION;
	float2 tc	: TEXCOORD0;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
	
	o.pos  = float4(i.pos, 1.0f);
	o.tc   = i.tc;
	
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_


#endif
