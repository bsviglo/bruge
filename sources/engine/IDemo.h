#pragma once

#include "SDL/SDL_events.h"
#include <string>

namespace brUGE
{
	
	//
	//----------------------------------------------------------------------------------------------
	class IDemo
	{
	public:
		IDemo()
		{
			m_descString = "IDemo description is empty.";
			REGISTER_CONSOLE_METHOD("d_desc", demoDesc, IDemo);
		}

		virtual ~IDemo() {}

		virtual bool init() = 0;
		virtual void shutdown() = 0;
		virtual void update(float dt) = 0;
		virtual void render(float dt) = 0;

		virtual bool handleMouseButtonEvent(const SDL_MouseButtonEvent& e) = 0;
		virtual bool handleMouseMotionEvent(const SDL_MouseMotionEvent& e) = 0;
		virtual bool handleMouseWheelEvent(const SDL_MouseWheelEvent& e) = 0;
		virtual bool handleKeyboardEvent(const SDL_KeyboardEvent& e) = 0;

	protected:
		int demoDesc()
		{
			ConPrint(m_descString);
			return 0;
		}
		std::string m_descString;
	};

} // brUGE
