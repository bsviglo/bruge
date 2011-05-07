#include "terrain_common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos : SV_POSITION;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

	//-- restore real position from i.xy and i.z
	float4 combinedPos = float4(i.xz.x, i.y, i.xz.y, 1.0f);
	combinedPos.xz += g_posOffset.xy;

	o.pos = mul(combinedPos, g_viewProjMat);

	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	//-- shader is not used, so don't worry about output data.
	return float4(1,1,1,1);
};
	
#endif