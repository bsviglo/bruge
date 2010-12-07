#include "Model.h"
#include "loader/ResourcesManager.h"
#include "render/IRenderDevice.h"
#include "render/RenderSystem.h"
#include "render/DebugDrawer.h"
#include "console/Console.h"

using namespace brUGE::utils;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------	
	StaticModel::StaticModel(const Ptr<Mesh>& mesh, const Ptr<BSPTree> bsp)
		: m_mesh(mesh), m_bsp(bsp)
	{
		m_worldMat.setIdentity();
		m_worldAABB = mesh->bounds();
	}

	//----------------------------------------------------------------------------------------------
	StaticModel::~StaticModel()
	{

	}

	//----------------------------------------------------------------------------------------------	
	void StaticModel::draw(
		VertexLayoutID vertexLayout, IShader* shader, bool bumped)
	{
		assert(shader != NULL);

		for (Mesh::SubMeshes::iterator iter = m_mesh->begin(); iter != m_mesh->end(); ++iter)
		{
			Mesh::SubMesh& submesh = *(*iter);

			rd()->setVertexLayout(vertexLayout);
			rd()->setIndexBuffer(submesh.IB.get());
			rd()->setVertexBuffer(0, submesh.mainVB.get());

			if (bumped)
			{
				rd()->setVertexBuffer(1, submesh.tangentVB.get());
			}
			
			rd()->setShader(shader);

			rd()->drawIndexed(submesh.primTopolpgy, 0, submesh.indicesCount);
		}
	}

	//----------------------------------------------------------------------------------------------
	void StaticModel::drawInstanced(
		VertexLayoutID vertexLayout, IShader* shader,
		uint numInstances, bool bumped
		)
	{
		assert(shader != NULL);
		assert(numInstances != 0);

		for (Mesh::SubMeshes::iterator iter = m_mesh->begin(); iter != m_mesh->end(); ++iter)
		{
			Mesh::SubMesh& submesh = *(*iter);

			rd()->setVertexLayout(vertexLayout);
			rd()->setIndexBuffer(submesh.IB.get());
			rd()->setVertexBuffer(0, submesh.mainVB.get());

			if (bumped)
			{
				rd()->setVertexBuffer(1, submesh.tangentVB.get());
			}

			rd()->setShader(shader);

			rd()->drawIndexedInstanced(submesh.primTopolpgy, 0, submesh.indicesCount, numInstances);
		}
	}

	//----------------------------------------------------------------------------------------------	
	const mat4f& StaticModel::transform() const
	{
		return m_worldMat;
	}

	//----------------------------------------------------------------------------------------------	
	void StaticModel::transform(const mat4f& m4)
	{
		m_worldMat = m4;
		m_worldAABB = m_mesh->bounds();
		m_worldAABB.transform(m_worldMat);
	}

	//----------------------------------------------------------------------------------------------	
	const AABB&	StaticModel::localBounds() const
	{
		return m_mesh->bounds();
	}

	//----------------------------------------------------------------------------------------------	
	const AABB&	StaticModel::worldBounds() const
	{
		return m_worldAABB;
	}

	//----------------------------------------------------------------------------------------------	
	bool StaticModel::intersects(
		float& dist, vec3f& pos, vec3f& normal,
		const vec3f& startPos, const vec3f& dir, float maxDist) const
	{
		if (!m_bsp.isValid())
			return false;

		//-- 1. convert startPos and dir into local space of model.
		mat4f invWorld = m_worldMat.getInverted();

		vec3f localStartPos = invWorld.applyToPoint(startPos);
		vec3f localDir		= invWorld.applyToVector(dir);

		vec3f localEndPos   = localStartPos + localDir.scale(maxDist);
		
		//-- 2. do test against AABB in local space of model.
		if (!m_mesh->bounds().intesectsSegment(localStartPos, localEndPos))
			return false;
		
		//-- 3. do test against BSP in local space of model.
		const BSPTri* bspTri;
		if (m_bsp->intersects(localStartPos, localEndPos, &dist, &pos, &bspTri))
		{
			vec3f v1 = bspTri->v[1] - bspTri->v[0];
			vec3f v2 = bspTri->v[2] - bspTri->v[0];
			normal = v1.cross(v2);
			normal.normalize();

			//-- 4. ... and if we have collision then convert it back into world space.
			pos    = m_worldMat.applyToPoint(pos);
			normal = m_worldMat.applyToVector(normal);
			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	/*static*/ uint Animation::m_forceFrameRate = 0;

	//----------------------------------------------------------------------------------------------
	/*static*/ int Animation::_setFrameRate(uint frameRate)
	{
		m_forceFrameRate = frameRate;
		return 1;
	}

	//----------------------------------------------------------------------------------------------
	Animation::Animation()
		:	m_animTime(0.0f), m_animComponents(0), m_jointCount(0), m_frameCount(0), m_frameRate(0)
	{
		//-- register console funcs.
		REGISTER_CONSOLE_FUNC("anim_frameRate", Animation::_setFrameRate);
	}

	//----------------------------------------------------------------------------------------------
	Animation::~Animation()
	{

	}

	//----------------------------------------------------------------------------------------------
	void Animation::setupFrame(Joint::Transform& transf, uint frame, uint joint)
	{
		//-- set base transformation.
		transf = m_baseFrame[joint];
		
		FrameData&	data  = m_frames[frame];
		int			flags = m_hierarchy[joint].flags;
		int			pos   = m_hierarchy[joint].startIdx;
		
		//-- update transformation specific for current frame.
		if (flags & 1 )	transf.pos.x	= data[pos++];
		if (flags & 2 )	transf.pos.y	= data[pos++];
		if (flags & 4 ) transf.pos.z	= data[pos++];
		if (flags & 8 ) transf.orient.x = data[pos++];
		if (flags & 16) transf.orient.y = data[pos++];
		if (flags & 32) transf.orient.z = data[pos++];

		renormalize(transf.orient);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateJoints(Joint::Transforms& skeleton, uint _1st, uint _2nd, float blend)
	{
		Joint::Transform first, second;

		for (uint i = 0; i < m_jointCount; ++i)
		{
			//-- find joint transformation at the fist and the second frames.
			setupFrame(first,  _1st, i);
			setupFrame(second, _2nd, i);
			
			//-- blend results using blend factor.
			skeleton[i].orient = slerp(first.orient, second.orient, blend);
			skeleton[i].pos    = lerp (first.pos, second.pos, blend);
		}
	}

	//----------------------------------------------------------------------------------------------
	void Animation::updateBounds(AABB& bound, uint _1st, uint _2nd, float blend)
	{
		//-- smoothly change one AABB to another.
		bound.min = lerp(m_bounds[_1st].min, m_bounds[_2nd].min, blend);
		bound.max = lerp(m_bounds[_1st].max, m_bounds[_2nd].max, blend);
	}
	
	//----------------------------------------------------------------------------------------------
	void Animation::tick(float animTime, const SkinnedMesh& mesh,
		Joint::Transforms& skeleton, AABB& bound, bool isLooped, bool isLocal)
	{
		float _totalAnimTime = m_animTime;
		float _frameRate	 = m_frameRate;

		if (m_forceFrameRate != 0)
		{
			_totalAnimTime  = (m_frameCount - 1) / (float)m_forceFrameRate;
			_frameRate = m_forceFrameRate;
		}
		
		//-- calculate blend frame.
		if (isLooped)
		{
			while (animTime > _totalAnimTime)
				animTime -= _totalAnimTime;
		}
		else
		{
			animTime = math::min(animTime, _totalAnimTime);
		}

		float _blendFrame = _frameRate * animTime;
		uint _1st   	  = floorf(_blendFrame);
		uint _2nd   	  = ceilf(_blendFrame);
		float blend		  = _blendFrame - _1st;

		//--ConPrint("Animation: time = %f; frame1 = %d; frame2 = %d; blend = %f",
		//--	animTime, _1st, _2nd, blend
		//--	);
		
		updateJoints(skeleton, _1st, _2nd, blend);		
		updateBounds(bound   , _1st, _2nd, blend);
		
		//-- reconstruct skeleton in world space.
		mesh.computeSkeleton(skeleton, isLocal);
	}

	//----------------------------------------------------------------------------------------------
	void Animation::goToFrame(uint frame, Joint::Transforms& skeleton, AABB& bound)
	{
		for (uint i = 0; i < m_jointCount; ++i)
		{
			setupFrame(skeleton[i], frame, i);
		}

		bound = m_bounds[frame];
	}

	//----------------------------------------------------------------------------------------------
	SkinnedModel::SkinnedModel(
		const Ptr<SkinnedMesh>& mesh, const Ptr<SkeletonCollider>* /*collider*/ /*= NULL*/
		) : m_mesh(mesh)
	{
		m_worldMat.setIdentity();
		m_localAABB = mesh->bounds();
		m_worldAABB = m_localAABB;

		//-- initialize nodes.
		m_skeleton.resize(m_mesh->jointsCount());
		m_mesh->setOriginSkeleton(m_skeleton);
	}

	//----------------------------------------------------------------------------------------------
	SkinnedModel::~SkinnedModel()
	{

	}

	//----------------------------------------------------------------------------------------------
	void SkinnedModel::tick(float dt)
	{
		if (m_animData.m_animation)
		{
			m_animData.m_animTime += dt;
			m_animData.m_animation->tick(
				m_animData.m_animTime, *m_mesh, m_skeleton, m_localAABB, m_animData.m_isLooped
				);

			m_worldAABB = m_localAABB;
			m_worldAABB.transform(m_worldMat);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	bool SkinnedModel::playAnim(const char* name, bool isLooped /* = false */)
	{
		Ptr<Animation> anim = ResourcesManager::instance().loadAnimation(
			(name + std::string(".md5anim")).c_str()
			);
		if (!anim)
		{
			ERROR_MSG("Can't load animation '%s'.", name);
			return false;
		}

		m_animData.m_isLooped  = isLooped;
		m_animData.m_animation = anim;
		m_animData.m_animTime  = 0.0f;

		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	bool SkinnedModel::stopAnim()
	{
		m_animData.m_isLooped  = false;
		m_animData.m_animation = NULL;
		m_animData.m_animTime  = 0.0f;

		m_mesh->setOriginSkeleton(m_skeleton);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedModel::drawSkeleton()
	{
		for (uint i = 0; i < m_skeleton.size(); ++i)
		{
			if (m_mesh->jointByIdx(i).m_parentIdx != -1)
			{
				const vec3f& startPos = m_worldMat.applyToPoint(m_skeleton[i].pos);
				const vec3f& endPos   = m_worldMat.applyToPoint(m_skeleton[m_mesh->jointByIdx(i).m_parentIdx].pos);

				DebugDrawer::instance().drawLine(startPos, endPos, Color(1,1,1,1));
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedModel::draw(VertexLayoutID vertexLayout, IShader* shader, bool bumped /* = false */)
	{
		assert(shader != NULL);

		//-- setup skeleton for all the sub-meshes.
		Joint::GPUTransform* ptr = m_mesh->skeletonTB()->map<Joint::GPUTransform>(IBuffer::ACCESS_WRITE_DISCARD);
		{
			for (uint i = 0; i < m_skeleton.size(); ++i)
			{
				ptr[i].pos    = vec4f(m_skeleton[i].pos, 0.0f);
				ptr[i].orient = m_skeleton[i].orient;
			}
		}
		m_mesh->skeletonTB()->unmap();

		shader->setTextureBuffer("tb_skeleton", m_mesh->skeletonTB());

		//for (SkinnedMesh::SubMeshes::iterator iter = m_mesh->begin(); iter != m_mesh->end(); ++iter)
		//{
		//	SkinnedMesh::SubMesh& submesh = *(*iter);

			SkinnedMesh::SubMesh& submesh = *(*m_mesh->begin());

			rd()->setVertexLayout(vertexLayout);
			rd()->setIndexBuffer(submesh.IB.get());
			rd()->setVertexBuffer(0, submesh.mainVB.get());

			if (bumped)
			{
				rd()->setVertexBuffer(1, submesh.tangentVB.get());
			}

			//-- add some auto constants.
			{
				shader->setTextureBuffer("tb_weights", submesh.weightsTB.get());
			}
			rd()->setShader(shader);

			rd()->drawIndexed(submesh.primTopolpgy, 0, submesh.indicesCount);
		//}
		
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedModel::drawInstanced(
		VertexLayoutID /*vertexLayout*/, IShader* /*shader*/, uint /*numInstances*/, bool /*bumped*/)
	{
		ERROR_MSG("To implement.");
	}

	//----------------------------------------------------------------------------------------------
	bool SkinnedModel::intersects(
		float& /*dist*/, vec3f& /*out*/, vec3f& /*normal*/,
		const vec3f& /*startPos*/, const vec3f& /*dir*/, float /*maxDist*/) const
	{
		ERROR_MSG("To implement.");
		return false;
	}

	//----------------------------------------------------------------------------------------------
	const mat4f& SkinnedModel::transform() const
	{
		return m_worldMat;
	}

	//----------------------------------------------------------------------------------------------
	void SkinnedModel::transform(const mat4f& m4)
	{
		m_worldMat = m4;
		m_worldAABB = m_localAABB;
		m_worldAABB.transform(m_worldMat);
	}

	//----------------------------------------------------------------------------------------------
	const AABB&	 SkinnedModel::localBounds() const
	{
		return m_localAABB;
	}

	//----------------------------------------------------------------------------------------------
	const AABB&	 SkinnedModel::worldBounds() const
	{
		return m_worldAABB;
	}

} // render
} // brUGE