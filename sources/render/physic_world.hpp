#pragma once

#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "math/Vector3.hpp"

//-- ToDo: reconsider.
#pragma warning(push, 3)
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#pragma warning(pop)

#include <vector>
#include <map>
#include <memory>

namespace brUGE
{
	class  Node;
	struct Transform;

namespace physic
{
	struct PhysObj;

	//-- Manages the range of instances of one particular physical object type loaded
	//-- from *.phys file.
	//----------------------------------------------------------------------------------------------
	class PhysObjDesc
	{
	public:

		//-- rigid body.
		//------------------------------------------------------------------------------------------
		class RigidBody : public btMotionState
		{
		public:
			struct Desc
			{
				std::string m_name;
				std::string m_node;
				bool		m_isKinematic;
				float		m_mass;
				vec3f		m_localInertia;
				vec3f		m_offset;
				uint		m_shape;
			};

		public:
			RigidBody();
			virtual ~RigidBody();

			virtual void getWorldTransform(btTransform& worldTrans) const;
			virtual void setWorldTransform(const btTransform& worldTrans);

			const char*	 m_name; //-- pointed at the managed object. No needed explicit deletion.
			Node*		 m_node;
			vec3f*	 	 m_offset;
			PhysObj*	 m_owner;
			btRigidBody* m_body;
		};
		typedef std::vector<RigidBody*>	RigidBodies;

		//-- constraint.
		//------------------------------------------------------------------------------------------
		class Constraint
		{
		public:
			struct Desc
			{
				std::string m_name;
			};

		public:
			Constraint()  {}
			~Constraint() { delete m_constraint; m_constraint = nullptr; }

			const char*		   m_name; //-- pointed at the managed object. No needed explicit deletion.
			btTypedConstraint* m_constraint;
		};
		typedef std::vector<Constraint*> Constraints;

	public:
		PhysObjDesc();
		~PhysObjDesc();

		bool load	(const utils::ROData& desc, btDynamicsWorld* dynamicsWorld, Transform* transform);
		bool create	(PhysObj*& obj, Transform* transform, Handle owner);
		bool destroy(PhysObj* obj);

	private:
		typedef btAlignedObjectArray<btCollisionShape*> Shapes;
		typedef std::vector<RigidBody::Desc>			RigidBodyDescs;
		typedef std::vector<Constraint::Desc>			ConstraintDescs;

		RigidBodyDescs			m_rigidBodyDescs;
		ConstraintDescs			m_constraintDescs;
		Shapes					m_shapes;
		std::vector<PhysObj*>	m_physObjs;
		btDynamicsWorld*		m_dynamicsWorld;  //-- pointed at the managed object. No needed explicit deletion.
	};


	//-- Represent the minimum logical unit of the physic system.
	//----------------------------------------------------------------------------------------------
	struct PhysObj
	{
		PhysObj();
		~PhysObj();

		void addToWorld  (btDynamicsWorld* world);
		void delFromWorld(btDynamicsWorld* world);

		void addImpulse(const vec3f& dir, const vec3f& relPos);

		Handle					 m_objectID;
		Transform*				 m_transform;
		PhysObjDesc::RigidBodies m_bodies;
		PhysObjDesc::Constraints m_constraints;
	};


	//-- Debug drawer.
	//----------------------------------------------------------------------------------------------
	class PhysDebugDrawer : public btIDebugDraw
	{
	public:
		PhysDebugDrawer() : m_modes(DBG_NoDebug) { }

		virtual void drawLine			(const btVector3& from, const btVector3& to, const btVector3& color);
		virtual void drawContactPoint	(const btVector3& /*PointOnB*/, const btVector3& /*normalOnB*/, btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/) { }
		virtual void reportErrorWarning	(const char* warningString);
		virtual void draw3dText			(const btVector3& /*location*/, const char* /*textString*/) { }
		virtual void setDebugMode		(int debugMode) { m_modes |= debugMode; }
		virtual int	 getDebugMode		() const { return m_modes;}

	private:
		int m_modes;
	};


	//-- Manages of all physical instances in the game, and updates dynamics simulation and
	//-- collision detection.
	//----------------------------------------------------------------------------------------------
	class PhysicWorld : public NonCopyable
	{
	public:
		PhysicWorld();
		~PhysicWorld();

		bool		init();

		//-- detect collisions. Called once per frame to update the whole game world.
		void		detectCollisions(float dt);
		//-- simulate rigid body dynamics.
		void		simulateDynamics(float dt);

		//-- add new object to the collision world.
		PhysObj*	addPhysicDef(const char* desc, Transform* transform, Handle owner);
		bool		delPhysicDef(PhysObj* physObj);

		//-- add terrain mesh to the physics world.
		bool		addTerrain(uint gridSize, float unitsPerCell, float* heights, float heightScale, float minHeight, float maxHeight);
		bool		delTerrain();

		bool		collide(const vec3f& origin, const vec3f& dir) const;
		bool		collide(vec3f& out, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, const vec3f& start, const vec3f& end) const;
		bool		collide(mat4f& localMat, Node*& node, Handle& objID, const vec3f& start, const vec3f& end) const;

	private:
		//-- console functions.
		int _drawWire(bool flag);
		int _drawAABB(bool flag);

	private:
		std::unique_ptr<btDynamicsWorld>					m_dynamicsWorld;
		std::unique_ptr<btBroadphaseInterface>				m_broadphase;
		std::unique_ptr<btCollisionDispatcher>				m_dispatcher;
		std::unique_ptr<btConstraintSolver>					m_solver;
		std::unique_ptr<btDefaultCollisionConfiguration>	m_collisionCfg;
		PhysDebugDrawer										m_debugDrawer;

		std::map<std::string, PhysObjDesc*>					m_physObjDescs;

		//-- terrain mesh.
		struct TerrainPhysics
		{
			TerrainPhysics() : m_shape(nullptr), m_rigidBody(nullptr) { }

			btHeightfieldTerrainShape*	m_shape;
			btRigidBody*				m_rigidBody;
		};
		TerrainPhysics										m_terrain;
	};

} //-- physic
} //-- brUGE
