#include "Sound/SoundSystem.h"
#include "Sound/SimpleSSManager.h"
#include <iostream>

namespace SoundSys{

	// 
	//------------------------------------------
	SoundSystem::SoundSystem(){
		
	}
	
	// 
	//------------------------------------------
	SoundSystem::~SoundSystem(){
		if(ssManager)
			delete ssManager;
		// Exit
		context = alcGetCurrentContext();
		device = alcGetContextsDevice(context);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
	
	// 
	//------------------------------------------
	void SoundSystem::setOrientation(const float *_at, const float *_up){
		orientation[0] = _at[0];
		orientation[1] = _at[1];
		orientation[2] = _at[2];
		orientation[3] = _up[0];
		orientation[4] = _up[1];
		orientation[5] = _up[2];
		alListenerfv(AL_ORIENTATION, orientation);
	}
	
	// 
	//------------------------------------------
	bool SoundSystem::initialize(){
		device = alcOpenDevice(NULL);
		if(device){
			context = alcCreateContext(device, NULL);
			alcMakeContextCurrent(context);
		}
		ALenum error;
		if((error = alGetError()) != AL_NO_ERROR){
			std::cout << "Error: " << error << std::endl;
			return false;
		}
		if((error = alcGetError(device)) != AL_NO_ERROR){
			std::cout << "Context error: " << error << std::endl;
			return false;
		}
		return true;
	}
	
	// 
	//------------------------------------------
	void SoundSystem::update(){
		ssManager->update();		
	}
	
	// 
	//------------------------------------------
	iSSManager* SoundSystem::createSSManager(const brString &_ssmType){
		ssManager = new SimpleSSManager();
		return ssManager;
	}

	// 
	//------------------------------------------
	void SoundSystem::setVelosity(const float *_velocity){
		velocity[0] = _velocity[0];
		velocity[1] = _velocity[1];
		velocity[2] = _velocity[2];
		alListenerfv(AL_VELOCITY, velocity);
	}

	// 
	//------------------------------------------
	void SoundSystem::setPosition(const float *_position){
		position[0] = _position[0];
		position[1] = _position[1];
		position[2] = _position[2];
		alListenerfv(AL_POSITION, position);
	}

	// 
	//------------------------------------------
	void SoundSystem::setPitch(float _pitch){
		pitch = _pitch;
		alListenerf(AL_PITCH, pitch);
	}

	// 
	//------------------------------------------
	void SoundSystem::setVolume(float _volume){
		volume = _volume;
		alListenerf(AL_GAIN, volume);
	}
}
