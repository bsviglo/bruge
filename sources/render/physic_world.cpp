#include "physic_world.hpp"
#include "game_world.hpp"
#include "math/Matrix4x4.h"
#include "pugixml/pugixml.hpp"
#include "utils/Data.hpp"
#include "utils/ArgParser.h"
#include "os/FileSystem.h"
#include "render/DebugDrawer.h"
#include "console/Console.h"

using namespace brUGE;
using namespace brUGE::os;
using namespace brUGE::utils;
using namespace brUGE::math;
using namespace brUGE::render;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	inline mat4f bullet2bruge(const btTransform& transform)
	{
		mat4f ret;

		const btVector3& orgn = transform.getOrigin();
		const btVector3& row1 = transform.getBasis()[0];
		const btVector3& row2 = transform.getBasis()[1];
		const btVector3& row3 = transform.getBasis()[2];

		//-- set identity.
		ret.setIdentity();
		
		//-- set orientation.
		ret.m00 = row1.x(); ret.m10 = row1.y(); ret.m20 = row1.z();
		ret.m01 = row2.x(); ret.m11 = row2.y(); ret.m21 = row2.z();
		ret.m02 = row3.x(); ret.m12 = row3.y(); ret.m22 = row3.z();
		
		//-- set translation
		ret.m30 = orgn.x(); ret.m31 = orgn.y(); ret.m32 = orgn.z();

		mat4f mat;
		mat.setLookAt(vec3f(0,0,0), vec3f(0,0,-1), vec3f(0,1,0));

		ret.postMultiply(mat);

		return ret;
	}

	//----------------------------------------------------------------------------------------------
	inline btTransform bruge2bullet(const mat4f& t)
	{
		btTransform ret;

		btMatrix3x3 basis(
			t.m00, t.m10, t.m20,
			t.m01, t.m11, t.m21,
			t.m02, t.m12, t.m22
			);

		btVector3 origin(
			t.m30, t.m31, -t.m32
			);

		ret.setOrigin(origin);
		ret.setBasis(basis);

		return ret;
	}
}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.


namespace brUGE
{
namespace physic
{

	//----------------------------------------------------------------------------------------------
	PhysicWorld::PhysicWorld()
	{
		//-- register console funcs.
		REGISTER_CONSOLE_METHOD("phys_drawWire", _drawWire, PhysicWorld);
		REGISTER_CONSOLE_METHOD("phys_drawAABB", _drawAABB, PhysicWorld);
	}

	//----------------------------------------------------------------------------------------------
	PhysicWorld::~PhysicWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::init()
	{
		//-- 1. create configuration contains default setup for memory, collision setup.
		m_collisionCfg = new btDefaultCollisionConfiguration();

		//-- 2. use the default collision dispatcher.
		m_dispatcher = new btCollisionDispatcher(m_collisionCfg);

		//-- 3. create broad phase. 
		m_broadphase = new btDbvtBroadphase();

		//-- 4. use the default constraint solver.
		m_solver = new btSequentialImpulseConstraintSolver;

		//-- 5. create dynamic world.
		m_dynamicsWorld = new btDiscreteDynamicsWorld(
			m_dispatcher, m_broadphase, m_solver, m_collisionCfg
			);

		//-- 6. set default gravity.
		m_dynamicsWorld->setGravity(btVector3(0.0f, -10.0f, 0.0f));

		//-- 7. set debug drawer.
		m_dynamicsWorld->setDebugDrawer(&m_debugDrawer);

		//-- test.
		{
			btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),50);
			btVector3 localInertia(0,0,0);

			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-80,0));

			//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0,myMotionState,groundShape,localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);

			//add the body to the dynamics world
			m_dynamicsWorld->addRigidBody(body);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::fini()
	{
		//-- ToDo:
		for (auto i = m_physObjDescs.begin(); i != m_physObjDescs.end(); ++i)
			delete i->second;

		delete m_dynamicsWorld;
		delete m_solver;
		delete m_broadphase;
		delete m_dispatcher;
		delete m_collisionCfg;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	PhysObj* PhysicWorld::addPhysicDef(const char* desc, Transform* transform, Handle owner)
	{
		auto result = m_physObjDescs.find(desc);
		if (result != m_physObjDescs.end())
		{
			PhysObj* obj = nullptr;
			result->second->create(obj, transform, owner);
			return obj;
		}
		else
		{
			RODataPtr data = FileSystem::instance().readFile("resources/" + std::string(desc));	

			std::unique_ptr<PhysObjDesc> physDesc(new PhysObjDesc());
			if (!data.isValid() || !physDesc->load(*data.get(), m_dynamicsWorld))
			{
				return nullptr;
			}

			//-- add new phys descriptor.
			m_physObjDescs[desc] = physDesc.release();

			PhysObj* obj = nullptr;
			m_physObjDescs[desc]->create(obj, transform, owner);
			return obj;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::delPhysicDef(PhysObj* physObj)		
	{
		for (auto i = m_physObjDescs.begin(); i != m_physObjDescs.end(); ++i)
		{
			if (i->second->destroy(physObj))
				return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicWorld::detectCollisions(float /*dt*/)
	{

	}

	//----------------------------------------------------------------------------------------------
	void PhysicWorld::simulateDynamics(float dt)
	{
		m_dynamicsWorld->stepSimulation(dt);
		m_dynamicsWorld->debugDrawWorld();
	}

	//----------------------------------------------------------------------------------------------
	PhysObjDesc::PhysObjDesc()
		: m_dynamicsWorld(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysObjDesc::~PhysObjDesc()
	{
		assert(m_physObjs.size() == 0);

		for (auto i = 0; i < m_shapes.size(); ++i)
		{
			delete m_shapes[i];
		}
		m_shapes.clear();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysObjDesc::load(const ROData& data, btDynamicsWorld* world)
	{
		m_dynamicsWorld = world;

		//-- 1. try to create DOM model from the input buffer.
		pugi::xml_document doc;
		auto result = doc.load_buffer(data.ptr(), data.length());

		if (!result)
			return false;

		//-- 2. check root element.
		auto root = doc.document_element();

		//-- 3. try to read <rigidBodies> section.
		{
			auto elems = root.child("rigidBodies");

			for (auto elem = elems.child("rigidBody"); elem; elem = elem.next_sibling("rigidBody"))
			{
				RigidBody::Desc bodyDesc;

				bodyDesc.m_name = elem.attribute("name").value();
				bodyDesc.m_node = elem.attribute("node").value();
				bodyDesc.m_mass = elem.attribute("mass").as_float();

				//-- create new shape.
				{
					auto shape = elem.child("shape");
					if (shape.empty())
						return false;

					auto type = shape.attribute("type").value();

					auto params = shape.child("params");
					if (params.empty())
						return false;

					btCollisionShape* btShape = nullptr;

					//-- create desired collision shape.
					if (strcmp(type, "box") == 0)
					{
						auto size = parseTo<vec3f>(params.attribute("size").value());

						btShape = new btBoxShape(
							btVector3(btScalar(size.x), btScalar(size.y), btScalar(size.z))
							);
					}
					else if (strcmp(type, "cylinder") == 0)
					{
						auto size = parseTo<vec3f>(params.attribute("size").value());

						btShape = new btCylinderShape(
							btVector3(btScalar(size.x), btScalar(size.y), btScalar(size.z))
							);
					}
					else
					{
						assert("type is not yet implemented.");
					}

					//-- calculate local inertia.
					btVector3 lInertia(0,0,0);
					if (bodyDesc.m_mass != 0.0f)
						btShape->calculateLocalInertia(bodyDesc.m_mass, lInertia);

					bodyDesc.m_localInertia = vec3f(lInertia.x(), lInertia.y(), lInertia.z());

					//-- add new shape to cache.
					m_shapes.push_back(btShape);

					bodyDesc.m_shape = m_shapes.size() - 1;

					//-- add new rigid body descriptor.
					m_rigidBodyDescs.push_back(bodyDesc);
				}
			}
		}

		//-- 4. try to read <constraints> section.
		/*
		{
			auto elems = root.child("constraints");

			for (auto elem = elems.child("constraint"); elem; elem = elem.next_sibling("constraint"))
			{
				auto type = elems.attribute("type").value();
				auto name = elems.attribute("name").value();
				auto objA = elems.attribute("objA").value();
				auto objB = elems.attribute("objB").value();

				//-- ToDo:
			}
		}
		*/

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysObjDesc::create(PhysObj*& obj, Transform* transform, Handle owner)
	{
		std::unique_ptr<PhysObj> physObj(new PhysObj);

		//-- 1. set transform.
		physObj->m_transform = transform;
			
		//-- 2. create all rigid bodies.
		for (auto i = m_rigidBodyDescs.begin(); i != m_rigidBodyDescs.end(); ++i)
		{
			RigidBody* body = new RigidBody();

			body->m_name  = i->m_name.c_str();
			body->m_owner = owner;
			body->m_node  = transform->m_nodes[0]; //-- ToDo: load needed node.

			btRigidBody::btRigidBodyConstructionInfo rbInfo(
				i->m_mass, body, m_shapes[i->m_shape],
				btVector3(i->m_localInertia.x, i->m_localInertia.y, i->m_localInertia.z)
				);

			body->m_body = new btRigidBody(rbInfo);

			physObj->m_bodies.push_back(body);
		}

		//-- 3. create all constraints.
		for (auto i = m_constraintDescs.begin(); i != m_constraintDescs.end(); ++i)
		{

		}

		//-- 4. add to physic world.
		physObj->addToWorld(m_dynamicsWorld);
	
		//-- 5. return result.
		obj = physObj.release();
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysObjDesc::destroy(PhysObj* obj)
	{
		for (uint i = 0; i < m_physObjs.size(); ++i)
		{
			if (m_physObjs[i] == obj)
			{
				m_physObjs[i]->delFromWorld(m_dynamicsWorld);

				delete m_physObjs[i];
				m_physObjs[i] = m_physObjs.back();
				m_physObjs.pop_back();
				return true;
			}
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------------
	PhysObj::PhysObj() : m_transform(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysObj::~PhysObj()
	{
		m_transform = nullptr;

		for (auto i = m_bodies.begin(); i != m_bodies.end(); ++i)
			delete *i;

		for (auto i = m_constraints.begin(); i != m_constraints.end(); ++i)
			delete *i;

		m_bodies.clear();
		m_constraints.clear();
	}

	//----------------------------------------------------------------------------------------------
	void PhysObj::addToWorld(btDynamicsWorld* world)
	{
		for (auto i = m_bodies.begin(); i != m_bodies.end(); ++i)
		{
			world->addRigidBody((*i)->m_body);
		}

		for (auto i = m_constraints.begin(); i != m_constraints.end(); ++i)
			world->addConstraint((*i)->m_constraint, true);
	}

	//----------------------------------------------------------------------------------------------
	void PhysObj::delFromWorld(btDynamicsWorld* world)
	{
		for (auto i = m_bodies.begin(); i != m_bodies.end(); ++i)
			world->removeRigidBody((*i)->m_body);

		for (auto i = m_constraints.begin(); i != m_constraints.end(); ++i)
			world->removeConstraint((*i)->m_constraint);
	}


	//----------------------------------------------------------------------------------------------
	PhysObjDesc::RigidBody::RigidBody()
		: m_name(nullptr), m_node(nullptr), m_owner(CONST_INVALID_HANDLE), m_body(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysObjDesc::RigidBody::~RigidBody()
	{
		delete m_body;
		m_body = nullptr;
	}

	//----------------------------------------------------------------------------------------------
	void PhysObjDesc::RigidBody::getWorldTransform(btTransform& worldTrans) const
	{
		worldTrans = bruge2bullet(m_node->matrix());
	}

	//----------------------------------------------------------------------------------------------
	void PhysObjDesc::RigidBody::setWorldTransform(const btTransform& worldTrans)
	{
		m_node->matrix(bullet2bruge(worldTrans));
	}

	//----------------------------------------------------------------------------------------------
	void PhysDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		mat4f mat;
		mat.setLookAt(vec3f(0,0,0), vec3f(0,0,-1), vec3f(0,1,0));

		DebugDrawer::instance().drawLine(
			mat.applyToPoint(vec3f(from.x(), from.y(), from.z())),
			mat.applyToPoint(vec3f(to.x(), to.y(), to.z())),
			Color(color.x(), color.y(), color.z())
			);
	}

	//----------------------------------------------------------------------------------------------
	void PhysDebugDrawer::reportErrorWarning(const char* warningString)
	{
		ConWarning(warningString);
		WARNING_MSG(warningString);
	}

	//----------------------------------------------------------------------------------------------
	int PhysicWorld::_drawWire(bool flag)
	{
		m_debugDrawer.setDebugMode(
			flag ? btIDebugDraw::DBG_DrawWireframe : !btIDebugDraw::DBG_DrawWireframe
			);
		return 0;
	}

	//----------------------------------------------------------------------------------------------
	int PhysicWorld::_drawAABB(bool flag)
	{
		m_debugDrawer.setDebugMode(
			flag ? btIDebugDraw::DBG_DrawAabb : !btIDebugDraw::DBG_DrawAabb
			);
		return 0;
	}

} //-- physic
} //-- brUGE