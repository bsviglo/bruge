#include "common.hlsl"

struct vs_out
{
	float4 pos	: SV_POSITION;
	float4 cPos	: TEXCOORD1;
	float2 tc	: TEXCOORD2;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float4 pos	: POSITION;
	float2 tc	: TEXCOORD0;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;
	
	o.pos  = i.pos;
	o.cPos = i.pos;
	o.tc   = i.tc;
	
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_


#endif
