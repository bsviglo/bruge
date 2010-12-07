#ifndef SRMANAGER_H_
#define SRMANAGER_H_

#include <map>
#include <list>
#include <iterator>
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>
#include "iSound.h"
#include "Utils/Data.h"
#include "Utils/br_String.h"
#include "Loader/br_ResourceManager.h"
#include "Engine/br_Exception.h"

using namespace brUGE;
using namespace brUGE::utils;

namespace SoundSys{
	class SRManager : public ibrResourceSubManager{
		friend class iSoundSource;
	public:

		//
		// 
		//------------------------------------------------------------
		enum SFE{
			NONE = -1,
			OGG  = 0,
			MP3	 = 1,
			WAV	 = 2
		};

		//
		// 
		//------------------------------------------------------------
		struct SoundData{

			// 
			//------------------------------------------
			SoundData(const brString &_sfn, bool _singleton): soundFileName(_sfn),
				singleton(_singleton),	
				data(NULL),
				dataSize(0),
				fileFormat(NONE){
				}
				bool 				singleton;
				Data 				*data;
				unsigned int 		dataSize;
				SFE					fileFormat;
				brString			soundFileName;
				typedef std::list<iSound*>  soundList;
				typedef std::list<iSound*>::iterator listIter;
				soundList  sl;
				listIter   li;
		};

		// 
		//------------------------------------------
		virtual ~SRManager();

		/**
		* register new sound in sound resources manager(SRManager)
		* @param
		* 		_soundName - sound name
		* @param
		* 		_soundFileName - name of sound file, from which retrieve data
		* @param
		* 		singleton - save binary data of file in PC memory 
		* 					or read data direct from this file
		* @return
		* 		true - if all OK, or false - when something wrong.
		*/
		bool registerSound( const brString &_soundName,
			const brString &_soundFileName,
			bool singleton = false);

		/**
		* unregister sound from sound resources manager(SRManager)
		* @param
		* 		_soundName - sound name
		* @return
		* 		true - if all OK, or  false - if something wrong.
		*/
		bool unregisterSound(const brString &_soundName);

		/**
		* retrieve sound interface from sound resources manager(SRManager)
		* @param
		* 		_soundName - sound name
		*/
		iSound* getSound(const brString &_soundName) const;

		/**
		* load all mapped resources from current directories and  archives
		* @return 
		* 		count of loaded files
		*/
		int loadResources(){return 0;};

		// 
		//------------------------------------------
		brResourceSubManagerType getId();

		// Функция производит всю загрузку ресурсов данного подменеджера
		// Note: запускается менеджером в отдельном потоке
		//------------------------------------------
		virtual BRbool load(const brString &_dir) throw(brException);

		// Производит полную выгрузку ресурсов данного менеджера
		//------------------------------------------
		virtual BRbool unload() throw(brException);

		// Возвращает статус загрузки или выгрузки ресурсов.
		// Если загрузки и выгрузки не происходит то возвращает 0.0
		//------------------------------------------
		virtual BRfloat getProgress();

		// 
		//------------------------------------------
		unsigned long getMemoryUsage(){
			return staticMemoryUsage;
		}

		// 
		//------------------------------------------
		unsigned long getMemoryUsage_kb(){
			return staticMemoryUsage/1024;
		}

		// 
		//------------------------------------------
		static brString getFileExt(const brString &_fileName);

		// 
		//------------------------------------------
		static bool caseInsCompare(const brString &left, const brString &right);

		// 
		//------------------------------------------
		static inline bool caseInsCharCompare(char _left, char _right){ 
#ifndef _WIN32
			return (std::toupper(_left) == std::toupper(_right));
#else
			return (toupper(_left) == toupper(_right));
#endif
		}

		// 
		//------------------------------------------
		SRManager();
	private:
		/**
		 * delete _sound data when sound source is delete or 
		 * sound no more need.
		 * This method can call only iSoundSource class and your's subclasses
		 */
		bool deleteSoundData(iSound *sound);

		mutable unsigned long staticMemoryUsage;
		typedef std::pair<brString, SoundData*>				SoundTableElement;				
		mutable std::list<SoundData*> 						soundDataList;
		mutable std::map<brString, SoundData*> 				soundTable;
		mutable std::list<iSound*>							soundList;
		mutable std::list<SoundData*>::iterator 			dataListIter;
		mutable std::list<iSound*>::iterator				soundListIter;
		mutable std::map<brString, SoundData*>::iterator	mapIter;
		mutable	float										progress;	
	};
}
#endif /*SRMANAGER_H_*/
/*
 	* 06/09/07	
*/
