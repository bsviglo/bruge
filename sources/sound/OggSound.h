#ifndef OGGSOUND_
#define OGGSOUND_

#include "br_Types.h"
#include "iSound.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

//forward declaration
	class Data;	
namespace SoundSys{

	//
	// 
	//------------------------------------------------------------
	class OggSound: public iSound{
	public:
		// 
		//------------------------------------------
		OggSound(const brString &_fileName);

		// 
		//------------------------------------------
		OggSound(const Data *_data);

		// 
		//------------------------------------------
		virtual ~OggSound();
		
		// 
		//------------------------------------------
		virtual int	read(char *_dataBuffer, int _size = -1);

		// 
		//------------------------------------------
		virtual bool seek(float _time);

		// 
		//------------------------------------------
		virtual size_t getSize();
	private:
		OggVorbis_File 	*oVF;
		vorbis_comment	*comment;
		vorbis_info		*info;
	protected:
		// 
		//------------------------------------------
		static int 	  seekOgg(void *_dataSource, ogg_int64_t _offset, int _whence);

		// 
		//------------------------------------------
		static int    closeOgg(void *_dataSource);

		// 
		//------------------------------------------
		static long   tellOgg(void *_dataSource);

		// 
		//------------------------------------------
		static size_t readOgg(void *_ptr, size_t _size, size_t _nmemb, void *_dataSource);
		

		// 
		//------------------------------------------
		static int    fastSeekOgg(void *_dataSource, ogg_int64_t _offset, int _whence);

		// 
		//------------------------------------------
		static int    fastCloseOgg(void *_dataSource);

		// 
		//------------------------------------------
		static long   fastTellOgg(void *_dataSource);

		// 
		//------------------------------------------
		static size_t fastReadOgg(void *_ptr, size_t _size, size_t _nmemb, void *_dataSource);
	};
}/*namespace brSS*/

#endif /*OGGSOUND_*/
/**
 	* 08/09/07 
 */

