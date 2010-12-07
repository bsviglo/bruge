#include "Sound/SimpleSSManager.h"
#include "Sound/SRManager.h"
#include "Sound/AmbientSS.h"
#include "Loader/br_ResourceManager.h"
#include <iostream>

namespace SoundSys{

	// 
	//------------------------------------------
	SimpleSSManager::SimpleSSManager(){
		
	}
	
	// 
	//------------------------------------------
	SimpleSSManager::~SimpleSSManager(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter)
			if(mapIter->second)
				delete mapIter->second;
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::update(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter){
//			std::cout << "Update ss name : " << mapIter->second->getSSName() << std::endl;
			mapIter->second->update();
		}
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::playAll(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter){		
			mapIter->second->play();
		}		
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::play(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end())
			mapIter->second->play();
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::pauseAll(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter){		
			mapIter->second->pause();
		}		
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::pause(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end())
			mapIter->second->pause();
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::stopAll(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter){		
			mapIter->second->stop();
		}		
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::stop(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end())
			mapIter->second->stop();
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::rewindAll(){
		for(mapIter = ssMap.begin(); mapIter != ssMap.end(); ++mapIter){		
			mapIter->second->rewind();
		}		
	}
	
	// 
	//------------------------------------------
	void SimpleSSManager::rewind(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end())
			mapIter->second->rewind();
	}
	
	// 
	//------------------------------------------
	int SimpleSSManager::clearSoundScene(){
		//stub
		return 0;
	}
	
	// 
	//------------------------------------------
	iSoundSource* SimpleSSManager::createSoundSource( const brString &_ssName,
													  const brString &_sName,
													  const brString &s_ssType,
													  bool _loop,
													  bool _stream,
													  int _bufCount){
		if(ssMap.find(_ssName) == ssMap.end()){
			iSound *s = SOUND_MANAGER_LOAD(_sName);
			if(!s){
				return NULL;
			}
			AmbientSS *ass = new AmbientSS(*s, _loop, _stream, _bufCount); 
			ass->setSSName(_ssName);
			std::cout << ass << std::endl;
			ssMap.insert(mapPair(_ssName, ass));
			return ass;
		}else{
			return NULL;
		}
	}
	
	// 
	//------------------------------------------
	iSoundSource* SimpleSSManager::getSoundSource(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end()){
			return mapIter->second;
		}else{
			return NULL;
		}
	}
	
	// 
	//------------------------------------------
	bool SimpleSSManager::removeSoundSource(const brString &_ssName){
		mapIter = ssMap.find(_ssName);
		if(mapIter != ssMap.end()){
			ssMap.erase(mapIter);
			return true;
		}else{
			return false;			
		}
	}
}



