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
		struct Vertex
		{
			vec3f pos;
			vec2f texCoord;
			vec3f normal;
		};

		struct Tangent
		{
			vec3f tangent;
			vec3f binormal;
		};	

		struct Face
		{
			vec3us index;
		};

		struct SubMesh
		{
			SubMesh() : indicesCount(0), primTopolpgy(PRIM_TOPOLOGY_TRIANGLE_LIST) { }

			uint16				  indicesCount;
			std::vector<Face>	  faces;
			std::vector<Vertex>	  vertices;
			std::vector<Tangent>  tangents;

			//-- gpu hardware buffers. 
			EPrimitiveTopology	  primTopolpgy;
			Ptr<IBuffer>		  mainVB;
			Ptr<IBuffer>		  tangentVB;
			Ptr<IBuffer>		  IB;
			Ptr<PipelineMaterial> material;

			bool buildBuffers(bool useNVTriStipOptimization = false);
			void buildTangents();
			void buildNormals();	
		};
		typedef std::vector<SubMesh*> SubMeshes; 

	public:
		Mesh();
		~Mesh();

		void		attach(SubMesh* submesh) { m_submeshes.push_back(submesh); }
		bool		build();
		const AABB& bounds() const { return m_aabb; }
		uint		gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const;

	private:
		SubMeshes m_submeshes;
		AABB	  m_aabb;
	};
	
	//-- auxiliary function helps us to create BSP from mesh.
	Ptr<utils::BSPTree> createBSPFromMesh(const Ptr<Mesh>& mesh);
	
	
	//
	//----------------------------------------------------------------------------------------------
	struct Joint
	{
	public:
		typedef std::vector<mat4f> WorldPalette;

		struct Transform
		{
			vec3f pos;
			quat  orient;
		};
		typedef std::vector<Transform> Transforms;

		//-- Needed to achieve 16 byte aligned structure, because GPU can work only with 16 byte
		//-- aligned structures.
		struct GPUTransform
		{
			vec4f pos;
			quat  orient;
		};

	public:
		std::string m_name;
		int			m_parentIdx;
		Transform	m_transform;

	public:
		inline vec3f transform(const vec3f& point) const
		{
			return m_transform.orient.rotate(point) + m_transform.pos;
		}

		inline vec3f invTransform(const vec3f& point) const
		{
			return m_transform.orient.getConjugated().rotate(point - m_transform.pos);
		}
	};
	typedef std::vector<Joint> Joints; 


	//----------------------------------------------------------------------------------------------
	class SkinnedMesh : public utils::RefCount
	{
	public:
		struct Vertex
		{
			vec3f normal;
			vec2f texCoord;
			uint  weightIdx;
			uint  weightCount;
		};

		struct Tangent
		{
			vec3f tangent;
			vec3f binormal;
		};	

		struct Face
		{
			vec3us index;
		};

		struct Weight
		{
			uint  joint;
			float weight;
			vec3f pos;
		};

		//-- Needed to achieve 16 byte aligned structure, because GPU can work only with 16 byte
		//-- aligned structures.
		struct GPUWeight
		{
			vec4f joint;  //-- .x - joint index, .yzw - padding
			vec4f weight; //-- .xyz - position, .w - weight. 
		};

		typedef std::vector<vec3f> Positions;

		struct SubMesh
		{
			SubMesh() : indicesCount(0), primTopolpgy(PRIM_TOPOLOGY_TRIANGLE_LIST) { }
		
			uint16				  indicesCount;
			std::vector<Face>	  faces;
			std::vector<Vertex>	  vertices;
			std::vector<Tangent>  tangents;
			std::vector<Weight>	  weights;

			//-- gpu hardware buffers.
			EPrimitiveTopology	  primTopolpgy;
			Ptr<IBuffer>		  mainVB;
			Ptr<IBuffer>		  tangentVB;
			Ptr<IBuffer>		  IB;
			Ptr<IBuffer>		  weightsTB;
			Ptr<PipelineMaterial> material;

			bool buildBuffers  ();
			void buildTangents (const Positions& positions);
			void buildNormals  (const Positions& positions);	
			void buildPositions(Positions& positions, const Joints& joints);
		};
		typedef std::vector<SubMesh*> SubMeshes; 

	public:
		SkinnedMesh();
		~SkinnedMesh();

		bool		  load(const utils::ROData& data);
		bool		  build();
		const AABB&   bounds() const	 { return m_originAABB; }
		const Joints& skeleton() const   { return m_joints; }
		uint		  gatherROPs(RenderSystem::EPassType pass, bool instanced, RenderOps& ops) const;

	private:
		bool loadJoints (const utils::ROData& data);
		bool loadSubMesh(const utils::ROData& data);

	private:
		SubMeshes	 m_submeshes;	
		Joints		 m_joints;
		AABB		 m_originAABB;
	};

} // render
} // brUGE
