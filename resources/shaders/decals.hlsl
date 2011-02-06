#include "common.hlsl"

//-- Note: we can experiment with data packing. This is far from ideal data packing rules.
//--	   It looks like it look only for simplicity to undestading what is going here.
struct vs_out
{
	float4 pos			: SV_POSITION;
	int	   instanceId	: TEXCOORD0;
	float4 row0			: TEXCOORD1;
	float4 row1			: TEXCOORD2;
	float4 row2			: TEXCOORD3;
	float4 row3			: TEXCOORD4;
};

struct Instance
{
	float4 m_dir;
	float4 m_pos;
	float4 m_up;
	float4 m_scale;
};

//-- instancing auto variable.
tbuffer tb_auto_Instancing
{
	Instance g_instances[1024];
};

#ifdef _VERTEX_SHADER_

struct vs_in
{                                           
	float3 pos		  : POSITION;
	float2 tc		  : TEXCOORD0;
	float3 normal	  : NORMAL;
	uint   instanceId : SV_InstanceID;
};

//-------------------------------------------------------------------------------------------------
vs_out main(vs_in i)
{
    vs_out o;

	//-- 1. retrieve instance data.
	Instance inst = g_instances[i.instanceId];

	//-- 2. compute world matrix.
	//--
	//-- zaxis = normal(dir)
	//-- xaxis = normal(cross(Up, zaxis))
	//-- yaxis = cross(zaxis, xaxis)
	//--
	//-- resulting matrix will be
	//-- xaxis.x  xaxis.y  xaxis.z  0
	//-- yaxis.x  yaxis.y  yaxis.z  0
	//-- zaxis.x  zaxis.y  zaxis.z  0
	//-- pos.x	  pos.y	   pos.z	1
	//--
	//-- then we premultiply it by the scale matrix.
	//-- world = scale * rotateTranslate.

	float3 scale = inst.m_scale;
	float3 pos	 = inst.m_pos;
	float3 zAxis = inst.m_dir.xyz;
	float3 yAxis = inst.m_up.xyz;
	float3 xAxis = cross(zAxis, xAxis);

	float4x4 scaleMat = 
	{
		{scale.x, 0, 0, 0},
		{0, scale.y, 0, 0},
		{0, 0, scale.z, 0},
		{0, 0, 0,       1}
	};

	float4x4 worldMat = 
	{
		{xAxis, 0},
		{yAxis, 0},
		{zAxis, 0},
		{pos,   1}
	};

	//-- 3. final world matrix.
	worldMat = mul(scaleMat, worldMat);

	//-- 4. compute data for the decal texture projection matrix.
	//--
	//-- zaxis = normal(dir)
	//-- xaxis = normal(cross(Up, zaxis))
	//-- yaxis = cross(zaxis, xaxis)
	//--
	//-- xaxis.x           yaxis.x           zaxis.x          0
	//-- xaxis.y           yaxis.y           zaxis.y          0
	//-- xaxis.z           yaxis.z           zaxis.z          0
	//-- -dot(xaxis, pos)  -dot(yaxis, pos)  -dot(zaxis, pos) l
	//--
	//-- ToDo: reconsider.
	//--
	float4x4 lookAtMat = 
	{
		{xAxis.x,			yAxis.x,			zAxis.x,			0},
		{xAxis.y,			yAxis.y,			zAxis.y,			0},
		{xAxis.z,			yAxis.z,			zAxis.z,			0},
		{-dot(xAxis, pos),  -dot(yAxis, pos),	-dot(zAxis, pos),	1}
	};

	//-- 5. Now do regular vertex shader with usage of the previous calculated data.
	float4 wPos	  = mul(float4(i.pos, 1.0f), worldMat);
	o.pos		  = mul(wPos, g_viewProjMat);
	o.instanceId  = i.instanceId;

	o.row0 = lookAtMat[0];
	o.row1 = lookAtMat[1];
	o.row2 = lookAtMat[2];
	o.row3 = lookAtMat[3];

    return o;
}

#endif

#ifdef _FRAGMENT_SHADER_

sampler 		  texSampler;
sampler			  depthSampler;
Texture2D<float4> diffTex;
Texture2D<float>  depthTex;

//-------------------------------------------------------------------------------------------------
float4 main(vs_out i) : SV_TARGET
{	
	//-- convert to texture coordinates.
	float2 texCoord;
	texCoord.x = 0.5f + 0.5f * i.pos.x;
	texCoord.y = 0.5f - 0.5f * i.pos.y;

	//-- read depth
	float pixelDepth = depthTex.Sample(depthSampler, texCoord.xy);

	//-- transform to world space
	float4 pixelWorldPos  = mul(float4(cPos.x, cPos.y, pixelDepth, 1.0f), invViewProjMat);
	pixelWorldPos /= pixelWorldPos.w;

	//-- calculate texture coordinates for projection texture.
	float4 pixelClipPosInTexSpace = mul(pixelWorldPos, g_positions[i.instanceId].viewProjMat);
	pixelClipPosInTexSpace /= pixelClipPosInTexSpace.w;

	//-- convert to texture coordinates.
	texCoord.x = 0.5f + 0.5f * pixelClipPosInTexSpace.x;
	texCoord.y = 0.5f - 0.5f * pixelClipPosInTexSpace.y;

	float4 oColor = diffTex.Sample(texSampler, texCoord.xy);

	return oColor + float4(g_positions[i.instanceId].color.rgb, 0.0f) * 0.5f * oColor.a;
}

#endif
