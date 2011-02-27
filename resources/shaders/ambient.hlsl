#include "common.hlsl"

#ifdef _VERTEX_SHADER_

	vs_out_common main(vs_in_common i)
	{
		return vs_common(i);
	};

#endif


#ifdef _FRAGMENT_SHADER_

	float4 main(vs_out_common i) : SV_TARGET
	{
		return float4(0.5f, 0.5f, 0.5f, 1.0f);
	};
	
#endif