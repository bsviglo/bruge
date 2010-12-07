#include "Sound/OggSound.h"
#include "Utils/Data.h"

namespace SoundSys{

	// 
	//------------------------------------------
	OggSound::OggSound(const brString &_fileName) : iSound(_fileName){
		//create main Ogg file structure
		oVF = new OggVorbis_File();
		//structure with callback functions
		ov_callbacks cb;
		cb.close_func = closeOgg;
		cb.read_func  =	readOgg;
		cb.seek_func  =	seekOgg;
		cb.tell_func  =	tellOgg;
		
		//initializing ogg file with facilities of vorbis libraries
		ov_open_callbacks(&fileStream, oVF, NULL, -1, cb);
		info 	= ov_info(oVF, -1);
		comment	= ov_comment(oVF, -1);
		channelNumber = info->channels;
		frequency	  = info->rate;
		bitsPerSample = 16;
	}
	
	// 
	//------------------------------------------
	OggSound::OggSound(const Data *_data) : iSound(_data){
		//create main Ogg file structure
		oVF = new OggVorbis_File();
		//structure with callback functions
		ov_callbacks cb;
		cb.close_func = fastCloseOgg;
		cb.read_func  =	fastReadOgg;
		cb.seek_func  =	fastSeekOgg;
		cb.tell_func  =	fastTellOgg;
				
		//initializing ogg file with facilities of vorbis libraries
		ov_open_callbacks(this, oVF, NULL, -1, cb);
		info 	= ov_info(oVF, -1);
		comment	= ov_comment(oVF, -1);
		channelNumber = info->channels;
		frequency	  = info->rate;
		bitsPerSample = 16;
	}
	
	// 
	//------------------------------------------
	OggSound::~OggSound(){
		if(!oVF){
			ov_clear(oVF);
			delete oVF;
		}
	}
	
	// 
	//------------------------------------------
	size_t OggSound::getSize(){
		double	totalTime = ov_time_total ( const_cast <OggVorbis_File *> ( oVF ), -1 );
		return (size_t)(totalTime * channelNumber * frequency * 2);
	}
	
	// 
	//------------------------------------------
	int OggSound::read(char* _dataBuffer, int _size){
		if(_size<0)
			_size = getSize();

		int	curSection, res;
		int	bytesRead = 0;

		while ( bytesRead < _size ){
			res = ov_read ( oVF, (char *)(_dataBuffer + bytesRead), _size - bytesRead, 0, 2, 1, &curSection );
			if ( res <= 0 )
				break;
			bytesRead += res;
		}
		return bytesRead;
	}
	
	// 
	//------------------------------------------
	bool OggSound::seek(float _time){
		ov_time_seek(oVF, _time);
		return true;
	}
	
	// 
	//------------------------------------------
	size_t OggSound::readOgg(void *_ptr, size_t _size, size_t _nmmemb, void *_dataSource){
		std::istream *file = reinterpret_cast<std::istream*>(_dataSource);
		file->read((char*)_ptr, _size*_nmmemb);
		return file->gcount();
	}
	
	// 
	//------------------------------------------
	int OggSound::seekOgg(void *_dataSource, ogg_int64_t _offset, int _whence){
		std::istream *file = reinterpret_cast<std::istream*>(_dataSource);
		std::ios_base::seekdir dir;
		file->clear();
		switch(_whence){
		case SEEK_SET:
			dir = std::ios::beg;
			break;
		case SEEK_CUR:
			dir = std::ios::cur;
			break;
		case SEEK_END:
			dir = std::ios::end;
			break;
		default:
			return -1;
		}
		file->seekg((std::streamoff)_offset, dir);
		return (file->fail() ? -1 : 0);
	}
	
	// 
	//------------------------------------------
	long OggSound::tellOgg(void *_dataSource){
		std::istream *file = reinterpret_cast<std::istream*>(_dataSource);
		return file->tellg();
	}
	
	// 
	//------------------------------------------
	int OggSound::closeOgg(void *_dataSource){
		return 0;
	}
	
	
	// 
	//------------------------------------------
	size_t OggSound::fastReadOgg(void *_ptr, size_t _size, size_t _nmmemb, void *_dataSource){
		OggSound *oggs = reinterpret_cast<OggSound*>(_dataSource);
		Data *data = oggs->soundData;
		if (data->getPos() >= data->getLength())
 			return 0;
  		return (size_t)data->getBytes(_ptr, _size*_nmmemb);
	}
	
	// 
	//------------------------------------------
	int OggSound::fastSeekOgg(void *_dataSource, ogg_int64_t _offset, int _whence){
		OggSound *oggs = reinterpret_cast<OggSound*>(_dataSource);
		Data *data = oggs->soundData;
				
		if (_whence == SEEK_SET)
  			data->seekAbs((int)_offset);
		else
		if (_whence == SEEK_CUR)
  			data->seekCur((int)_offset);
		else 
		if (_whence == SEEK_END)
 			data->seekAbs(data->getLength() + (int)_offset);

		return data->getPos();
	}
	 
	// 
	//------------------------------------------
	long OggSound::fastTellOgg(void *_dataSource){
		OggSound *oggs = reinterpret_cast<OggSound*>(_dataSource);
		Data *data = oggs->soundData;
		return data->getPos();
	}

	// 
	//------------------------------------------
	int OggSound::fastCloseOgg(void *_dataSource){
		return 0;
	}
}
