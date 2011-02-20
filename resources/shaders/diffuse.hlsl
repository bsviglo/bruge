#include "common.hlsl"

#ifdef _VERTEX_SHADER_

vs_out_common main(vs_in_common i)
{
	return vs_common(i);
};

#endif


#ifdef _FRAGMENT_SHADER_

texture2D(float4, t_auto_diffuseMap);
texture2D(float4, t_auto_decalsMask);

float4 main(vs_out_common i) : SV_TARGET
{
	//-- decal calculation stuff.
	float2 ssc		  = i.pos.xy * g_screenRes.zw;
	float4 srcColor   = sample2D(t_auto_diffuseMap, i.texCoord.xy);
	float4 decalColor = sample2D(t_auto_decalsMask, ssc);
	
	float3 outRGB     = lerp(srcColor.xyz, decalColor.xyz, decalColor.w);
	
	//--
	float3 dir = -g_worldLightPos + i.wPos;

	float3 normal = normalize(i.normal);
	dir = normalize(dir);

	float  light = max(dot(normal, dir), 0.15f);
	float3 oCol  = outRGB * light;

	return float4(oCol, 1.0f);
};
	
#endif