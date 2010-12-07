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
	// ������ ���������� �� ������������� ��������� ���������� ����� � �������������
	// ��������� ����������. � ������ ������ ����������� ���������� OpenAL
	//------------------------------------------------------------
	class SoundSystem{
	public:
		// 
		//------------------------------------------
		SoundSystem();

		// 
		//------------------------------------------
		virtual ~SoundSystem();
		
		// ������������� �������� � ��������� OpenAL ����������
		//------------------------------------------
		bool initialize();

		// ������� �������� ���������� ����� �������������� ���� 
		//------------------------------------------
		iSSManager* createSSManager(const brString &_ssmType);

		// ���������� ��������� �������� �������
		//------------------------------------------
		void update();
		
		// ������������� ���������� ���������. �.�. ������, ������� ����� ���� ������ ����.
		//------------------------------------------
		void setOrientation(const float *_at, const float *_up);

		// ������������� �������� ���������.
		// Note: ������������ ������ ��� ����� ������� �������
		//------------------------------------------
		void setVelosity(const float *_velocity);

		// ������������� ������� ���������
		//------------------------------------------
		void setPosition(const float *_position);

		// ������������� �������
		//------------------------------------------
		void setPitch(float _pitch);

		// ������������� ���������
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
