#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "math/Vector3.hpp"

#include "PhysX/PxPhysicsAPI.h"

#include <vector>
#include <unordered_map>

namespace brUGE
{
	class  Node;
	struct Transform;

namespace physics
{

	//------------------------------------------------------------------------------------------------------------------
	class IPhysicsComponent : public IComponent
	{
	public:

		//--------------------------------------------------------------------------------------------------------------
		enum EType : int32
		{
			TYPE_UNKNOWN				= 0,
			TYPE_RIGID_BODY				= 1 << 1,
			TYPE_BOX_COLLIDER			= 1 << 2,
			TYPE_SPHERE_COLLIDER		= 1 << 3,
		};

	public:
		IPhysicsComponent(EType type) : IComponent(IComponent::FAMILY_TYPE_PHYSICS), m_type(type) { }
		virtual ~IPhysicsComponent() = 0 { }

	protected:
		EType m_type;
	};


	//------------------------------------------------------------------------------------------------------------------
	class RigidBodyComponent : public IPhysicsComponent
	{
	public:
		RigidBodyComponent(IPhysicsComponent::EType type) : IPhysicsComponent(IPhysicsComponent::TYPE_RIGID_BODY){ }
		virtual ~RigidBodyComponent() override { }

	private:
		Handle m_rigidBody;
	};

	//------------------------------------------------------------------------------------------------------------------
	class BoxColliderComponent : public IPhysicsComponent
	{
	public:
		BoxColliderComponent(IPhysicsComponent::EType type) : IPhysicsComponent(IPhysicsComponent::TYPE_BOX_COLLIDER) { }
		virtual ~BoxColliderComponent() override { }

	private:
		Handle m_boxCollider;
	};

	//------------------------------------------------------------------------------------------------------------------
	class SphereColliderComponent : public IPhysicsComponent
	{
	public:
		SphereColliderComponent(IPhysicsComponent::EType type) : IPhysicsComponent(IPhysicsComponent::TYPE_SPHERE_COLLIDER) { }
		virtual ~SphereColliderComponent() override { }

	private:
		Handle m_sphereCollider;
	};


	//----------------------------------------------------------------------------------------------
	class PhysicsSystem : public ISystem
	{
	public:

		//----------------------------------------------------------------------------------------------
		class World : public ISystem::IWorld
		{
		public:
			World();
			virtual ~World() override;

			virtual bool	init() override;

			virtual void	activate() override;
			virtual void	deactivate() override;

			virtual Handle	createComponent() override;
			virtual Handle	createComponent(const pugi::xml_node& data) override;
			virtual Handle	cloneComponent(Handle id) override;
			virtual bool	removeComponent(Handle id) override;

			virtual bool	registerGameObject(Handle gameObj) override;
			virtual bool	unregisterGameObject(Handle gameObj) override;

			//-- other methods
			void			makeKinematic(Handle physObj, bool flag);
			void			addImpulse(Handle physObj, const vec3f& impulse, const vec3f& worldPos);

		private:
			physx::PxScene*														m_scene;
			std::vector<std::unique_ptr<PhysicsObjectType::Instance>>			m_physObjs;
			std::unordered_map<std::string, std::unique_ptr<PhysicsObjectType>>	m_physObjTypes;
		};

		//----------------------------------------------------------------------------------------------
		class Context : public ISystem::IContext
		{
		public:
			struct CollisionCallback
			{
				CollisionCallback() : m_distance(0.0f), m_gameObj(CONST_INVALID_HANDLE), m_physObj(CONST_INVALID_HANDLE), m_node(nullptr) { }

				vec3f	m_wPos;
				vec3f	m_wNormal;
				float	m_distance;
				Handle	m_gameObj;
				Handle	m_physObj;
				Node*	m_node;
			};

		public:
			Context();
			virtual ~Context() override;

			virtual bool	init(ISystem* system, IWorld* world) override;

			bool			collide(CollisionCallback& cc, const vec3f& start, const vec3f& end) const;
		};

	public:
		PhysicsSystem();
		virtual ~PhysicsSystem() override;

		virtual bool init() override;
		virtual void update(IWorld* world) override;
		virtual void process(IContext* context) override;

	private:
		physx::PxFoundation*					m_foundation;
		physx::PxPhysics*						m_physics;
		physx::PxDefaultCpuDispatcher*			m_dispatcher;
		physx::PxDefaultAllocator				m_allocator;
		physx::PxDefaultErrorCallback			m_errorCallback;
		physx::PxVisualDebuggerConnection*		m_debuggerConnection;
	};


	//-- Manages the range of instances of one particular physical object type loaded from *.phys file.
	//----------------------------------------------------------------------------------------------
	class PhysicsObjectType
	{
	public:

		struct Instance;

		//------------------------------------------------------------------------------------------
		class RigidBody
		{
		public:

			struct Desc
			{
				Desc() : m_isKinematic(false), m_mass(0.0f), m_shapeIdx(0) { }

				struct Material
				{
					Material() : m_dynamicFriction(0.5f), m_staticFriction(0.5f), m_restitution(0.5f) { }

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
			~RigidBody();

			const char*				m_name; //-- pointed at the managed object. No needed explicit deletion.
			Node*					m_node;
			Instance*				m_owner;
			physx::PxRigidDynamic*	m_actor;
		};

		//------------------------------------------------------------------------------------------
		class Joint
		{
		public:
			struct Desc
			{
				std::string m_name;
				std::string m_type;
				std::string m_objA;
				std::string m_objB;
				vec3f		m_offsetA;
				vec3f		m_offsetB;
			};

		public:
			Joint();
			~Joint();

			const char*		m_name; //-- pointed at the managed object. No needed explicit deletion.
			physx::PxJoint*	m_pxJoint;
		};

		//------------------------------------------------------------------------------------------
		struct Instance
		{
			Instance() : m_gameObj(CONST_INVALID_HANDLE), m_physObj(CONST_INVALID_HANDLE), m_transform(nullptr) { }
			~Instance() { }

			void		enterScene(physx::PxScene* scene);
			void		leaveScene(physx::PxScene* scene);
			RigidBody*	findBody(const std::string& name);
			Joint*		findJoint(const std::string& name);

			Handle									m_physObj;
			Handle									m_gameObj;
			Transform*								m_transform;
			std::vector<std::unique_ptr<RigidBody>>	m_bodies;
			std::vector<std::unique_ptr<Joint>>		m_joints;
		};

	public:
		PhysicsObjectType();
		~PhysicsObjectType();

		bool						load(const utils::ROData& desc);
		std::unique_ptr<Instance>	createInstance(Transform* transform, Handle gameObj);

	private:
		std::vector<RigidBody::Desc>			m_rigidBodyDescs;
		std::vector<Joint::Desc>				m_jointDescs;
		std::vector<physx::PxShape*>			m_shapes;
		std::vector<physx::PxMaterial*>			m_materials;
	};


	//--
	//----------------------------------------------------------------------------------------------
	class PhysicsWorld : public NonCopyable
	{
	public:
		struct CollisionCallback
		{
			CollisionCallback() : m_distance(0.0f), m_gameObj(CONST_INVALID_HANDLE), m_physObj(CONST_INVALID_HANDLE), m_node(nullptr) { }

			vec3f	m_wPos;
			vec3f	m_wNormal;
			float	m_distance;
			Handle	m_gameObj;
			Handle	m_physObj;
			Node*	m_node;
		};

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

		//-- ToDo: for testing only
		void		makeKinematic(Handle physObj, bool flag);

		void		addImpulse(Handle physObj, const vec3f& impulse, const vec3f& worldPos);

		bool		collide(CollisionCallback& cc, const vec3f& start, const vec3f& end) const;

	private:
		void		updateGraphicsTransforms();
		void		updatePhysicsTransforms();
		void		debugDraw();

	private:
		physx::PxFoundation*					m_foundation;
		physx::PxPhysics*						m_physics;
		physx::PxScene*							m_scene;
		physx::PxDefaultCpuDispatcher*			m_dispatcher;
		physx::PxDefaultAllocator				m_allocator;
		physx::PxDefaultErrorCallback			m_errorCallback;
		physx::PxVisualDebuggerConnection*		m_debuggerConnection;

		std::vector<std::unique_ptr<PhysicsObjectType::Instance>>			m_physObjs;
		std::unordered_map<std::string, std::unique_ptr<PhysicsObjectType>>	m_physObjTypes;
	};

} //-- physic
} //-- brUGE
