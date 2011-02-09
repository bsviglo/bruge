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
		float3 dir = -g_worldLightPos + i.wPos;

		float3 normal = normalize(i.normal);
		dir = normalize(dir);
	
		float  light = max(dot(normal, dir), 0.05f);
		float3 oCol  = t_auto_diffuseMap_tex.Sample(t_auto_diffuseMap_sml, i.texCoord).xyz * light;

		return float4(oCol, 1.0f);
	};
	
#endif