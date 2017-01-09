#pragma once

#include "prerequisites.hpp"
#include <array>

namespace brUGE
{
	//-- Note: to guaranty compact one byte aligned packing.
#pragma pack(push, 1)

	//----------------------------------------------------------------------------------------------
	struct StaticMeshFormat
	{
		struct Header
		{
			Header() : m_format{0}, m_version(0) { }

			std::array<char, 20>	m_format;
			uint32					m_version;
		};

		struct Info
		{
			Info() : m_numSubMeshes(0), m_format{0}, m_formatSize(0), m_aabb{0.0f} { }

			uint8					m_numSubMeshes;
			std::array<char, 20>	m_format;
			uint8					m_formatSize;
			std::array<float, 6>	m_aabb;
		};

		struct SubInfo
		{
			SubInfo() : m_name{0}, m_numVertices(0), m_numVertexStreams(0), m_numIndices(0) { }

			std::array<char, 20>	m_name;
			uint16					m_numVertices;
			uint8					m_numVertexStreams;
			uint16					m_numIndices;
		};

		struct VertexStream
		{
			VertexStream() : m_elemSize(0) { }

			uint8					m_elemSize;
		};
	};


	//----------------------------------------------------------------------------------------------
	struct SkinnedMeshFormat
	{
		struct Header
		{
			char   m_format[20];
			uint32 m_version;
		};

		struct Skeleton
		{
			struct Info
			{
				uint8 m_numJoints;
			};

			struct Joint
			{
				char  m_name[20];
				int8  m_parent;
			};
		};

		struct InvBindPose
		{
			float m_matrix[16];
		};

		struct Info
		{
			uint8 m_numSubMeshes;
			char  m_format[20];
			uint8 m_formatSize;
			float m_aabb[6];
		};

		struct SubInfo
		{
			char   m_name[20];
			uint16 m_numVertices;
			uint8  m_numVertexStreams;
			uint16 m_numIndices;
		};

		struct VertexStream
		{
			uint8 m_elemSize;
		};
	};

	//----------------------------------------------------------------------------------------------
	struct SkinnedMeshAnimationFormat
	{
		struct Header
		{
			char m_format[20];
			uint m_version;
		};

		struct Skeleton
		{
			struct Info
			{
				uint8 m_numJoints;
			};

			struct Joint
			{
				char  m_name[20];
				int8  m_parent;
			};
		};

		struct Info
		{
			uint16 m_numFrames;
			uint8  m_frameRate;
		};

		struct Bound
		{
			float m_aabb[6];
		};

		struct Joint
		{
			float m_quat[4];
			float m_pos[3];
		};
	};

#pragma pack(pop)

} //-- brUGE
