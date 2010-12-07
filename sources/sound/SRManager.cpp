#include "Sound/SRManager.h"
#include "Sound/OggSound.h"
#include "Sound/WavSound.h"
#include "OSspecific/br_FileSystem.h"
#include "Console/br_Console.h"

using namespace brUGE;

namespace SoundSys{

	// 
	//------------------------------------------
	SRManager::SRManager(){
		staticMemoryUsage = 0;		
	}
	
	// 
	//------------------------------------------
	SRManager::~SRManager(){
//		if(!soundTable.empty()){
//			soundTable.erase(soundTable.begin(), soundTable.end());
//		}
		if(!soundDataList.empty()){
			for(dataListIter = soundDataList.begin();
						dataListIter != soundDataList.end(); ++dataListIter){
				delete (*dataListIter)->data;
			}
		}
		if(!soundList.empty()){
			for(soundListIter = soundList.begin(); 
							soundListIter != soundList.end(); ++soundListIter){
				delete *soundListIter;
			}
		}
	}
	
	// 
	//------------------------------------------
	bool SRManager::registerSound( const brString &_soundName,
								   const brString &_soundFileName,
								   bool _singleton){
		SoundData *sd = new SoundData(_soundFileName, _singleton);
		soundDataList.push_back(sd);
		mapIter = soundTable.find(_soundName);
		if(mapIter == soundTable.end()){
			//retrieve extension of file
			brString extension = SRManager::getFileExt(_soundFileName);
			if(SRManager::caseInsCompare(extension, "OGG")){
				sd->fileFormat = OGG;
			}else if(SRManager::caseInsCompare(extension, "WAV")){
				sd->fileFormat = WAV;
				//every time play file from main memory(RAM)
				sd->singleton = true;
			}else if(SRManager::caseInsCompare(extension, "MP3")){
				sd->fileFormat = MP3;
			}
			soundTable.insert(SoundTableElement(_soundName, sd));
			return true;
		}else{
			return false;
		}
	}
	
	// 
	//------------------------------------------
	bool SRManager::unregisterSound(const brString &_soundName){
		mapIter = soundTable.find(_soundName);
		if(mapIter == soundTable.end()){
			return false;
		}else{
			SoundData::soundList soundList = mapIter->second->sl;
			SoundData::listIter listIter = mapIter->second->li;
			for(listIter = soundList.begin();
						listIter != soundList.end(); ++listIter){
				delete *listIter;
			}
			soundTable.erase(mapIter);
			return true;
		}
	}
	
	// 
	//------------------------------------------
	iSound* SRManager::getSound(const brString &_soundName) const{
		mapIter = soundTable.find(_soundName);
		if(mapIter == soundTable.end()){
			return NULL;
		}else{
			//retrieve singleton state
			SoundData *sd = mapIter->second;
			bool sing = sd->singleton;
			
			iSound *is = NULL;
			if(sd->fileFormat == OGG){
				if(sing){
					if(!sd->data){
						sd->data = new Data(sd->soundFileName.c_str());
						sd->dataSize = sd->data->getLength();
						staticMemoryUsage += sd->dataSize;
					}
					is = new OggSound(new Data(sd->data->getPtr(0), sd->data->getLength()));
				}else{
					is = new OggSound(sd->soundFileName);					
				}
				soundList.push_back(is);
				sd->sl.push_back(is);
				return is;
			}else if(sd->fileFormat == WAV){
				sd->data = new Data(sd->soundFileName.c_str());
				sd->dataSize = sd->data->getLength();
				staticMemoryUsage += sd->dataSize;
				is = new WavSound(new Data(sd->data->getPtr(0), sd->data->getLength()));
				soundList.push_back(is);
				sd->sl.push_back(is);
				return is;
			}else if(sd->fileFormat == MP3){
				return NULL;
			}
			return NULL;
		}
	}
	
	// 
	//------------------------------------------
	brString SRManager::getFileExt(const brString &_fileName){
		size_t i = _fileName.rfind('.', _fileName.length());
		if(i != brString::npos){
			return (_fileName.substr(i+1, _fileName.length()-i));
		}
		return "";
	}
	
	// 
	//------------------------------------------
	bool SRManager::caseInsCompare(const brString &_left, const brString &_right){
		return (_left.size() == _right.size())
					&&(std::equal(_left.begin(),_left.end(),_right.begin(),caseInsCharCompare));
	}

	// 
	//------------------------------------------
	brResourceSubManagerType SRManager::getId(){
		return RSMT_SOUND;
	}

	// Функция производит всю загрузку ресурсов данного подменеджера
	// Note: запускается менеджером в отдельном потоке
	//------------------------------------------
	BRbool SRManager::load(const brString &_dir) throw(brException){
		progress = 0.0f;

		std::vector<brString> filesArray;

		if(!os::brFileSystem::getFilesListInDir(_dir, filesArray)){
			throw brException("directory " + _dir + " is incorrect.");
		}

		if(filesArray.empty()){
			WARNING_F("[Sound Manager]Directory %s is empty.", _dir.c_str());
			return false;
		}

		brString fullPath;
		for(size_t iter = 0; iter < filesArray.size(); ++iter){
			fullPath.clear();
			fullPath.append(_dir + "\\");
			fullPath.append(filesArray[iter].c_str());
			// Регистрация ресурса
			registerSound(filesArray[iter], fullPath, true);
			// Фиктивный вызов метода, для того что бы быть уверенным,
			// что ресурс загрузился с диска в RAM именно сейчас
			getSound(filesArray[iter]);
			ConPrint(fullPath.c_str());
			progress = ((iter+1) * 100.0f)/ filesArray.size();
		}
		LOG_F("[Sound Manager]Total loaded sounds number: %d", soundTable.size());
		LOG_F("[Sound Manager]Total loaded sounds size: %d Kb", getMemoryUsage_kb());
		return true;
	}

	// Производит полную выгрузку ресурсов данного менеджера
	//------------------------------------------
	BRbool SRManager::unload() throw(brException){
		//TODO: корректное удаление из памяти
		return true;
	}

	// Возвращает статус загрузки или выгрузки ресурсов.
	// Если загрузки и выгрузки не происходит то возвращает 0.0
	//------------------------------------------
	BRfloat SRManager::getProgress(){
		return progress;
	}
}
