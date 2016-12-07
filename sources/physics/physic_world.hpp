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

	//-- Manages the range of instances of one particular physical object type loaded from *.phys file.
	//----------------------------------------------------------------------------------------------
	class PhysicsObjectType
	{
	public:

		struct Instance;

		//-- rigid body.
		//------------------------------------------------------------------------------------------
		class RigidBody
		{
		public:
			struct Desc
			{
				Desc() : m_isKinematic(false), m_mass(0.0f), m_shapeIdx(0) { }

				struct Material
				{
					Material() : m_dynamicFriction(0.0f), m_staticFriction(0.0f), m_restitution(0.0f) { }

					float m_dynamicFriction;
					float m_staticFriction;
					float m_restitution;
				};

				std::string m_name;
				std::string m_node;
				Material	m_material;
				float		m_mass;
				vec3f		m_localInertia;
				vec3f		m_offset;
				uint		m_shapeIdx;
				bool		m_isKinematic;
			};

		public:
			RigidBody();
			virtual ~RigidBody();

			const char*				m_name; //-- pointed at the managed object. No needed explicit deletion.
			Node*					m_node;
			Instance*				m_owner;
			physx::PxRigidDynamic*	m_actor;
		};

		struct Instance
		{
			Handle									m_gameObj;
			Transform*								m_transform;
			std::vector<std::unique_ptr<RigidBody>>	m_bodies;
		};

	public:
		PhysicsObjectType(physx::PxPhysics* physics);
		~PhysicsObjectType();

		bool	load(const utils::ROData& desc);
		Handle	createInstance(Transform* transform, Handle gameObj);
		void	removeInstance(Handle instance);

	private:
		physx::PxPhysics*						m_physics;
		std::vector<RigidBody::Desc>			m_rigidBodyDescs;
		//std::vector<Constraints::Desc>			m_constrainDescs;
		std::vector<physx::PxShape*>			m_shapes;
		std::vector<std::unique_ptr<Instance>>	m_instances;
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
		void		removePhysicsObject(Handle physObj);

		//-- add terrain mesh to the physics world.
		bool		createTerrainPhysicsObject(uint gridSize, float unitsPerCell, float* heights, float heightScale, float minHeight, float maxHeight);
		bool		removeTerrainPhysicsObject();

		void		addImpulse(Handle physObj, const vec3f& dir, const vec3f& relPos);

		bool		collide(const vec3f& origin, const vec3f& dir) const;
		bool		collide(vec3f& out, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, Handle& gameObj, const vec3f& start, const vec3f& end) const;


		physx::PxPhysics* physics() { return m_physics; }
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
