#ifndef _WAVSOUND_H_
#define _WAVSOUND_H_

#include "br_Types.h"
#include "iSound.h"

//forward declaration
class Data;

namespace brUGE{
	namespace utils{
		class brString;
	}
}

using namespace brUGE::utils;

namespace SoundSys{

	//
	// 
	//------------------------------------------------------------
    class WavSound : public iSound{
	public:

		// 
		//------------------------------------------
		WavSound(const brString &_fileName);

		// 
		//------------------------------------------
		WavSound(const Data *_data);

		// 
		//------------------------------------------
		~WavSound();

		// 
		//------------------------------------------
		virtual int read(char *_buf, int _size = -1);

		// 
		//------------------------------------------
		virtual bool seek(float _time);

		// 
		//------------------------------------------
		virtual size_t getSize();

		//
		// 
		//------------------------------------------------------------
		enum
		{
			RIFF = 0x46464952,
			WAVE = 0x45564157,
			FMT  = 0x20746D66,
			DATA = 0x61746164,
		};
	private:
		int	dataOffs;
		int	dataLength;
	};
}/*namespace brSS*/
#endif /*_WAVSOUND_H_*/
