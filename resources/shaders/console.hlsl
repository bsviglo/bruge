#include "common.hlsl"

struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 texCoord	: TEXCOORD0;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float3 pos		: POSITION;
	float2 texCoord	: TEXCOORD0;
};

vs_out main(vs_in i)
{
    vs_out o;
	o.texCoord	= i.texCoord;
	o.pos		= float4(i.pos, 1.0);
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

sampler 		  conSampl;
Texture2D<float4> conTex;

float4 main(vs_out i) : SV_TARGET
{	
    return conTex.Sample(conSampl, i.texCoord);
}

#endif
