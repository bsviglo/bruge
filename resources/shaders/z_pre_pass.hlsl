#include "common.hlsl"

struct vs_out
{
	float4 pos  : SV_POSITION;
	float4 cPos : TEXCOORD0;
	float2 tc	: TEXCOORD1;
};

#ifdef _VERTEX_SHADER_

vs_out main(vs_in_common i)
{
	vs_out o;

	float4 wPos	= mul(float4(i.pos, 1), g_worldMat);
	o.pos		= mul(wPos, g_viewProjMat);
	o.cPos 		= o.pos;
	o.tc   		= i.texCoord;
	
	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

texture2D(float4, t_auto_diffuseMap);

float4 main(vs_out i) : SV_TARGET
{
	return i.cPos.z / i.cPos.w;
};
	
#endif