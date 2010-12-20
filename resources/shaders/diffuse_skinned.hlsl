
//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos	: SV_POSITION;
	float2 tc	: TEXCOORD0;
};

//-- 16 byte aligned.
struct Weight
{
	float4 joint; //-- .x   - joint index .yzw - unused (padding).
	float4 pos;   //-- .xyz - position .w - weight 
};

//-- 4 byte aligned.
tbuffer tb_weights
{
	Weight g_weights[2048];
};

//-- 64 byte aligned.
tbuffer tb_skeleton
{
	float4x4 g_bones[256];
};

//--------------------------------------------------------------------------------------------------
float3 boneTransf(in uint idx, in float3 pos)
{
	float4x4 bone = g_bones[idx];
    return mul(pos, bone);
}

cbuffer cb_uniforms
{
	float4x4 g_worldMat;
	float4x4 g_viewProjMat;
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float3 normal		: NORMAL;
	float2 tc			: TEXCOORD0;
	uint   weightIdx	: TEXCOORD1;
	uint   weightCount	: TEXCOORD2;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in input)
{
    vs_out o;
    
    float3 worldPos = float3(0,0,0);
    for (uint i = 0; i < input.weightCount; ++i)
    {
		Weight weight = g_weights[i + input.weightIdx];
		worldPos += weight.pos.w * boneTransf(weight.joint.x, weight.pos.xyz); 
    }
    
	o.pos = mul(worldPos, g_viewProjMat);
	o.tc  = input.tc;

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

sampler 		  diffuse_sml;
Texture2D<float4> diffuse_tex;

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{
	return float4(diffuse_tex.Sample(diffuse_sml, i.tc.xy).xyz, 0.0f);
}

#endif
