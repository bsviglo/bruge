#include "game_world.hpp"

#include "engine/Engine.h"
#include "os/FileSystem.h"
#include "render/render_system.hpp"
#include "render/render_world.hpp"
#include "render/physic_world.hpp"

//-- http://pugixml.org/
#include "pugixml/pugixml.hpp"


namespace brUGE
{
	using namespace utils;
	using namespace os;
	using namespace render;

	//----------------------------------------------------------------------------------------------
	bool GameWorld::init()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool GameWorld::fini()
	{
		for (uint i = 0; i < m_objs.size(); ++i)
		{
			delete m_objs[i];
		}
		m_objs.clear();

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

		//-- 1. load render part of the game object.
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

				m_meshInst = Engine::instance().renderWorld().addMeshDef(desc, &m_transform);
			}
		}

		//-- 2. setup transform of the game object.
		{
			MeshInstance* meshInst = Engine::instance().renderWorld().getMeshDef(m_meshInst);

			//-- 2.1. setup root matrix.
			if (orient)	m_transform.m_worldMat = *orient;
			else		m_transform.m_worldMat.setIdentity();

			//-- 2.2. setup nodes bucket.
			m_transform.m_nodes.push_back(new Node("root", m_transform.m_worldMat));

			//-- 2.3. if model is animated, then gather all the skeleton bones as nodes.
			if (meshInst->m_skinnedMesh)
			{
				//-- 2.3.1. resize mesh world palette to match the bones count in the skinned mesh.
				meshInst->m_worldPalette.resize(meshInst->m_skinnedMesh->skeleton().size());

				//-- 2.3.2. initialize nodes. 
				for (uint i = 0; i < meshInst->m_worldPalette.size(); ++i)
				{
					const Joint& joint   = meshInst->m_skinnedMesh->skeleton()[i];
					mat4f&		 nodeMat = meshInst->m_worldPalette[i];

					m_transform.m_nodes.push_back(new Node(joint.m_name.c_str(), nodeMat));
				}
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
				const char* desc = physicsNode.attribute("file").value();

				m_physObj = Engine::instance().physicWorld().addPhysicDef(desc, &m_transform, objID);
			}
		}

		//-- ToDo:
		m_animCtrl = CONST_INVALID_HANDLE;

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