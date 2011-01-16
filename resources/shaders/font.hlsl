#include "common.hlsl"

struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR0;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float4 pos		: POSITION;
	float2 texCoord	: TEXCOORD0;
	float4 color	: COLOR0;
};

vs_out main(vs_in i)
{
    vs_out o;
	o.texCoord = i.texCoord;
	o.pos	   = i.pos;
	o.color	   = i.color;
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

sampler 		 fontSampl;
Texture2D<float> fontTex;

float4 main(vs_out i) : SV_TARGET
{	
    return float4(i.color.xyz, i.color.w * fontTex.Sample(fontSampl, i.texCoord));
}

#endif
