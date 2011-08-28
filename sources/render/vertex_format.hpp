#pragma once

#include "prerequisites.hpp"
#include "math/Vector2.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

namespace brUGE
{
namespace render
{

//-- Note: to guaranty compact one byte aligned packing.
#pragma pack(push, 1)

	//-- position only.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZ
	{
		vec3f m_pos;
	};

	//-- position and color.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZC
	{
		vec3f m_pos;
		vec4f m_color;
	};

	//-- position and normal.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZN
	{
		vec3f m_pos;
		vec3f m_normal;
	};
	
	//-- position and texture coordinates.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZUV
	{
		vec3f m_pos;
		vec2f m_tc;
	};

	//-- position, normal and texture coordinates.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZNUV
	{
		vec3f m_pos;
		vec3f m_normal;
		vec2f m_tc;
	};

	//-- position, normal, texture coordinates, tangent and binormal.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZNUVTB
	{
		vec3f m_pos;
		vec3f m_normal;
		vec2f m_tc;
		vec3f m_tangent;
		vec3f m_binormal;
	};

	//-- position, normal, texture coordinates, 3 indices and 3 weights.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZNUVI3W3
	{
		vec3f  m_pos;
		vec3f  m_normal;
		vec2f  m_tc;
		vec3ui m_joints;
		vec3f  m_weights;
	};

	//-- position, normal, texture coordinates, 3 indices, 3 weights, tangent and binormal.
	//----------------------------------------------------------------------------------------------
	struct VertexXYZNUVI3W3TB
	{
		vec3f  m_pos;
		vec3f  m_normal;
		vec2f  m_tc;
		vec3ui m_joints;
		vec3f  m_weights;
		vec3f  m_tangent;
		vec3f  m_binormal;
	};

	//-- xz-position, and texture coordinates. Used for terrain.
	//----------------------------------------------------------------------------------------------
	struct VertexXZUV
	{
		vec2f m_xz;
		vec2f m_uv;
	};

	//-- y-position and normal. Used for terrain.
	//----------------------------------------------------------------------------------------------
	struct VertexYN
	{
		float m_y;
		vec3f m_normal;
	};

#pragma pack(pop)

} //-- render
} //-- brUGE