#ifndef iSOUND_H_
#define iSOUND_H_

#include <fstream>
#include "Utils/Data.h"


namespace brUGE{
	namespace utils{
		class brString;
	}
}

using namespace brUGE::utils;

namespace SoundSys{
	//forward declaration
	class SRManager;
	
	/**
	 * Contain specific by format of sound file PMC data
	 * and give possibility for retrieving and manipulating this data
	 */ 
	class iSound{
		// make ISRManager friend
		friend class SRManager;
	public:
		iSound(const iSound &_sound);
		iSound(const Data *_soundData);
		iSound(const brString &_file);
		virtual ~iSound();
		/**
		 * Read next PMC data block(unpacked audio data) from given file
		 * and save it into first parameter
		 * @param 
		 * 		dataBuffer - A pointer to an output buffer.
		 * 	 	The decoded output is inserted into this buffer.
		 * @param
		 * 		size - PMC block size
		 */  
		virtual int	read(char *_dataBuffer, int _size = -1)=0;
		/**
		 * Retrieve position in seconds in sound
		 */
		virtual bool seek(float _time)=0;
		/**
		 * Get size of unpacked audio data
		 */
		virtual size_t getSize()=0;

		// 
		//------------------------------------------
		int getFormat(){
			return format;
		}

		// 
		//------------------------------------------
		int getChannelNumber(){
			return channelNumber;
		}

		// 
		//------------------------------------------
		int getFrequency(){
			return frequency;
		}

		// 
		//------------------------------------------
		int getBitsPerSample(){
			return bitsPerSample;
		}

		// 
		//------------------------------------------
		bool hasError(){
			return hasErr;
		}
	protected:
		std::ifstream fileStream;
		Data*		   soundData;	
		int channelNumber;
		int	format;
		int frequency;
		int bitsPerSample;
		bool hasErr;
	};
}
#endif /*iSOUND_H_*/
/*
 	* 06/09/07	
*/
