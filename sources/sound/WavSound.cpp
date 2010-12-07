#include "Sound/WavSound.h"
#include "Utils/br_String.h"

namespace SoundSys{

#pragma pack (push, 1)

	struct WavFmt
	{
		unsigned short encoding;
		unsigned short channels;
		unsigned int   frequency;
		unsigned int   byterate;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
	};

#pragma pack (pop)

	// 
	//------------------------------------------
	WavSound::WavSound(const brString &_fileName):iSound(_fileName){
		hasErr = true;
		
	}

	// 
	//------------------------------------------
	WavSound::WavSound(const Data *_data) : iSound(_data){
		hasErr = false;
		long magic  = soundData->getLong();
		long length = soundData->getLong();
		long magic2 = soundData->getLong();

		if(magic != RIFF || magic2 != WAVE){
			hasErr = true;
			return;
		}

		for(; soundData->getPos() < soundData->getLength();){
			magic  = soundData->getLong();
			length = soundData->getLong();

			if(magic == FMT){
				WavFmt	format;
				soundData->getBytes(&format, sizeof(format));
				if (format.encoding != 1){
					hasErr = true;
					return;
				}
				channelNumber = format.channels;
				frequency     = format.frequency;
				bitsPerSample = format.bitsPerSample;
			}
			else{
				if(magic == DATA){
					dataOffs   = soundData->getPos();
					dataLength = length;
					break;
				}else{
					soundData->seekCur(length);
				}
			}
		}
		if(channelNumber < 1 || frequency == 0 || dataOffs == 0 || dataLength == 0){
			hasErr = true;
		}
	}

	// 
	//------------------------------------------
	WavSound::~WavSound(){

	}

	// 
	//------------------------------------------
	int	WavSound::read(char *_buf, int _size){
		int	bytesLeft = dataLength - (soundData->getPos() - dataOffs);

		if(_size < 0 || _size > bytesLeft)
			_size = bytesLeft;

		return soundData->getBytes(_buf, _size);
	}

	// 
	//------------------------------------------
	size_t	WavSound::getSize(){
		return dataLength;
	}

	// 
	//------------------------------------------
	bool WavSound::seek(float _time){
		soundData->seekAbs(dataOffs + (int)(_time * channelNumber * frequency * bitsPerSample/2));	// XXX - 16 bit ???
		return true;
	}

}/*namespace brSS*/
