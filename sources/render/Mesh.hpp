#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "render/IRenderDevice.h"
#include "render/Renderer.hpp"
#include "render/materials.hpp"
#include "math/math_all.hpp"
#include "utils/Data.hpp"
#include <vector>

namespace brUGE
{
namespace render
{

	// 
	//----------------------------------------------------------------------------------------------
	class Mesh
	{
	public:
		//------------------------------------------------------------------------------------------
		struct SubMesh
		{
			SubMesh() : m_numIndices(0) { }

			struct Desc
			{
				Desc() : m_name{0}, m_numVertices(0) { }

				struct Stream
				{
					Stream() : m_elemSize(0) { }

					uint8				m_elemSize;
					std::vector<byte>	m_vertices;
				};

				std::array<char, 20>	m_name;
				uint16					m_numVertices;
				std::vector<uint16>		m_indices;
				std::vector<Stream>		m_streams;
			};

			uint16									m_numIndices;
			std::shared_ptr<IBuffer>				m_IB;
			std::vector<std::shared_ptr<IBuffer>>	m_VBs;
			mutable std::vector<IBuffer*>			m_pVBs;
			std::shared_ptr<PipelineMaterial>		m_pMaterial;
			std::shared_ptr<Material>				m_sMaterial;
		};
		typedef std::vector<SubMesh> SubMeshes; 

	public:
		Mesh();
		~Mesh();

		bool		load(const utils::ROData& data, const std::string& name);
		int			instancingID() const { return m_instacingID; }
		const AABB& bounds() const { return m_aabb; }
		uint		gatherROPs(Renderer::EPassType pass, bool instanced, RenderOps& ops) const;

	private:
		SubMeshes m_submeshes;
		AABB	  m_aabb;
		int		  m_instacingID;
	};


	//--
	//----------------------------------------------------------------------------------------------
	struct Joint
	{
	public:
		struct Transform
		{
			quat  m_orient;
			vec3f m_pos;
		};

	public:
		char m_name[20];
		int	 m_parent;
	};
	typedef std::vector<Joint>				Skeleton;
	typedef std::vector<Joint::Transform>	TransformPalette;
	typedef std::vector<mat4f>				MatrixPalette;
	typedef std::vector<mat4f>				InvBindPose;


	//--
	//----------------------------------------------------------------------------------------------
	class SkinnedMesh
	{
	public:
		//------------------------------------------------------------------------------------------
		struct SubMesh
		{
			struct Desc
			{
				struct Stream
				{
					uint8				m_elemSize;
					std::vector<byte>	m_vertices;
				};

				char					m_name[20];
				uint16					m_numVertices;
				std::vector<uint16>		m_indices;
				std::vector<Stream>		m_streams;
			};

			uint16									m_numIndices;
			std::shared_ptr<IBuffer>				m_IB;
			std::vector<std::shared_ptr<IBuffer>>	m_VBs;
			mutable std::vector<IBuffer*>			m_pVBs;
			std::shared_ptr<PipelineMaterial>		m_pMaterial;
			std::shared_ptr<Material>				m_sMaterial;
		};
		typedef std::vector<SubMesh> SubMeshes; 

	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool				 load(const utils::ROData& data, const std::string& name);
		const AABB&			 bounds() const			{ return m_aabb; }
		const Skeleton&		 skeleton() const		{ return m_skeleton; }
		const MatrixPalette& invBindPose() const	{ return m_invBindPose; }
		uint				 gatherROPs(Renderer::EPassType pass, bool instanced, RenderOps& ops) const;

	private:
		SubMeshes	  m_submeshes;	
		Skeleton	  m_skeleton;
		MatrixPalette m_invBindPose;
		AABB		  m_aabb;
	};

} // render
} // brUGE
