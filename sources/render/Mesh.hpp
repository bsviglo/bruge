#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include "render/IRenderDevice.h"
#include "render/render_system.hpp"
#include "render/materials.hpp"
#include "math/math_all.hpp"
#include "utils/BSPTree.h"
#include "utils/Data.hpp"
#include <vector>

namespace brUGE
{
namespace render
{

	// 
	//----------------------------------------------------------------------------------------------
	class Mesh : public utils::RefCount
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

			uint16							m_numIndices;
			Ptr<IBuffer>					m_IB;
			std::vector<Ptr<IBuffer>>		m_VBs;
			mutable std::vector<IBuffer*>	m_pVBs;
			Ptr<PipelineMaterial>			m_pMaterial;
			Ptr<Material>					m_sMaterial;
		};
		typedef std::vector<SubMesh> SubMeshes; 

	public:
		Mesh();
		~Mesh();

		bool		load(const utils::ROData& data, const std::string& name);
		int			instancingID() const { return m_instacingID; }
		const AABB& bounds() const { return m_aabb; }
		uint		gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const;

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
	class SkinnedMesh : public utils::RefCount
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

			uint16							m_numIndices;
			Ptr<IBuffer>					m_IB;
			std::vector<Ptr<IBuffer>>		m_VBs;
			mutable std::vector<IBuffer*>	m_pVBs;
			Ptr<PipelineMaterial>			m_pMaterial;
			Ptr<Material>					m_sMaterial;
		};
		typedef std::vector<SubMesh> SubMeshes; 

	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool				 load(const utils::ROData& data, const std::string& name);
		const AABB&			 bounds() const			{ return m_aabb; }
		const Skeleton&		 skeleton() const		{ return m_skeleton; }
		const MatrixPalette& invBindPose() const	{ return m_invBindPose; }
		uint				 gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const;

	private:
		SubMeshes	  m_submeshes;	
		Skeleton	  m_skeleton;
		MatrixPalette m_invBindPose;
		AABB		  m_aabb;
	};

} // render
} // brUGE
