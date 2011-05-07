#include "physic_world.hpp"
#include "game_world.hpp"
#include "math/Matrix4x4.hpp"
#include "pugixml/pugixml.hpp"
#include "utils/Data.hpp"
#include "utils/ArgParser.h"
#include "os/FileSystem.h"
#include "render/DebugDrawer.h"
#include "render/Color.h"
#include "render/Mesh.hpp"
#include "console/Console.h"
#include <algorithm>

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

		//-- from the right-handed coordinate system to left-handed with additional respect to
		//-- conversion from column-major matrix layout to row-major.
		ret.m00 = -row1.x(); ret.m01 = +row2.x(); ret.m02 = -row3.x(); ret.m03 = 0.0f;
		ret.m10 = -row1.y(); ret.m11 = +row2.y(); ret.m12 = -row3.y(); ret.m13 = 0.0f;
		ret.m20 = -row1.z(); ret.m21 = +row2.z(); ret.m22 = -row3.z(); ret.m23 = 0.0f;
		ret.m30 = -orgn.x(); ret.m31 = +orgn.y(); ret.m32 = -orgn.z(); ret.m33 = 1.0f;

		return ret;
	}

	//----------------------------------------------------------------------------------------------
	inline btTransform bruge2bullet(const mat4f& t)
	{
		btTransform ret;

		btMatrix3x3 basis(
			-t.m00, -t.m10, -t.m20,
			+t.m01, +t.m11, +t.m21,
			-t.m02, -t.m12, -t.m22
			);

		btVector3 origin(
			-t.m30, t.m31, -t.m32
			);

		ret.setOrigin(origin);
		ret.setBasis(basis);

		return ret;
	}

	//----------------------------------------------------------------------------------------------
	inline btVector3 bruge2bullet(const vec3f& v3)
	{
		return btVector3(-v3.x, v3.y, -v3.z);
	}

	//----------------------------------------------------------------------------------------------
	inline vec3f bullet2bruge(const btVector3& v3)
	{
		return vec3f(-v3.x(), v3.y(), -v3.z());
	}

	//----------------------------------------------------------------------------------------------
	inline vec3f bullet2vec3f(const btVector3& v3)
	{
		return vec3f(v3.x(), v3.y(), v3.z());
	}

	//----------------------------------------------------------------------------------------------
	Node* findNode(const std::string& name, Nodes& nodes)
	{
		auto iter = std::find_if(nodes.begin(), nodes.end(), [name](Node* node) {
			return (name == node->name());
		});

		assert(iter != nodes.end());

		return *iter;
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
		//-- ToDo:
		for (auto i = m_physObjDescs.begin(); i != m_physObjDescs.end(); ++i)
			delete i->second;
		
		//-- ToDo: to guaranty the right calling sequence of destructors.
		m_dynamicsWorld.reset();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::init()
	{
		//-- 1. create configuration contains default setup for memory, collision setup.
		m_collisionCfg.reset(new btDefaultCollisionConfiguration());

		//-- 2. use the default collision dispatcher.
		m_dispatcher.reset(new btCollisionDispatcher(m_collisionCfg.get()));

		//-- 3. create broad phase. 
		m_broadphase.reset(new btDbvtBroadphase());

		//-- 4. use the default constraint solver.
		m_solver.reset(new btSequentialImpulseConstraintSolver);

		//-- 5. create dynamic world.
		m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(
			m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionCfg.get()
			));

		//-- 6. set default gravity.
		m_dynamicsWorld->setGravity(btVector3(0.0f, -10.0f, 0.0f));

		//-- 7. set debug drawer.
		m_dynamicsWorld->setDebugDrawer(&m_debugDrawer);

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
			if (!data.isValid() || !physDesc->load(*data.get(), m_dynamicsWorld.get(), transform))
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
	bool PhysicWorld::addTerrain(
		uint gridSize, float unitsPerCell, float* heights,
		float heightScale, float minHeight, float maxHeight)
	{
		//-- create heightfield shape.
		m_terrain.m_shape = new btHeightfieldTerrainShape(
			gridSize, gridSize, heights, heightScale, minHeight, maxHeight,	1, PHY_FLOAT, false
			);

		assert(m_terrain.m_shape && "null terrain heightfield shape.");

		//-- scale the shape.
		btVector3 localScaling(unitsPerCell, 1, unitsPerCell);
		m_terrain.m_shape->setLocalScaling(localScaling);

		//-- set origin to middle of heightfield.
		btTransform tr;
		tr.setIdentity();

		float middleY = (maxHeight - minHeight) * 0.5f;
		float offsetY = maxHeight - middleY;

		btMatrix3x3 mat3x3;
		mat3x3.setEulerZYX(0, degToRad(180), 0);
		tr.setBasis(mat3x3);
		tr.setOrigin(btVector3(0, offsetY, 0));

		m_terrain.m_rigidBody = new btRigidBody(
			0, 0, m_terrain.m_shape, btVector3(0,0,0)
			);	

		m_terrain.m_rigidBody->setWorldTransform(tr);
		m_terrain.m_rigidBody->setContactProcessingThreshold(0.0f);
		m_dynamicsWorld->addRigidBody(m_terrain.m_rigidBody);

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::delTerrain()
	{
		//-- remove terrain from physics world.
		m_dynamicsWorld->removeRigidBody(m_terrain.m_rigidBody);

		//-- delete.
		delete m_terrain.m_rigidBody;
		delete m_terrain.m_shape;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::collide(const vec3f& origin, const vec3f& dir) const
	{
		btVector3 start = bruge2bullet(origin);
		btVector3 end   = bruge2bullet(origin + dir.scale(1000.0f));

		btCollisionWorld::ClosestRayResultCallback cb(start, end);
		m_dynamicsWorld->rayTest(start, end, cb);

		return cb.hasHit();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::collide(vec3f& out, const vec3f& origin, const vec3f& dir) const
	{
		btVector3 start = bruge2bullet(origin);
		btVector3 end   = bruge2bullet(origin + dir.scale(1000.0f));

		btCollisionWorld::ClosestRayResultCallback cb(start, end);
		m_dynamicsWorld->rayTest(start, end, cb);

		if (cb.hasHit())
		{
			out = bullet2bruge(cb.m_hitPointWorld);
			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicWorld::collide(mat4f& localMat, Node*& node, const vec3f& origin, const vec3f& dir) const
	{
		btVector3 start = bruge2bullet(origin);
		btVector3 end   = bruge2bullet(origin + dir.scale(1000.0f));

		btCollisionWorld::ClosestRayResultCallback cb(start, end);
		m_dynamicsWorld->rayTest(start, end, cb);

		if (cb.hasHit())
		{
			//-- ToDo: terrain is not suit for this current collision system
			//--	   We need some reworking here. Reconsider.
			if (!cb.m_collisionObject->getUserPointer())
				return false;

			auto body  = static_cast<PhysObjDesc::RigidBody*>(cb.m_collisionObject->getUserPointer());
			auto world = cb.m_collisionObject->getWorldTransform();

			//vec3f localPoint = bullet2bruge(world.invXform(cb.m_hitPointWorld));
			vec3f localPoint  = bullet2vec3f(world.invXform(cb.m_hitPointWorld));
			vec3f localNormal = bullet2bruge(world.getBasis().inverse() * cb.m_hitNormalWorld);

			mat4f mat;
			vec3f up(0,1,0);

			if (almostZero(fabs(localNormal.dot(up)) - 1.0f))
			{
				up = vec3f(0,0,1);
			}

			//-- apply offset to local point.
			localPoint += *body->m_offset;

			localMat.setLookAt(localPoint, localNormal, up);
			localMat.invert();

			node = body->m_node;

			return true;
		}

		return false;
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
	bool PhysObjDesc::load(const ROData& data, btDynamicsWorld* world, Transform* transform)
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

				bodyDesc.m_name			= elem.attribute("name").value();
				bodyDesc.m_node			= elem.attribute("node").value();
				bodyDesc.m_mass			= elem.attribute("mass").as_float();
				bodyDesc.m_offset		= parseTo<vec3f>(elem.attribute("offset").value());
				bodyDesc.m_isKinematic	= elem.attribute("kinematic").as_bool();

				//-- create new shape.
				{
					auto shape = elem.child("shape");
					if (shape.empty())
						return false;

					std::string type = shape.attribute("type").value();

					auto params = shape.child("params");
					if (params.empty())
						return false;

					btCollisionShape* btShape = nullptr;

					//-- create desired collision shape.
					if (type == "box")
					{
						auto size = parseTo<vec3f>(params.attribute("size").value());

						btShape = new btBoxShape(btVector3(size.x, size.y, size.z));
					}
					else if (type == "cylinder" || type == "cylinderX" || type == "cylinderZ")
					{
						vec2f lr(0,0);

						auto attr = params.attribute("size");
						if (attr.empty())
						{
							auto bone1  = params.attribute("bone1").value();
							auto bone2  = params.attribute("bone2").value();

							Node* node1 = findNode(bone1, transform->m_nodes);
							Node* node2 = findNode(bone2, transform->m_nodes);

							vec3f dir = node2->matrix().applyToOrigin() - node1->matrix().applyToOrigin();

							lr.x = params.attribute("radius").as_float();
							lr.y = dir.length() * 0.5f;
							
							//-- make calculated offset.
							dir = node1->matrix().getInverted().applyToVector(dir);
							bodyDesc.m_offset = dir.scale(0.5f);
						}
						else
						{
							lr = parseTo<vec2f>(attr.value());
						}

						if		(type == "cylinder")	btShape = new btCylinderShape (btVector3(lr.x, lr.y, lr.x));
						else if (type == "cylinderX")	btShape = new btCylinderShapeX(btVector3(lr.y, lr.x, lr.x));
						else							btShape = new btCylinderShapeZ(btVector3(lr.x, lr.x, lr.y));
					}
					else
					{
						assert("type is not yet implemented.");
					}

					//-- calculate local inertia.
					btVector3 lInertia(0,0,0);
					if (bodyDesc.m_mass != 0.0f)
						btShape->calculateLocalInertia(bodyDesc.m_mass, lInertia);

					bodyDesc.m_localInertia = vec3f(lInertia);

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
	bool PhysObjDesc::create(PhysObj*& obj, Transform* transform, Handle objectID)
	{
		std::unique_ptr<PhysObj> physObj(new PhysObj);

		//-- 1. set transform.
		physObj->m_objectID  = objectID;
		physObj->m_transform = transform;
			
		//-- 2. create all rigid bodies.
		for (auto i = m_rigidBodyDescs.begin(); i != m_rigidBodyDescs.end(); ++i)
		{
			RigidBody* body = new RigidBody();

			body->m_name   = i->m_name.c_str();
			body->m_owner  = physObj.get();
			body->m_offset = &i->m_offset;

			//-- find anchor joint.
			body->m_node = findNode(i->m_node, transform->m_nodes);

			btRigidBody::btRigidBodyConstructionInfo rbInfo(
				i->m_mass, body, m_shapes[i->m_shape],
				btVector3(i->m_localInertia.x, i->m_localInertia.y, i->m_localInertia.z)
				);

			body->m_body = new btRigidBody(rbInfo);

			//-- set user pointer as a pointer to the PhysObj object.
			body->m_body->setUserPointer(body);

			//-- make it kinematic if needed.
			if (i->m_isKinematic)
			{
				body->m_body->setCollisionFlags(
					body->m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT
					);  
				body->m_body->setActivationState(DISABLE_DEACTIVATION); 
			}

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
	PhysObj::PhysObj() : m_transform(nullptr), m_objectID(CONST_INVALID_HANDLE)
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
	void PhysObj::addImpulse(const vec3f& dir, const vec3f& relPos)
	{
		m_bodies[0]->m_body->applyImpulse(
			bruge2bullet(dir), bruge2bullet(relPos)
			);
	}

	//----------------------------------------------------------------------------------------------
	PhysObjDesc::RigidBody::RigidBody()
		: m_name(nullptr), m_node(nullptr), m_owner(nullptr)
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
		//-- apply offset.
		mat4f transform(m_node->matrix());
		transform.preTranslation(*m_offset);

		worldTrans = bruge2bullet(transform);
	}


	//----------------------------------------------------------------------------------------------
	void PhysObjDesc::RigidBody::setWorldTransform(const btTransform& worldTrans)
	{
		//-- apply transform
		mat4f transform = bullet2bruge(worldTrans);
		transform.postTranslation(m_offset->scale(-1));

		//-- update AABB.
		//-- ToDo: optimize.
		{
			btVector3 aabbMin, aabbMax;
			btTransform identityTransform;
			identityTransform.setIdentity();
			m_body->getCollisionShape()->getAabb(
				identityTransform, aabbMin, aabbMax
				);
			
			Transform& tr = *m_owner->m_transform;
			tr.m_localBounds = AABB(bullet2bruge(aabbMin), bullet2bruge(aabbMax));
			tr.m_worldBounds = tr.m_localBounds.getTranformed(transform);
		}
		
		//-- update root node matrix.
		m_node->matrix(transform);
	}

	//----------------------------------------------------------------------------------------------
	void PhysDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		DebugDrawer::instance().drawLine(
			bullet2bruge(from), bullet2bruge(to),
			Color(color.x(), color.y(), color.z())
			);
	}

	//----------------------------------------------------------------------------------------------
	void PhysDebugDrawer::reportErrorWarning(const char* warningString)
	{
		ConWarning (warningString);
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