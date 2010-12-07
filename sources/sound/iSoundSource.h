#ifndef ISOUNDSOURCE_H_
#define ISOUNDSOURCE_H_

#include	"br_Types.h"
#include	"Utils/br_String.h"
#include 	<queue>
#include 	<iostream>

using namespace brUGE::utils;

namespace SoundSys{

	//forward declaration
	class iSound;
	class iSSManager;
	
	//
	// »нтерфейс к источнику звука. 
	//------------------------------------------------------------
	class iSoundSource{
		friend class iSSManager;
	public:

		// 
		//------------------------------------------
		iSoundSource(iSound &_soundData,
					 bool _loop = false,
					 bool _stream = false,
					 int _bufCount = -1);	
	public:

		//
		// 
		//------------------------------------------------------------
		enum State{
			UNKNOWN = 0x0000,
			INITIAL = 0x1011,
			PLAYING = 0x1012,
			PAUSED	= 0x1013,
			STOPPED = 0x1014
		};
	
		// 
		//------------------------------------------
		virtual ~iSoundSource();

		// 
		//------------------------------------------
		void update();

		// 
		//------------------------------------------
		void play();

		// 
		//------------------------------------------
		void pause();

		// 
		//------------------------------------------
		void stop();

		// ѕовторить сначала
		//------------------------------------------
		void rewind();

		// 
		//------------------------------------------
		void setPosition(const float *_position){
			position[0] = _position[0];
			position[1] = _position[1];
			position[2] = _position[2];
		}

		// 
		//------------------------------------------
		void setVelosity(const float *_velocity){
			velocity[0] = _velocity[0];
			velocity[1] = _velocity[1];
			velocity[2] = _velocity[2];
		}

		// 
		//------------------------------------------
		void setPitch(float _pitch){
			pitch = _pitch;
		}

		// 
		//------------------------------------------
		void setGain(float _gain){
			gain = _gain;
		}

		// 
		//------------------------------------------
		int getBufSize(){
			return bufSize; 
		}

		// 
		//------------------------------------------
		void setBufSize(int _bufSize){
			bufSize = _bufSize;			
		}

		// 
		//------------------------------------------
		unsigned getSourceId(){
			return source;
		}
		
		// 
		//------------------------------------------
		const brString &getSSName(){
			return ssName;
		}
		
		// 
		//------------------------------------------
		void setSSName(const brString &_ssName){
			ssName = _ssName;			
		}
		
		// 
		//------------------------------------------
		void printError();

		// ¬озвращает текущее состо€ние источника звука, возможные варианты перечисленны
		// в перечислении State
		//------------------------------------------
		State getState();

		// 
		//------------------------------------------
		static unsigned soundFormat(int _channelNumber, int _bitsPerSample);
	private:
		std::queue<unsigned> buffersQueue;
		bool 		looping;
		bool 		streaming;
		float 		pitch;
		float 		gain;
		float 		position[3];
		float 		velocity[3];
		unsigned	source;
		unsigned	*buffers;
		char 		*tempBuf;
		int 		bufCount;
		int 		bufSize;
		int			frequency;
		unsigned	dataFormat;
		iSound		*soundData;
		brString	ssName;
		enum
		{
			BUFFER_SIZE = 88200
		};
	};
}
#endif /*ISOUNDSOURCE_H_*/
/*
 	* 06/09/07	
*/
