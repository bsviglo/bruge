#include "Sound/iSound.h"
#include "Utils/br_String.h"
#include <iostream>

namespace SoundSys{
	// 
	//------------------------------------------
	iSound::iSound(const iSound &_sound){
		bitsPerSample = _sound.bitsPerSample;
		channelNumber = _sound.channelNumber;
		format		  = _sound.format;
		frequency	  = _sound.frequency;
		//PMC	 	  = new char[sizeof(_sound.PMC)];
		//*PMC		  = *_sound.PMC;	
	}
	
	// 
	//------------------------------------------
	iSound::iSound(const Data *_soundData) : bitsPerSample(0),
											  channelNumber(0),
											  format(0),
											  frequency(0),
											  //PMC(NULL),
											  //fileStream(NULL),
											  soundData(NULL){
		soundData = const_cast<Data*>(_soundData);		
	}
	
	// 
	//------------------------------------------
	iSound::iSound(const brString &_fileName) :	  bitsPerSample(0),
												  channelNumber(0),
												  format(0),
												  frequency(0),
												  //PMC(NULL),
												  //fileStream(NULL),
												  soundData(NULL){
		fileStream.open(_fileName.c_str(), std::ios::binary | std::ios::in);
		if(!fileStream.is_open()){
			std::cout << "File " << _fileName.c_str() << " not founded." << std::endl;
		}
	}
	
	// 
	//------------------------------------------
	iSound::~iSound(){
//		if(PMC)
//			delete PMC;
		if(soundData)
			delete soundData;
		if(fileStream && fileStream.is_open())
			fileStream.close();
	}
}

