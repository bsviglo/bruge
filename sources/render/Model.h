#pragma once

#include "prerequisites.h"
#include "math/Matrix4x4.h"
#include "math/AABB.h"
#include "utils/BSPTree.h"
#include "utils/Ptr.h"
#include "utils/Data.hpp"
#include "render/Mesh.hpp"

namespace brUGE
{
namespace render
{
	
	//
	//----------------------------------------------------------------------------------------------
	class Model : public utils::RefCount
	{
	public:
		virtual ~Model() { }

		virtual bool		 playAnim(const char* /*name*/, bool /*isLooped*/) { return false; }
		virtual bool		 stopAnim() { return false; }
		virtual void		 drawSkeleton() { }

		virtual const mat4f& transform() const = 0;
		virtual void		 transform(const mat4f& m4) = 0;
		virtual const AABB&	 localBounds() const = 0;
		virtual const AABB&	 worldBounds() const = 0;
		
		virtual void tick(float /*dt*/) { }

		//-- Because shader in terms of brUGE engine is not just the place where stored shader code,
		//-- but much more, for example, this shader store all information about passed to it
		//-- shader constants, textures, samplers and so on.
		virtual void draw(
						VertexLayoutID vertexLayout, IShader* shader, bool bumped = false
						) = 0;

		virtual void drawInstanced(
						VertexLayoutID vertexLayout, IShader* shader,
						uint numInstances, bool bumped = false
						) = 0;

		virtual bool intersects(
						float& dist, vec3f& out, vec3f& normal,
						const vec3f& startPos, const vec3f& dir,
						float maxDist = 1000.0f
						) const = 0;
	};


	//
	//----------------------------------------------------------------------------------------------
	class SkinnedModel : public Model
	{
	public:
		SkinnedModel(const Ptr<SkinnedMesh>& mesh, const Ptr<SkeletonCollider>* collider = NULL);
		virtual ~SkinnedModel();

		virtual bool		 playAnim(const char* name, bool isLooped = false);
		virtual bool		 stopAnim();
		virtual void		 drawSkeleton();

		virtual const mat4f& transform() const;
		virtual void		 transform(const mat4f& m4);
		virtual const AABB&	 localBounds() const;
		virtual const AABB&	 worldBounds() const;

		virtual void tick(float dt);
		
		virtual void draw(
			VertexLayoutID vertexLayout, IShader* shader, bool bumped = false
			);

		virtual void drawInstanced(
			VertexLayoutID vertexLayout, IShader* shader,
			uint numInstances, bool bumped = false
			);

		virtual bool intersects(
			float& dist, vec3f& out, vec3f& normal,
			const vec3f& startPos, const vec3f& dir, float maxDist
			) const;

	private:
		mat4f				  m_worldMat;
		AABB				  m_localAABB;
		AABB				  m_worldAABB;
		Nodes				  m_nodes;
		Joint::Transforms	  m_skeleton;
		Ptr<SkeletonCollider> m_collider;
		Ptr<SkinnedMesh>	  m_mesh;

		struct AnimData
		{
			bool		   m_isLooped;
			float		   m_animTime;
			Ptr<Animation> m_animation;
		};
		AnimData			 m_animData;		
	};


	//
	//----------------------------------------------------------------------------------------------
	class StaticModel : public Model
	{
	public:
		StaticModel(const Ptr<Mesh>& mesh, const Ptr<utils::BSPTree> bsp = NULL);
		virtual ~StaticModel();
	
		virtual const mat4f& transform() const;
		virtual void		 transform(const mat4f& m4);

		virtual const AABB&	localBounds() const;
		virtual const AABB&	worldBounds() const;

		virtual void draw(
			VertexLayoutID vertexLayout, IShader* shader, bool bumped
			);

		virtual void drawInstanced(
			VertexLayoutID vertexLayout, IShader* shader,
			uint numInstances, bool bumped
			);

		virtual bool intersects(
			float& dist, vec3f& out, vec3f& normal,
			const vec3f& startPos, const vec3f& dir, float maxDist
			) const;
		
	private:
		Nodes				m_nodes;
		mat4f				m_worldMat;
		AABB				m_worldAABB;
		Ptr<Mesh>			m_mesh;
		Ptr<utils::BSPTree> m_bsp;
	};

} // render
} // brUGE