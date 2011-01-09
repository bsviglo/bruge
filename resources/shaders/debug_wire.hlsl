struct vs_out
{
	float4 pos	 : SV_POSITION;	
	float4 color : COLOR0;
};

#ifdef _VERTEX_SHADER_

//-- per frame auto variables.
cbuffer cb_auto_PerFrame
{
	float4x4 g_viewMat;
	float4x4 g_viewProjMat;
};

struct vs_in
{                                           
	float3 pos	 : POSITION;
	float4 color : TEXCOORD0;
};

vs_out main(vs_in i)
{
    vs_out o;
	o.pos	= mul(float4(i.pos, 1.0f), g_viewProjMat);
	o.color	= i.color;
    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

float4 main(vs_out i) : SV_TARGET
{	
    return i.color;
}

#endif
