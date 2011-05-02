#include "terrain_common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;	
	float2 tc		: TEXCOORD0;
	float3 normal	: TEXCOORD1;
	float3 wPos		: TEXCOORD2;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

	//-- restore real position from i.xy and i.z
	float4 combinedPos = float4(i.xz.x, i.y, i.xz.y, 1.0f);
	combinedPos.xz += g_posOffset.xy;

	o.wPos	 = combinedPos;
	o.pos	 = mul(combinedPos, g_viewProjMat);
	o.normal = i.normal;

	//-- ToDo: implement.
	o.tc	 = i.tc;

	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float  dist = length(i.wPos - g_cameraPos.xyz);
	float3 norm = normalize(i.normal);

	return float4(norm.x, norm.y, 0.0f, dist);
};
	
#endif