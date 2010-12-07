#pragma once

#include "prerequisites.h"
#include "utils/Data.hpp"
#include "math/Vector3.h"

//-- ToDo: reconsider.
#pragma warning(push, 3)
#include "bullet/btBulletDynamicsCommon.h"
#pragma warning(pop)

#include <vector>
#include <map>

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
				float		m_mass;
				vec3f		m_localInertia;
				uint		m_shape;
			};

		public:
			RigidBody();
			virtual ~RigidBody();

			virtual void getWorldTransform(btTransform& worldTrans) const;
			virtual void setWorldTransform(const btTransform& worldTrans);

			const char*  m_name;
			Node*		 m_node;
			Handle		 m_owner;
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
			~Constraint() {}

			const char*		   m_name;
			btTypedConstraint* m_constraint;
		};
		typedef std::vector<Constraint*> Constraints;

	public:
		PhysObjDesc();
		~PhysObjDesc();

		bool load	(const utils::ROData& desc, btDynamicsWorld* dynamicsWorld);
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
		btDynamicsWorld*		m_dynamicsWorld;
	};


	//-- Represent the minimum logical unit of the physic system.
	//----------------------------------------------------------------------------------------------
	struct PhysObj
	{
		PhysObj();
		~PhysObj();

		void addToWorld  (btDynamicsWorld* world);
		void delFromWorld(btDynamicsWorld* world);

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
	class PhysicWorld
	{
	public:
		PhysicWorld();
		~PhysicWorld();

		bool		init();
		bool		fini();

		//-- detect collisions. Called once per frame to update the whole game world.
		void		detectCollisions(float dt);
		//-- simulate rigid body dynamics.
		void		simulateDynamics(float dt);

		//-- add new object to the collision world.
		PhysObj*	addPhysicDef(const char* desc, Transform* transform, Handle owner);
		bool		delPhysicDef(PhysObj* physObj);

		bool		collide(const vec3f& origin, const vec3f& dir) const;
		bool		collide(float& dist, vec3f& out, const vec3f& origin, const vec3f& dir) const;
		bool		collide(mat4f& localMat, Node& node, const vec3f& origin, const vec3f& dir) const;

	private:
		//-- console functions.
		int _drawWire(bool flag);
		int _drawAABB(bool flag);

	private:
		btDynamicsWorld*					m_dynamicsWorld;
		btBroadphaseInterface*				m_broadphase;
		btCollisionDispatcher*				m_dispatcher;
		btConstraintSolver*					m_solver;
		btDefaultCollisionConfiguration*	m_collisionCfg;
		PhysDebugDrawer						m_debugDrawer;

		std::map<std::string, PhysObjDesc*>	m_physObjDescs;
	};

} //-- physic
} //-- brUGE
