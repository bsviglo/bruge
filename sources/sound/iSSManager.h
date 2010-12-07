#ifndef ISSMANAGER_H_
#define ISSMANAGER_H_

#include "br_Types.h"
#include "iSoundSource.h"

namespace brUGE{
	namespace utils{
		class brString;
	}
}

using namespace brUGE::utils;

namespace SoundSys{
	//forward declaration
	class iSound;
	/**
	 * Interface for storing and handling sound sources
	 */
	class iSSManager{
	public:
		iSSManager();
		virtual ~iSSManager();
		/**
		 * Update sound system state, listener and each sound sources 
		 * (e.g. listener position, orientation; sound sources positions; swaping buffers,
		 *  when activate stream regime )
		 */ 
		virtual void update()=0;
		/**
		 * Create new sound source with sound and others parameters
		 * @param
		 * 		loop - looping this sound source
		 * @param
		 * 		stream - using streaming sound reproduction
		 */  
		virtual iSoundSource* createSoundSource(const brString &_ssName, const brString &_sName,
							const brString &_ssType, bool _loop = false,
							bool _stream = false, int _bufCount = -1)=0;
		/**
		 * Retrieve sound source by name
		 * @param 
		 * 		_ssName sound source name
		 */
		virtual iSoundSource* getSoundSource(const brString &_ssName)=0;
		virtual bool removeSoundSource(const brString &_ssName)=0;
		/**
		 * Clear sound scene, i.e. delete all sound sources from this scene
		 */
		virtual int clearSoundScene()=0;
		virtual void playAll()=0;
		virtual void stopAll()=0;
		virtual void rewindAll()=0;
		virtual void pauseAll()=0;
		virtual void play(const brString &_ssName)=0;
		virtual void pause(const brString &_ssName)=0;
		virtual void stop(const brString &_ssName)=0;
		virtual void rewind(const brString &_ssName)=0;
		/**
		 * Sets for all sound sources gain value
		 */ 
		void setGain(float _gain){
			gain = _gain;
		}
		/**
		 * Sets for all sound sources pitch value
		 */
		void setPitch(float _pitch){
			pitch = _pitch;
		}
		/**
		 * Sets for all sound sources looping 
		 */
		void setSoundLooping(bool _looping){
			soundLooping = _looping;
		}
		/**
		 * Sets for all sound sources streaming mode
		 */
		void setSoundSteaming(bool _streaming){
			soundStreaming = _streaming;
		}
	private:
		float 			gain;
		float 			pitch;
		float 			velocity;
		bool  			soundLooping;
		bool  			soundStreaming;
	};
}
#endif /*ISSMANAGER_H_*/
/*
 	* 06/09/07	
*/
