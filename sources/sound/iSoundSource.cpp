#include "Sound/iSoundSource.h"
#include "Sound/iSound.h"
#include <queue>	
#include <iostream>
#ifdef _WIN32
	#include    "AL/al.h"
	#include	"AL/alc.h"
	//#include 	<alut.h>
#else
	#include	<AL/al.h>
	#include	<AL/alc.h>
	#include 	<AL/alut.h> 
#endif

namespace SoundSys{

	// 
	//------------------------------------------
	iSoundSource::iSoundSource( iSound &_soundData,
					 			bool _loop,
					 			bool _stream,
					 			int _bufCount):soundData(&_soundData),
					 						  looping(_loop),
					 						  streaming(_stream),
					 						  pitch(1.0f),
					 						  gain(1.0f),
					 						  source(-1),
					 						  buffers(NULL),
					 						  bufSize(BUFFER_SIZE),
					 						  bufCount(_bufCount),
					 						  dataFormat(0){
		velocity[0]=0.0f;
		velocity[1]=0.0f;
		velocity[2]=0.0f;
		
		position[0]=0.0f;
		position[1]=0.0f;
		position[2]=-3.0f;
		
		alGenSources(1, &source);
		alSourcefv(source, AL_POSITION, position);
		alSourcefv(source, AL_VELOCITY, velocity);
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, gain);
		if(streaming)
			alSourcei(source, AL_LOOPING, false);
								
		int size = 0;
		dataFormat = iSoundSource::soundFormat(soundData->getChannelNumber(), soundData->getBitsPerSample());
		if(dataFormat == AL_FORMAT_STEREO16){
			std::cout << "Format : " << "AL_FORMAT_STEREO16" << std::endl;
		}
		frequency = soundData->getFrequency();
		//if buffer number or size of PMC data of file less
		//than BUFFER_SIZE then off streaming playing 
		if((bufCount <= 1) || (bufSize > (int)soundData->getSize())){
			//std::cout << " ___HERE__ "<< std::endl;
			//std::cout << "bufCount = " << bufCount << std::endl;
			//std::cout << "bufSize = " << bufSize << std::endl;
			//std::cout << "soundSize = " << soundData->getSize() << std::endl;
			//set streaming flag in off
			streaming = false;
			buffers = new ALuint;
			alGenBuffers(1, buffers);
			//initialize temp buffers
			tempBuf = new char[soundData->getSize()];
			//read data into temp buffer
			size = soundData->read(tempBuf);
			alBufferData(buffers[0], dataFormat, tempBuf, size, frequency);
			alSourcei(source, AL_BUFFER, buffers[0]);
			//check errors
					ALenum error;
					if((error = alGetError()) != AL_NO_ERROR)
						std::cout << "Error: " << error << std::endl;
			delete tempBuf;
		}else{
			buffers = new ALuint[bufCount];
			alGenBuffers(bufCount, buffers);
			tempBuf = new char[bufSize];
			for(int i=0; i<bufCount; ++i){
				std::cout << "Buffer: " << buffers[i] << std::endl;
				buffersQueue.push(buffers[i]);
			}
			printError();
		}
	}
	
	// 
	//------------------------------------------
	void iSoundSource::update(){
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, gain);
		alSourcefv(source, AL_POSITION, position);
		alSourcefv(source, AL_VELOCITY, velocity);

		std::cout << "state " << getState() << std::endl;
		
		ALint processed = 0;
		int size = 0;	
		ALuint tempVal = 0;
		if(streaming){
			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
			if(processed == 0){
				return;
			}else{
				//delete from queue all processed buffers
				for(int i=0; i<processed; ++i){
					tempVal = buffersQueue.front();
					std::cout << " UPDATE BUFFER  ==> "<< tempVal << std::endl;
					std::cout << "-----------------------------------------" << std::endl;
					alSourceUnqueueBuffers(source, 1, &tempVal);
					buffersQueue.pop();
					//read new PMC data into a processed buffers
					size = soundData->read(tempBuf, BUFFER_SIZE);
					if((size > 0) || (size == 0 && looping)){
						alBufferData(tempVal, dataFormat, tempBuf, size, frequency);
						alSourceQueueBuffers(source, 1, &tempVal);
						buffersQueue.push(tempVal);
						if(size < bufSize && looping)
							soundData->seek(0.0f);
					}else{
						int queued = -1;
						alGetSourcei (source, AL_BUFFERS_QUEUED, &queued);
						if (queued == 0)
							soundData->seek(0.0f);
					}
				}
			}
		}
		printError();
		return;
	}
	
	// 
	//------------------------------------------
	void iSoundSource::play(){
		int state = 0;
		int size = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING)
			return;
		if(state != AL_PAUSED && streaming){
			for(int i=0; i<bufCount; ++i){
				size = soundData->read(tempBuf, BUFFER_SIZE);
				alBufferData(buffers[i], dataFormat, tempBuf, size, frequency);
			}
			alSourceQueueBuffers(source, bufCount, buffers);
		}
		alSourcePlay(source);
		printError();
		return;
	}
	
	// 
	//------------------------------------------
	void iSoundSource::pause(){
		alSourcePause(source);		
		printError();
	}
	
	// 
	//------------------------------------------
	void iSoundSource::stop(){
		alSourceStop(source);
		soundData->seek(0.0f);
		if(streaming){
			ALint queued;
			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
			if (queued > 0){
				ALuint tempVal = 0;
				for(int i=0; i<bufCount; ++i){
					tempVal = buffersQueue.front();
					buffersQueue.pop();
					buffersQueue.push(tempVal);
					alSourceUnqueueBuffers(source, 1, &tempVal);
				}
			}
		}
		printError();
		return;
	}
	
	// 
	//------------------------------------------
	void iSoundSource::rewind(){
		if(streaming){
			alSourceStop(source);
			soundData->seek(0.0f);
			ALint queued;
			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
			if (queued > 0){
				ALuint tempVal = 0;
				for(int i=0; i<bufCount; ++i){
					tempVal = buffersQueue.front();
					buffersQueue.pop();
					buffersQueue.push(tempVal);
					alSourceUnqueueBuffers(source, 1, &tempVal);
				}
			}
			play();
		}else{
			alSourceRewind(source);
		}
		printError();
	}
	
	// 
	//------------------------------------------
	iSoundSource::~iSoundSource(){
		if(buffers){
			delete buffers;
			buffers = NULL;
		}
		if(tempBuf)
			delete [] tempBuf;
	}

	// 
	//------------------------------------------
	ALuint	iSoundSource :: soundFormat(int _channelNumber, int _bitsPerSample){
		if (_channelNumber == 1)
			return _bitsPerSample == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;

		if (_channelNumber == 2)
			return _bitsPerSample == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
			
		if (_channelNumber == 4)
			if (_bitsPerSample == 16)
				return alGetEnumValue ("AL_FORMAT_QUAD16");
			else
				return alGetEnumValue ("AL_FORMAT_QUAD8");
				
		if (_channelNumber == 6)
			if (_bitsPerSample == 16)
				return alGetEnumValue("AL_FORMAT_51CHN16");
			else
				return alGetEnumValue("AL_FORMAT_51CHN8");
		return 0;
	}

	// 
	//------------------------------------------
	void iSoundSource::printError(){
		ALenum error = alGetError();
		if(error != AL_NO_ERROR)
			std::cout << "Error : "<< error << std::endl; 
	}

	// 
	//------------------------------------------
	iSoundSource::State iSoundSource::getState(){
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);

		switch(state)
		{
		case INITIAL:
			return INITIAL;
		case PLAYING:
			return PLAYING;
		case PAUSED:
			return PAUSED;
		case STOPPED:
			return STOPPED;
		default:
			return UNKNOWN;
		}
	}
}

