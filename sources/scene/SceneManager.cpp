#include "SceneGraph/br_SceneManager.h"
#include "SceneGraph/br_StaticObject.h"
#include "Renderer/br_BaseCamera.h"

#include <assert.h>

namespace brUGE{

	DEFINE_SINGLETON(brSceneManager);
		
	using namespace utils;
	
	//------------------------------------------
	brSceneManager::brSceneManager(){
		quadTree = new brQuadTree();
		quadTree->buildTree(4, 4, Rect2d(init_vec2(-250.0f, -250.0f), init_vec2(250.0f, 250.0f)));

		// Регистрация консольной комманды.
		//------------------------------------------
		REGISTER_CONSOLE_METHOD("addSimpleMesh",_addSimpleObject, brSceneManager, this);
	}
	
	//------------------------------------------
	brSceneManager::~brSceneManager(){

	}
	
	//------------------------------------------
	brStaticObject *brSceneManager::getSceneObject(Handle _handle){
		mapIter = objectMap.find(_handle);
		if(mapIter != objectMap.end()){
			return mapIter->second;
		}else{
			return NULL;
		}	
	}
	
	//------------------------------------------
	void brSceneManager::setSceneObject(brStaticObject *_object)
	{
		if(objectMap.find(++objectCount) == objectMap.end())
		{
			objectMap.insert(mapPair(objectCount, _object));
			oArray.push_back(_object);
			quadTree->addObject(*_object);
		}
	}

	void brSceneManager::setTerrain(brTerrain &terrain)
	{

	}
	
	//------------------------------------------
	void brSceneManager::clenupScene(){
		for(mapIter = objectMap.begin(); mapIter != objectMap.end(); ++mapIter){
			if(mapIter->second)
				delete mapIter->second;
		}

		for(vecIter = oArray.begin(); vecIter != oArray.end(); ++vecIter){
			delete *vecIter;
		}
	}
	
	
	//------------------------------------------
	void brSceneManager::updateQuadTree(){
		//TODO: обновить положение объектов, если они покинули свои ноды.
	}
	
	
	//------------------------------------------
	void brSceneManager::findObjects(Camera *camera){
		//TODO: set flag isCulled in false or true to all render entities
	}
	
	//------------------------------------------
	void brSceneManager::_addSimpleObject(const ParamList &params){
		if (params.size() < 3){
			ConWarning(" * position[vec3], (scale[vec3], rotate[vec3]{yaw, pitch, roll - in degrees}, meshName[string])");
			return;
		}

		std::string meshName;
		vec3f pos		= init_vec3(0.0f, 0.0f, 0.0f);
		vec3f scale		= init_vec3(1.0f, 1.0f, 1.0f);
		vec3f rotate	= init_vec3(0.0f, 0.0f, 0.0f);

		pos[0] = params[0].toFloat();
		pos[1] = params[1].toFloat();
		pos[2] = params[2].toFloat();

		if (params.size() >= 6){
			scale[0] = params[3].toFloat();
			scale[1] = params[4].toFloat();
			scale[2] = params[5].toFloat();
		}

		if (params.size() >= 9){
			rotate[0] = DEG_TO_RAD(params[6].toFloat());
			rotate[1] = DEG_TO_RAD(params[7].toFloat());
			rotate[2] = DEG_TO_RAD(params[8].toFloat());
		}

		if (params.size() >= 10)
			meshName = params[9];
			
		//plane
		brRenderEntity *tSM = new brRenderEntity();
		brStaticObject *tSO = new brStaticObject();
		tSM->enableVBOUsing(true);
		tSM->enableDebug(true);

		if (meshName.length()){
			tSM->load(meshName);
		}else{
			tSM->load("pinky.sf1");
		}

		tSM->setLightingType(LT_STDLIGHTING);

		tSM->init(pos, 
				  scale,
				  get_rot_mat3X3(rotate));
		tSO->loadRenderEntity(*tSM);
		setSceneObject(tSO);
	}

}/*end namespace brUGE*/