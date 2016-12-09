#include "physic_world.hpp"
#include "scene/game_world.hpp"
#include "math/Matrix4x4.hpp"
#include "pugixml/pugixml.hpp"
#include "utils/Data.hpp"
#include "utils/ArgParser.h"
#include "os/FileSystem.h"
#include "render/DebugDrawer.h"
#include "render/Color.h"
#include "render/Mesh.hpp"
#include <algorithm>

using namespace physx;
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
namespace physics
{

	//----------------------------------------------------------------------------------------------
	PhysicsWorld::PhysicsWorld() : m_foundation(nullptr), m_physics(nullptr), m_scene(nullptr), m_dispatcher(nullptr), m_debuggerConnection(nullptr)
	{
		//-- register console funcs.
		REGISTER_CONSOLE_METHOD("phys_drawWire", _drawWire, PhysicsWorld);
		REGISTER_CONSOLE_METHOD("phys_drawAABB", _drawAABB, PhysicsWorld);
	}

	//----------------------------------------------------------------------------------------------
	PhysicsWorld::~PhysicsWorld()
	{
		//-- ToDo:
		for (auto i = m_physObjDescs.begin(); i != m_physObjDescs.end(); ++i)
			delete i->second;
		
		m_scene->release();
		m_dispatcher->release();
		if (m_debuggerConnection)
			m_debuggerConnection->release();
		m_physics->release();
		m_physics->getProfileZoneManager()->release();
		m_foundation->release();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::init()
	{
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
		auto* profileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_foundation);

		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, profileZoneManager);

		if (m_physics->getPvdConnectionManager())
		{
			m_physics->getVisualDebugger()->setVisualizeConstraints(true);
			m_physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
			m_physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
			m_debuggerConnection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 10);
		}

		{
			//-- ToDo: for now we don't create any worker threads
			m_dispatcher = PxDefaultCpuDispatcherCreate(0);

			PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
			sceneDesc.gravity		= PxVec3(0.0f, -9.81f, 0.0f);
			sceneDesc.cpuDispatcher = m_dispatcher;
			sceneDesc.filterShader	= PxDefaultSimulationFilterShader;

			m_scene = m_physics->createScene(sceneDesc);
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	Handle PhysicsWorld::createPhysicsObject(const char* desc, Transform* transform, Handle gameObj)
	{
		PhysicsObjectType* factory = nullptr;

		auto result = m_physObjTypes.find(desc);
		if (result != m_physObjTypes.end())
		{
			factory = result->second.get();
		}
		else
		{
			auto data = FileSystem::instance().readFile("resources/" + std::string(desc));	

			auto newType = std::make_unique<PhysicsObjectType>();
			if (!data || !newType->load(*data.get()))
			{
				return CONST_INVALID_HANDLE;
			}

			//-- add new phys descriptor.
			m_physObjTypes[desc] = std::move(newType);
		}

		return factory->createInstance(transform, gameObj);
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::removePhysicsObject(Handle physObj)		
	{
		for (auto i = m_physObjDescs.begin(); i != m_physObjDescs.end(); ++i)
		{
			if (i->second->removeInstance(physObj))
				return true;
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::simulate(float dt)
	{
		m_scene->simulate(dt);
		m_scene->fetchResults(true);
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::createTerrainPhysicsObject(uint, float, float*, float, float, float)
	{
#if 0
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
#endif

		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::removeTerrainPhysicsObject()
	{
#if 0
		if (!m_terrain.m_shape || !m_terrain.m_rigidBody)
			return true;

		//-- remove terrain from physics world.
		m_dynamicsWorld->removeRigidBody(m_terrain.m_rigidBody);

		//-- delete.
		delete m_terrain.m_rigidBody;
		delete m_terrain.m_shape;
#endif
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::collide(const vec3f& origin, const vec3f& dir) const
	{
		btVector3 start = bruge2bullet(origin);
		btVector3 end   = bruge2bullet(origin + dir.scale(1000.0f));

		btCollisionWorld::ClosestRayResultCallback cb(start, end);
		m_dynamicsWorld->rayTest(start, end, cb);

		return cb.hasHit();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::collide(vec3f& out, const vec3f& start, const vec3f& end) const
	{
		btVector3 btStart = bruge2bullet(start);
		btVector3 btEnd   = bruge2bullet(end);

		btCollisionWorld::ClosestRayResultCallback cb(btStart, btEnd);
		m_dynamicsWorld->rayTest(btStart, btEnd, cb);

		if (cb.hasHit())
		{
			out = bullet2bruge(cb.m_hitPointWorld);
			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::collide(mat4f& localMat, Node*& node, const vec3f& start, const vec3f& end) const
	{
		Handle objID = CONST_INVALID_HANDLE;
		return collide(localMat, node, objID, start, end);
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::collide(
		mat4f& localMat, Node*& node, Handle& objID, const vec3f& start, const vec3f& end) const
	{
		btVector3 btStart = bruge2bullet(start);
		btVector3 btEnd   = bruge2bullet(end);

		btCollisionWorld::ClosestRayResultCallback cb(btStart, btEnd);
		m_dynamicsWorld->rayTest(btStart, btEnd, cb);

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

			node  = body->m_node;
			objID = body->m_owner->m_objectID;

			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::PhysicsObjectType(PxPhysics* physics) : m_physics(physics)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::~PhysicsObjectType()
	{
		assert(m_instances.size() == 0);

		for (auto& shape : m_shapes)
			shape->release();

		for (auto& material : m_materials)
			material->release();

		m_materials.clear();
		m_shapes.clear();
		m_physics = nullptr;
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsObjectType::load(const ROData& data)
	{
		//-- try to create DOM model from the input buffer.
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
			return false;

		//-- check root element.
		auto root = doc.document_element();

		//-- ToDo: default material
		m_materials.emplace_back(m_physics->createMaterial(0.5f, 0.5f, 0.5f));

		//-- try to read <rigidBodies> section.
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

					PxTransform localTransform(bodyDesc.m_offset.x, bodyDesc.m_offset.y, bodyDesc.m_offset.z);
					PxShape* pxShape = nullptr;

					//-- create desired collision shape.
					if (type == "box")
					{
						auto size = parseTo<vec3f>(params.attribute("size").value());

						pxShape = m_physics->createShape(PxBoxGeometry(size.x, size.y, size.z), *m_materials[0]);
					}
					else if (type == "capsule" || type == "capsuleX" || type == "capsuleZ")
					{
						auto radius		= params.attribute("radius").as_float();
						auto halfHeight = params.attribute("halfHeight").as_float();

						if (type == "capsule")
							localTransform = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))) * localTransform;
						else if (type == "capsuleZ")
							localTransform = PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0))) * localTransform;

						pxShape = m_physics->createShape(PxCapsuleGeometry(radius, halfHeight), *m_materials[0]);
					}
					else if (type == "sphere")
					{
						auto radius = params.attribute("radius").as_float();

						pxShape = m_physics->createShape(PxSphereGeometry(radius), *m_materials[0]);
					}
					else
					{
						assert(!"Undefined rigid body type.");
					}

					pxShape->setLocalPose(localTransform);

					//-- add new shape to cache.
					m_shapes.push_back(pxShape);

					bodyDesc.m_shapeIdx = m_shapes.size() - 1;

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
	Handle PhysicsObjectType::createInstance(Transform* transform, Handle gameObj)
	{
		auto instance = std::make_unique<Instance>();

		//-- 1. set transform.
		instance->m_gameObj   = gameObj;
		instance->m_transform = transform;
			
		//-- 2. create all rigid bodies.
		for (const auto& desc : m_rigidBodyDescs)
		{
			auto body = std::make_unique<RigidBody>();

			body->m_name	= desc.m_name.c_str();
			body->m_node	= findNode(desc.m_node, transform->m_nodes);
			body->m_owner	= instance.get();
			body->m_actor	= m_physics->createRigidDynamic(PxTransform(PxMat44(transform->m_worldMat.data)));

			body->m_actor->userData = &body;
			body->m_actor->attachShape(*m_shapes[desc.m_shapeIdx]);
			body->m_actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, desc.m_isKinematic);

			PxRigidBodyExt::updateMassAndInertia(*body->m_actor, 10.0f);

			instance->m_bodies.push_back(body);
		}

		//-- 3. create all constraints.
		//for (const auto& desc : m_constraintDescs)
		//{
		//
		//}
	
		m_instances.push_back(instance);
		return m_instances.size() - 1;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsObjectType::removeInstance(Handle instance)
	{
		assert(instance != INVALID_HANDLE_VALUE && instance < m_instances.size());

		m_instances[instance].reset();
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
		m_bodies[0]->m_body->activate(true);
		m_bodies[0]->m_body->applyImpulse(
			bruge2bullet(dir), bruge2bullet(relPos)
			);
	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::RigidBody::RigidBody()
		: m_name(nullptr), m_node(nullptr), m_owner(nullptr), m_actor(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::RigidBody::~RigidBody()
	{
		m_actor->release();
		m_actor = nullptr;
		m_name = nullptr;
		m_node = nullptr;
		m_owner = nullptr;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsObjectType::RigidBody::getWorldTransform(btTransform& worldTrans) const
	{
		//-- apply offset.
		mat4f transform(m_node->matrix());
		transform.preTranslation(*m_offset);

		worldTrans = bruge2bullet(transform);
	}


	//----------------------------------------------------------------------------------------------
	void PhysicsObjectType::RigidBody::setWorldTransform(const btTransform& worldTrans)
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
	int PhysicsWorld::_drawWire(bool flag)
	{
		m_debugDrawer.setDebugMode(
			flag ? btIDebugDraw::DBG_DrawWireframe : !btIDebugDraw::DBG_DrawWireframe
			);
		return 0;
	}

	//----------------------------------------------------------------------------------------------
	int PhysicsWorld::_drawAABB(bool flag)
	{
		m_debugDrawer.setDebugMode(
			flag ? btIDebugDraw::DBG_DrawAabb : !btIDebugDraw::DBG_DrawAabb
			);
		return 0;
	}

} //-- physic
} //-- brUGE