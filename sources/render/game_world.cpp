#include "game_world.hpp"

#include "engine/Engine.h"
#include "os/FileSystem.h"
#include "render/animation_engine.hpp"
#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/mesh_manager.hpp"
#include "render/physic_world.hpp"

//-- http://pugixml.org/
#include "pugixml/pugixml.hpp"

using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::os;
using namespace brUGE::render;

namespace brUGE
{
	//----------------------------------------------------------------------------------------------
	GameWorld::GameWorld()
	{

	}

	//----------------------------------------------------------------------------------------------
	GameWorld::~GameWorld()
	{
		for (uint i = 0; i < m_objs.size(); ++i)
		{
			delete m_objs[i];
		}
		m_objs.clear();
	}
	
	//----------------------------------------------------------------------------------------------
	bool GameWorld::init()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::loadMap(const char* /*mapName*/)
	{
		assert(0);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::saveMap(const char* /*mapName*/)
	{
		assert(0);

		return false;
	}

	//----------------------------------------------------------------------------------------------
	Handle GameWorld::addGameObj(const char* desc, const mat4f* orient/* = NULL*/)
	{
		std::unique_ptr<IGameObj> obj(new IGameObj());

		RODataPtr data = os::FileSystem::instance().readFile(desc);
		if (!data)
			return CONST_INVALID_HANDLE;

		if (!obj->load(*data.get(), m_objs.size(), orient))
		{
			return CONST_INVALID_HANDLE;
		}
		else
		{
			m_objs.push_back(obj.release());
			return m_objs.size() - 1;
		}
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::delGameObj(Handle handle)
	{
		if (handle == CONST_INVALID_HANDLE || static_cast<size_t>(handle) >= m_objs.size())
			return false;

		delete m_objs[handle];
		m_objs[handle] = m_objs.back();
		m_objs.pop_back();

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::beginUpdate(float /*dt*/)
	{
		
	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::preAnimUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::postAnimUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	void GameWorld::endUpdate()
	{

	}

	//----------------------------------------------------------------------------------------------
	bool IGameObj::load(const ROData& data, Handle objID, const mat4f* orient/* = NULL*/)
	{
		pugi::xml_document	   doc;
		pugi::xml_parse_result result = doc.load_buffer(data.ptr(), data.length());

		if (!result)
			return false;

		pugi::xml_node objectDesc = doc.document_element();

		MeshManager& meshManager = Engine::instance().renderWorld().meshManager();

		//-- 1. setup root matrix.
		{
			if (orient)	m_transform.m_worldMat = *orient;
			else		m_transform.m_worldMat.setIdentity();

			m_transform.m_nodes.push_back(new Node("root", m_transform.m_worldMat));
		}

		//-- 2. load render part of the game object.
		{
			pugi::xml_node renderNode = objectDesc.child("render");
			if (renderNode.empty())
			{
				m_meshInst = CONST_INVALID_HANDLE;
			}
			else
			{
				MeshInstance::Desc desc;
				desc.fileName = renderNode.attribute("file").value();

				m_meshInst = meshManager.addMesh(desc, &m_transform);
			}
		}

		//-- 3. parse physics/collision data of the game object.
		{
			pugi::xml_node physicsNode = objectDesc.child("physics");
			if (physicsNode.empty())
			{
				m_physObj = nullptr;
			}
			else
			{
				if (auto desc = physicsNode.attribute("file"))
				{
					m_physObj = Engine::instance().physicWorld().addPhysicDef(
						desc.value(), &m_transform, objID
						);
				}
			}
		}

		//-- 4. setup animation part.
		{
			MeshInstance& meshInst = meshManager.getMesh(m_meshInst);

			if (meshInst.m_skinnedMesh)
			{
				AnimationData::Desc desc;
				desc.m_idleAnim  = nullptr;
				desc.m_meshInst  = &meshInst;
				desc.m_transform = &m_transform;

				m_animCtrl = Engine::instance().animationEngine().addAnimDef(desc);
			}
			else
			{
				m_animCtrl = CONST_INVALID_HANDLE;
			}
		}

		//-- store its own unique handle in the world after complete successfully initialization.
		m_self = objID;

		return true;

		//-- 3. setup animation data.
		/*
		if (mInst->m_skinnedMesh)
		{
			AnimationData* aInst = new AnimationData;
			aInst->m_meshInst = mInst;
			aInst->m_transform = &m_transform;
		}
		else
		{
			m_animCtrl = CONST_INVALID_HANDLE;

			m_animCtrl = Engine()::instance().animationEngine().addAnimDef(aInst);
		}

		//-- 4. setup transform of the game object.

		//-- 4.1. setup root matrix.
		if (orient)	m_transform.m_worldMat = *orient;
		else		m_transform.m_worldMat.setIdentity();
		
		//-- 4.2. setup initial bounds.
		m_transform.m_localBounds = mInst->m_skinnedMesh->bounds();

		//-- 4.3. setup nodes bucket.
		m_transform.m_nodes.push_back(new Node("root", m_transform.m_worldMat));
		
		//-- 4.4. if model is animated, then gather all the skeleton bones as nodes.
		if (mInst->m_skinnedMesh)
		{
			//-- 4.4.1. resize mesh world palette to match the bones count in the skinned mesh.
			mInst->m_worldPalette.resize(mInst->m_skinnedMesh->skeleton().size());

			//-- 4.4.2. initialize nodes. 
			for (uint i = 0; i < mInst->m_worldPalette.size(); ++i)
			{
				const Joint& joint   = mInst->m_skinnedMesh->skeleton()[i];
				const mat4f& nodeMat = mInst->m_worldPalette[i];

				m_transform.m_nodes.push_back(new Node(joint.m_name.c_str(), nodeMat));
			}
		}

		//-- 5. final setup.
		m_meshInst = Engine()::instance().renderWorld().addMeshDef(mInst);
		*/
	}

	//----------------------------------------------------------------------------------------------
	bool IGameObj::save(WOData& /*inData*/)
	{
		assert(0);
		return false;
	}

	//----------------------------------------------------------------------------------------------
	Transform::Transform()
	{

	}

	//----------------------------------------------------------------------------------------------
	Transform::~Transform()
	{
		for (uint i = 0; i < m_nodes.size(); ++i)
			delete m_nodes[i];

		m_nodes.clear();
	}

} //-- brUGE