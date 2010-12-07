#ifndef AMBIENTSS_H_
#define AMBIENTSS_H_

#include "iSoundSource.h"

namespace SoundSys{
	//forward declaation
	class iSound;
	class iSSManager;

	class AmbientSS : public iSoundSource{
		friend class iSSManager;
	public:
		// 
		//------------------------------------------
		AmbientSS(iSound &_soundData,
				  bool _loop = false,
				  bool _stream = false,
				  int _bufCount = -1) : iSoundSource(_soundData,
						  							 _loop,
						  							 _stream,
						  							 _bufCount){
		}
	public:
		// 
		//------------------------------------------
		virtual ~AmbientSS(){};
	};

}

#endif /*AMBIENTSS_H_*/
