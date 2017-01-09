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


//--------------------------------------------------------------------------------------------------
namespace
{
	//----------------------------------------------------------------------------------------------
	Node* findNode(const std::string& name, Nodes& nodes)
	{
		auto iter = std::find_if(nodes.begin(), nodes.end(), [name](const auto& node) {
			return (name == node->name());
		});

		assert(iter != nodes.end());

		return iter->get();
	}

	//-- physx to\from bruge conversion functions
	//----------------------------------------------------------------------------------------------
	inline PxVec3		bruge2physx(const vec3f& v)			{ return PxVec3(v.x, v.y, v.z); }
	inline vec3f		physx2bruge(const PxVec3& v)		{ return vec3f(v.x, v.y, v.z); }
	inline PxTransform	bruge2physx(const mat4f& m)			{ return PxTransform(PxMat44(const_cast<mat4f&>(m).data)); }
	inline mat4f		physx2bruge(const PxMat44& m)		{ return mat4f(m.front()); }
	inline mat4f		physx2bruge(const PxTransform& t)	{ return mat4f(PxMat44(t).front()); }

	//--
	bool g_debugDrawEnabled = false;
}


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
		m_physObjs.clear();
		m_physObjTypes.clear();
		
		m_scene->release();
		m_dispatcher->release();
		auto* profileZoneManager = m_physics->getProfileZoneManager();
		if (m_debuggerConnection)
			m_debuggerConnection->release();
		PxCloseExtensions();
		m_physics->release();
		profileZoneManager->release();
		m_foundation->release();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::init()
	{
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
		auto* profileZoneManager = &PxProfileZoneManager::createProfileZoneManager(m_foundation);

		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, profileZoneManager);
		if (!PxInitExtensions(*m_physics))
			return false;

		if (m_physics->getPvdConnectionManager())
		{
			m_physics->getVisualDebugger()->setVisualizeConstraints(true);
			m_physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
			m_physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
			m_physics->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS, true);
			m_debuggerConnection = PxVisualDebuggerExt::createConnection(m_physics->getPvdConnectionManager(), "127.0.0.1", 5425, 10);
		}

		{
			//-- ToDo: for now we don't create any worker threads
			m_dispatcher = PxDefaultCpuDispatcherCreate(0);

			PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
			sceneDesc.gravity		= PxVec3(0.0f, -9.81f, 0.0f);
			sceneDesc.cpuDispatcher = m_dispatcher;
			sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
			sceneDesc.flags			= PxSceneFlag::eENABLE_ACTIVETRANSFORMS;

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
			factory = m_physObjTypes[desc].get();
		}

		//-- instantiate phys obj of the particular type
		if (auto instance = factory->createInstance(transform, gameObj))
		{
			instance->m_physObj = m_physObjs.size();
			instance->enterScene(m_scene);
			m_physObjs.push_back(std::move(instance));
			return m_physObjs.size() - 1;
		}

		return CONST_INVALID_HANDLE;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::removePhysicsObject(Handle physObj)		
	{
		assert(static_cast<uint32>(physObj) < m_physObjs.size() && m_physObjs[physObj]);

		m_physObjs[physObj]->leaveScene(m_scene);
		m_physObjs[physObj].reset();
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::simulate(float dt)
	{
		updatePhysicsTransforms();

		m_scene->simulate(dt);
		m_scene->fetchResults(true);

		updateGraphicsTransforms();

		//-- debug draw
		if (g_debugDrawEnabled)
		{
			debugDraw();
		}
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
	void PhysicsWorld::makeKinematic(Handle physObj, bool flag)
	{
		const auto& instance = m_physObjs[physObj];

		for (auto& body : instance->m_bodies)
		{
			body->m_actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, flag);
		}
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::addImpulse(Handle physObj, const vec3f& impulse, const vec3f& wPos)
	{
		const auto& instance = m_physObjs[physObj];
		auto&		body	 = *instance->m_bodies[0]->m_actor;

		if (body.getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
			return;

		PxRigidBodyExt::addForceAtPos(
			body, PxVec3(impulse.x, impulse.y, impulse.z), PxVec3(wPos.x, wPos.y, wPos.z), PxForceMode::eIMPULSE
		);
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsWorld::collide(CollisionCallback& cc, const vec3f& start, const vec3f& end) const
	{
		PxVec3 pxStart(start.x, start.y, start.z);
		PxVec3 pxEnd(end.x, end.y, end.z);
		PxVec3 pxUnitDir = (pxEnd - pxStart); 
		PxReal pxMaxDistance = pxUnitDir.normalize(); 
		PxRaycastBuffer pxHit;

		bool status = m_scene->raycast(pxStart, pxUnitDir, pxMaxDistance, pxHit);

		if (status)
		{
			auto& block = pxHit.block;
			auto body	= static_cast<PhysicsObjectType::RigidBody*>(block.actor->userData);

			cc.m_wPos		= vec3f(&block.position[0]);
			cc.m_wNormal	= vec3f(&block.normal[0]);
			cc.m_distance	= block.distance;

			if (body)
			{
				cc.m_node	 = body->m_node;
				cc.m_gameObj = body->m_owner->m_gameObj;
				cc.m_physObj = body->m_owner->m_physObj;
			}

			return true;
		}

		return false;
	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::PhysicsObjectType()
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::~PhysicsObjectType()
	{
		for (auto& shape : m_shapes)
			shape->release();

		for (auto& material : m_materials)
			material->release();

		m_materials.clear();
		m_shapes.clear();
	}

	//----------------------------------------------------------------------------------------------
	bool PhysicsObjectType::load(const ROData& data)
	{
		pugi::xml_document doc;
		if (!doc.load_buffer(data.ptr(), data.length()))
			return false;

		//-- check root element.
		auto root = doc.document_element();

		//-- ToDo: default material
		m_materials.emplace_back(PxGetPhysics().createMaterial(0.5f, 0.5f, 0.5f));

		//-- read <rigidBodies> section.
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

					PxTransform localTransform(bruge2physx(bodyDesc.m_offset));
					PxShape* pxShape = nullptr;

					//-- create desired collision shape.
					if (type == "box")
					{
						auto size = parseTo<vec3f>(params.attribute("size").value());

						pxShape = PxGetPhysics().createShape(PxBoxGeometry(size.x, size.y, size.z), *m_materials[0]);
					}
					else if (type == "capsule" || type == "capsuleX" || type == "capsuleZ")
					{
						auto radius		= params.attribute("radius").as_float();
						auto halfHeight = params.attribute("halfHeight").as_float();

						if (type == "capsule")
							localTransform = localTransform * PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
						else if (type == "capsuleZ")
							localTransform = localTransform * PxTransform(PxQuat(PxHalfPi, PxVec3(0, 1, 0)));

						pxShape = PxGetPhysics().createShape(PxCapsuleGeometry(radius, halfHeight), *m_materials[0]);
					}
					else if (type == "sphere")
					{
						auto radius = params.attribute("radius").as_float();

						pxShape = PxGetPhysics().createShape(PxSphereGeometry(radius), *m_materials[0]);
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

		//-- read <joints> section.
		{
			auto elems = root.child("joints");

			for (auto elem = elems.child("joint"); elem; elem = elem.next_sibling("joint"))
			{
				Joint::Desc jointDesc;

				jointDesc.m_name	= elem.attribute("name").value();
				jointDesc.m_type	= elem.attribute("type").value();
				jointDesc.m_objA	= elem.attribute("objA").value();
				jointDesc.m_objB	= elem.attribute("objB").value();
				jointDesc.m_offsetA = parseTo<vec3f>(elem.attribute("offsetA").value());
				jointDesc.m_offsetB = parseTo<vec3f>(elem.attribute("offsetB").value());

				m_jointDescs.push_back(jointDesc);
			}
		}

		return true;
	}

	//----------------------------------------------------------------------------------------------
	std::unique_ptr<PhysicsObjectType::Instance> PhysicsObjectType::createInstance(Transform* transform, Handle gameObj)
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
			body->m_actor	= PxGetPhysics().createRigidDynamic(PxTransform(bruge2physx(body->m_node->matrix())));

			body->m_actor->userData = body.get();
			body->m_actor->attachShape(*m_shapes[desc.m_shapeIdx]);
			body->m_actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, desc.m_isKinematic);

			PxRigidBodyExt::updateMassAndInertia(*body->m_actor, 10.0f);

			instance->m_bodies.push_back(std::move(body));
		}

		//-- 3. create all joints.
		for (const auto& desc : m_jointDescs)
		{
			auto joint = std::make_unique<Joint>();

			joint->m_name = desc.m_name.c_str();

			auto b0 = instance->findBody(desc.m_objA);
			auto b1 = instance->findBody(desc.m_objB);

			PxTransform t0(bruge2physx(desc.m_offsetA));
			PxTransform t1(bruge2physx(desc.m_offsetB));

			if (desc.m_type == "spheric")
			{
				auto* pxJoint = PxSphericalJointCreate(PxGetPhysics(), b0->m_actor, t0, b1->m_actor, t1);
				pxJoint->setLimitCone(PxJointLimitCone(PxPi / 4, PxPi / 4, 0.05f));
				pxJoint->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);
				pxJoint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);

				joint->m_pxJoint = pxJoint;
			}
			else if (desc.m_type == "revolute")
			{
				auto* pxJoint = PxRevoluteJointCreate(PxGetPhysics(), b0->m_actor, t0, b1->m_actor, t1);
				pxJoint->setLimit(PxJointAngularLimitPair(-PxPi / 4, PxPi / 4, 0.01f));
				pxJoint->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
				pxJoint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);

				joint->m_pxJoint = pxJoint;
			}

			instance->m_joints.push_back(std::move(joint));
		}
	
		return instance;
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsObjectType::Instance::enterScene(physx::PxScene* scene)
	{
		for (const auto& body : m_bodies)
		{
			scene->addActor(*body->m_actor);
		}
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsObjectType::Instance::leaveScene(physx::PxScene* scene)
	{
		for (const auto& body : m_bodies)
		{
			scene->removeActor(*body->m_actor);
		}
	}
	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::RigidBody* PhysicsObjectType::Instance::findBody(const std::string& name)
	{
		for (auto& body : m_bodies)
		{
			if (body->m_name == name)
				return body.get();
		}

		return nullptr;
	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::Joint* PhysicsObjectType::Instance::findJoint(const std::string& name)
	{
		for (auto& joint : m_joints)
		{
			if (joint->m_name == name)
				return joint.get();
		}
		return nullptr;
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
	PhysicsObjectType::Joint::Joint() : m_name(nullptr), m_pxJoint(nullptr)
	{

	}

	//----------------------------------------------------------------------------------------------
	PhysicsObjectType::Joint::~Joint()
	{
		m_pxJoint->release();
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::updateGraphicsTransforms()
	{
		// retrieve array of actors that moved
		PxU32 nbActiveTransforms = 0;
		auto* activeTransforms = m_scene->getActiveTransforms(nbActiveTransforms);

		// update each render object with the new transform
		for (PxU32 i = 0; i < nbActiveTransforms; ++i)
		{
			auto& entry = activeTransforms[i];
			auto* body  = static_cast<PhysicsObjectType::RigidBody*>(entry.userData);
			body->m_node->matrix(physx2bruge(entry.actor2World));

			//-- update AABB.
			//-- ToDo: optimize.
			{
				const auto& aabb = entry.actor->getWorldBounds();
				body->m_owner->m_transform->m_worldBounds = AABB(physx2bruge(aabb.minimum), physx2bruge(aabb.maximum));
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::updatePhysicsTransforms()
	{
		PxU32 nbActors = m_scene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			m_scene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC, (PxActor**)&actors[0], nbActors);

			for (const auto& actor : actors)
			{
				if (auto* rigidDynamic = actor->isRigidDynamic())
				{
					if (rigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
					{
						auto* body    = static_cast<PhysicsObjectType::RigidBody*>(actor->userData);

						rigidDynamic->setKinematicTarget(bruge2physx(body->m_node->matrix()));
					}
				}
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	void PhysicsWorld::debugDraw()
	{
		PxU32 nbActors = m_scene->getNbActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC);
		if (nbActors)
		{
			std::vector<PxRigidActor*> actors(nbActors);
			m_scene->getActors(PxActorTypeSelectionFlag::eRIGID_DYNAMIC, (PxActor**)&actors[0], nbActors);

			for (auto actor : actors)
			{
				auto shapeCount = actor->getNbShapes();
				std::vector<PxShape*> shapes(shapeCount);
				actor->getShapes((PxShape**)&shapes[0], shapeCount);

				for (auto shape : shapes)
				{
					auto transform = PxMat44(PxShapeExt::getGlobalPose(*shape, *actor));

					PxGeometryHolder h = shape->getGeometry();

					switch (h.getType())
					{
					case PxGeometryType::eBOX:
						DebugDrawer::instance().drawBox(
							physx2bruge(h.box().halfExtents).scale(2.0f), mat4f(transform.front()), Color(0, 1, 0, 0));
					break;
					case PxGeometryType::eCAPSULE:
						//-- PhysX uses OX axis as an up vector and we use OY.
						DebugDrawer::instance().drawCapsule(
							h.capsule().radius, h.capsule().halfHeight, physx2bruge(transform).preRotateZ(degToRad(90.0f)), Color(0, 0, 1, 0));
						break;
					case PxGeometryType::eSPHERE:
						DebugDrawer::instance().drawSphere(h.sphere().radius, physx2bruge(transform), Color(1, 0, 0, 0));
						break;
					case PxGeometryType::eCONVEXMESH:
					case PxGeometryType::eTRIANGLEMESH:
						break;
					default:
						break;
					}
				}
			}
		}

		//-- wireframe draw
	}


} //-- physic
} //-- brUGE