#include "common.hlsl"

//--------------------------------------------------------------------------------------------------
struct vs_out
{
	float4 pos		: SV_POSITION;
	float2 tc		: TEXCOORD0;
	float3 wPos		: TEXCOORD1;
	float3 normal	: NORMAL;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TEXCOORD2;
	float3 binormal : TEXCOORD3;
#endif
};

#ifdef _VERTEX_SHADER_

#ifdef PIN_INSTANCED
	//-- instancing auto variable.
	tbuffer tb_auto_Instancing
	{
		float4x4 g_instances[128];
	};
#endif

//--------------------------------------------------------------------------------------------------
struct vs_in
{                                           
	float3 pos	    : POSITION;
	float2 tc	    : TEXCOORD0;
	float3 normal   : NORMAL;
#ifdef PIN_BUMP_MAP
	float3 tangent  : TANGENT;
	float3 binormal : BINORMAL;
#endif
#ifdef PIN_INSTANCED
	uint   instID	: SV_InstanceID;
#endif
};

//--------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
	vs_out o;

#ifdef PIN_INSTANCED
	float4x4 worldMat = g_instances[i.instID];
#else
	float4x4 worldMat = g_worldMat;
#endif

	float4 wPos = mul(float4(i.pos, 1), worldMat);

	o.normal    = mul(float4(i.normal, 0), worldMat);
	o.wPos	    = wPos;
	o.pos	    = mul(wPos, g_viewProjMat);
	o.tc	    = i.tc;

#ifdef PIN_BUMP_MAP
	o.tangent  = mul(float4(i.tangent, 0),  worldMat);
	o.binormal = mul(float4(i.binormal, 0), worldMat);
#endif

	return o;
};

#endif


#ifdef _FRAGMENT_SHADER_

//--------------------------------------------------------------------------------------------------
#ifdef PIN_ALPHA_TEST
	texture2D(float4, diffuseMap);
#endif

#ifdef PIN_BUMP_MAP
	texture2D(float4, bumpMap);
#endif

//--------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
#ifdef PIN_ALPHA_TEST
	float alpha = sample2D(diffuseMap, i.tc).a;
	if (g_alphaRef >= alpha)
		discard;
#endif

	float  dist = length(i.wPos - g_cameraPos.xyz);

#ifdef PIN_BUMP_MAP
	float3 nn   = (2.0f * sample2D(bumpMap, i.tc).xyz - float3(1,1,1));
	float3 norm = normalize(nn.x * i.tangent + nn.y * i.binormal + nn.z * i.normal);
#else
	float3 norm = normalize(i.normal);
#endif

	return float4(norm.x, norm.y, 0.0f, dist);
};
	
#endif