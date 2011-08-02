#pragma once

#include "prerequisites.hpp"
#include "Camera.h"

namespace brUGE
{

	//----------------------------------------------------------------------------------------------
	class CursorCamera : public Camera
	{
	public:
		CursorCamera(const render::Projection& projection);
		virtual ~CursorCamera();

		virtual void update(bool updateInput, float dt);
		virtual void updateMouse(float dx, float dy, float dz);

		void source(const mat4f* src)	{ m_source = src; }
		void target(const mat4f* trg)	{ m_target = trg; }

	private:
		const mat4f* m_source;
		const mat4f* m_target;
	};

} //-- brUGE