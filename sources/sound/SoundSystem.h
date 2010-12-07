#ifndef SOUNDSYSTEM_H_
#define SOUNDSYSTEM_H_

#include "br_Types.h"
#include "iSSManager.h"
#include <string>
#include "AL/al.h"
#include "AL/alc.h"

using namespace brUGE::utils;

namespace SoundSys{

	//
	// Объект отвечающий за инициализацию менеджера источников звука и инициализацию
	// звукового устройства. В данном случае посредством библиотеки OpenAL
	//------------------------------------------------------------
	class SoundSystem{
	public:
		// 
		//------------------------------------------
		SoundSystem();

		// 
		//------------------------------------------
		virtual ~SoundSystem();
		
		// Производиться создание и настройка OpenAL устройства
		//------------------------------------------
		bool initialize();

		// Создает менеджер источников звука опеределенного типа 
		//------------------------------------------
		iSSManager* createSSManager(const brString &_ssmType);

		// Обновление состояния звуковой системы
		//------------------------------------------
		void update();
		
		// Устанавливает ориентация слушателя. Т.е. игрока, который может быть только один.
		//------------------------------------------
		void setOrientation(const float *_at, const float *_up);

		// Устанавливает скорость слушателя.
		// Note: Используется только для учета эффекта Доплера
		//------------------------------------------
		void setVelosity(const float *_velocity);

		// Устанавливает позицию слушателя
		//------------------------------------------
		void setPosition(const float *_position);

		// Устанавливает битрейт
		//------------------------------------------
		void setPitch(float _pitch);

		// Устанавливает громкость
		//------------------------------------------
		void setVolume(float _volume);
		
	private:
		float velocity[3];
		float position[3];
		float orientation[6];
		float volume;
		float pitch;
		iSSManager	*ssManager;
		ALCdevice 	*device;
		ALCcontext	*context; 
	};
}

#endif /*SOUNDSYSTEM_H_*/
