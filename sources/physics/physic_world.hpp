#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "math/Vector3.hpp"

#include "PhysX/PxPhysicsAPI.h"

#include <vector>
#include <map>

namespace brUGE
{
	class  Node;
	struct Transform;

namespace physics
{
	struct PhysObj;

	//-- Manages the range of instances of one particular physical object type loaded from *.phys file.
	//----------------------------------------------------------------------------------------------
	class PhysObjDesc
	{
	public:

		//-- rigid body.
		//------------------------------------------------------------------------------------------
		class RigidBody
		{
		public:
			struct Desc
			{
				Desc() : m_isKinematic(false), m_mass(0.0f), m_shapeIdx(0) { }

				std::string m_name;
				std::string m_node;
				bool		m_isKinematic;
				float		m_mass;
				vec3f		m_localInertia;
				vec3f		m_offset;
				uint		m_shapeIdx;
			};

		public:
			RigidBody();
			virtual ~RigidBody();

			const char*				m_name; //-- pointed at the managed object. No needed explicit deletion.
			Node*					m_node;
			PhysObj*				m_owner;
			physx::PxRigidDynamic*	m_actor;
		};
		typedef std::vector<RigidBody*>	RigidBodies;

	public:
		PhysObjDesc();
		~PhysObjDesc();

		bool load			(const utils::ROData& desc, physx::PxScene* scene, Transform* transform);
		bool createInstance	(PhysObj*& obj, Transform* transform, Handle owner);
		bool removeInstance (PhysObj* obj);

	private:
		std::vector<physx::PxShape*>	m_shapes;
		std::vector<PhysObj*>			m_physObjs;
	};


	//-- Represent the minimum logical unit of the physic system.
	//-- ToDo: substitute with Handle
	//----------------------------------------------------------------------------------------------
	struct PhysObj
	{
		PhysObj();
		~PhysObj();

		Handle					 m_objectID;
		Transform*				 m_transform;
		PhysObjDesc::RigidBodies m_bodies;
	};


	//--
	//----------------------------------------------------------------------------------------------
	class PhysicsWorld : public NonCopyable
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		bool		init();

		//-- simulate rigid body dynamics.
		void		simulate(float dt);

		//-- add new object to the collision world.
		Handle		createPhysicsObject(const char* desc, Transform* transform, Handle gameObj);
		void		removePhysicsObject(Handle handle);

		//-- add terrain mesh to the physics world.
		bool		createTerrainPhysicsObject(uint gridSize, float unitsPerCell, float* heights, float heightScale, float minHeight, float maxHeight);
		bool		removeTerrainPhysicsObject();

		void		addImpulse(Handle physObj, const vec3f& dir, const vec3f& relPos);

		bool		collide(const vec3f& origin, const vec3f& dir) const;
		bool		collide(vec3f& out, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, Handle& objID, const vec3f& start, const vec3f& end) const;
	private:
		//-- console functions.
		int _drawWire(bool flag);
		int _drawAABB(bool flag);

	private:
		physx::PxFoundation*					m_foundation;
		physx::PxPhysics*						m_physics;
		physx::PxScene*							m_scene;
		physx::PxDefaultCpuDispatcher*			m_dispatcher;
		physx::PxDefaultAllocator				m_allocator;
		physx::PxDefaultErrorCallback			m_errorCallback;
		physx::PxVisualDebuggerConnection*		m_debuggerConnection;

		std::map<std::string, PhysObjDesc*>		m_physObjDescs;
	};

} //-- physic
} //-- brUGE
