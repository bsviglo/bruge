#include "common.hlsl"

#ifdef _VERTEX_SHADER_

	vs_out_common main(vs_in_common i)
	{
		return vs_common(i);
	};

#endif


#ifdef _FRAGMENT_SHADER_

sampler 		  t_auto_diffuseMap_sml;
Texture2D<float4> t_auto_diffuseMap_tex;

	float4 main(vs_out_common i) : SV_TARGET
	{
		return i.pos.z / i.pos.w;
	};
	
#endif