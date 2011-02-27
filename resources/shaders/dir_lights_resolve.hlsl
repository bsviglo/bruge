#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct DirLight
{
	float4 m_dir;
	float4 m_color;
};

//--------------------------------------------------------------------------------------------------
tbuffer tb_auto_Instancing
{
	DirLight g_lights[16];
};

//-- vertex 2 fragment.
//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;	
	float3 vDir		: TEXCOORD0;
	uint   instID	: TEXCOORD1;
};

#ifdef _VERTEX_SHADER_

//--------------------------------------------------------------------------------------------------
struct vs_in
{
	float3 pos		: POSITION;
	float2 texCoord	: TEXCOORD0;
	uint   instID 	: SV_InstanceID;
};

//--------------------------------------------------------------------------------------------------
vs_out main(in vs_in i)
{
	vs_out o;
	o.pos 	 = float4(i.pos, 1.0f);
	o.instID = i.instID;

	//-- calculate world space camera to vertex direction.
	float4 wPos = mul(float4(i.pos.xy, g_farNearPlane.x, 1.0f), g_invViewProjMat);
	wPos.xyz /= wPos.w;

	o.vDir = (wPos.xyz - g_cameraPos);
	
	return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
texture2D(float4, t_auto_depthMap);

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	float2 	 tc    = i.pos.xy * g_screenRes.zw;
	float4 	 nz    = sample2D(t_auto_depthMap, tc);
	DirLight light = g_lights[i.instID];
	
	float4 c = light.m_color;
	float3 l = light.m_dir.xyz;
    float3 v = normalize(-i.vDir);
    float3 h = normalize(l + v);
	float3 n = float3(nz.xy, sqrt(1.0f - nz.x * nz.x - nz.y * nz.y));
	
    float diff = max(0.1f, dot(l, n));
    float lum  = luminance(c.rgb);
    float spec = lum * pow(max(0.0f, dot(h, n)), 20.0f);
	
	return float4(diff * c.rgb, spec);
}
	
#endif