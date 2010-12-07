#include "common.hlsl"

#ifdef _VERTEX_SHADER_

	vs_out main(vs_in i)
	{
		return vs_common(i);
	};

#endif


#ifdef _FRAGMENT_SHADER_

sampler 		  diffuse_sml;
Texture2D<float4> diffuse_tex;

	float4 main(vs_out i) : SV_TARGET
	{
		float3 dir = g_worldLightPos - i.wPos;

		float3 normal = normalize(i.normal);
		dir = normalize(dir);
	
		float  light = min(0.5f, max(dot(normal, dir), 0.1f));
		float3 oCol  = diffuse_tex.Sample(diffuse_sml, i.texCoord).xyz * light;

		return float4(oCol, 1.0f);
	};
	
#endif