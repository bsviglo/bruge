#ifndef SIMPLESSMANAGER_H_
#define SIMPLESSMANAGER_H_

#include "br_Types.h"
#include "iSSManager.h"
#include <string>
#include <map>

namespace SoundSys{

	//
	// 
	//------------------------------------------------------------
	class SimpleSSManager : public iSSManager{
	public:
		SimpleSSManager();
		virtual ~SimpleSSManager();
		/**
		 * Update sound system state, listener and each sound sources 
		 * (e.g. listener position, orientation; sound sources positions; swaping buffers,
		 *  when activate stream regime )
		 */ 
		virtual void update();
		/**
		 * Create new sound source with sound and others parameters
		 * @param
		 * 		loop - looping this sound source
		 * @param
		 * 		stream - using streaming sound reproduction
		 */  
		virtual iSoundSource* createSoundSource(const brString &_ssName, const brString &_sName,
										const brString &_ssType, bool _loop = false,
										bool _stream = false, int _bufCount = -1);
		/**
		 * Retrieve sound source by name
		 * @param 
		 * 		_ssName sound source name
		 */
		virtual iSoundSource* getSoundSource(const brString &_ssName);
		virtual bool removeSoundSource(const brString &_ssName);
		/**
		 * Clear sound scene, i.e. delete all sound sources from this scene
		 */
		virtual int clearSoundScene();
		virtual void playAll();
		virtual void stopAll();
		virtual void rewindAll();
		virtual void pauseAll();
		virtual void play(const brString &_ssName);
		virtual void pause(const brString &_ssName);
		virtual void stop(const brString &_ssName);
		virtual void rewind(const brString &_ssName);		

	private:
		typedef std::map<brString, iSoundSource*> soundSourceMap;
		typedef std::map<brString, iSoundSource*>::iterator mapIterator;
		typedef std::pair<brString, iSoundSource*> mapPair;	 
		soundSourceMap 	ssMap;
		mapIterator 	mapIter;
	};
}
#endif /*SIMPLESSMANAGER_H_*/
