#pragma once

#include "control/input_listener.h"
#include "console/Console.h"
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

		virtual bool handleMouseClick	(const MouseEvent& me) = 0;
		virtual bool handleMouseMove	(const MouseAxisEvent& mae) = 0;
		virtual bool handleKeyboardEvent(const KeyboardEvent& ke) = 0;

	protected:
		int demoDesc()
		{
			ConPrint(m_descString);
			return 0;
		}
		std::string m_descString;
	};

} // brUGE
