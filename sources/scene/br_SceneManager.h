/**
 * br_SceneManager.h
 * This file is part of "brUGE" graphical game engine
 * author Bronislau Svihla (BronX)
 * Copyright (C) 2006-2008 Bronislau Svihla
 */
#ifndef _BR_SCENE_MANAGER_H_
#define _BR_SCENE_MANAGER_H_

#include "br_Types.h"
#include "Console/br_Console.h"
#include "Utils/br_Singleton.h"
#include "SceneGraph/br_QuadTree.h"

#include <map>
#include <vector>

namespace brUGE{
	
	class brStaticObject;

	namespace scene
	{
		class FreeCamera;
	}

	namespace render
	{
		class brTerrain;
		class brRenderManager;
	}

	using namespace render;
	using namespace scene;

	class brSceneManager : public utils::Singleton<brSceneManager>{

		DECLARATE_SINGLETON(brSceneManager);

		friend class brRenderManager;			
	public:
		brSceneManager();
		~brSceneManager();

		void setSceneObject(brStaticObject *object);
		void setTerrain(brTerrain &terrain);
		void clenupScene();
		void updateQuadTree();
		void findObjects(Camera *camera);

		brStaticObject *getSceneObject(Handle _handle);

	private:
		void _addSimpleObject(const  ParamList &params);

	private:
		typedef std::map<Handle, brStaticObject*>				sceneObjectMap;
		typedef std::pair<Handle, brStaticObject*>				mapPair;
		typedef std::vector<brStaticObject*>					objectArray;
		typedef std::vector<brStaticObject*>::iterator			vecIterator;
		typedef std::map<Handle, brStaticObject*>::iterator	mapIterator;

		sceneObjectMap	objectMap;
		mapIterator		mapIter;
		Handle			objectCount;
		objectArray		oArray;
		vecIterator		vecIter;

		//Scene manager for quickly find render objects in world
		brQuadTree *quadTree;
	};

}/*end namespace brUGE*/

#endif/*_BR_SCENE_MANAGER_H_*/

/**
	* 21/11/2007
		created.
*/