#pragma once

#include "prerequisites.hpp"
#include "engine/IComponent.hpp"
#include "engine/ISystem.hpp"
#include "SDL/SDL_events.h"

namespace brUGE
{
namespace input
{

	//----------------------------------------------------------------------------------------------
	class InputComponent : public IComponent
	{

	};

	//----------------------------------------------------------------------------------------------
	class InputSystem : public ISystem
	{
	public:

		class World : public ISystem::IWorld
		{
		};

		class Context : public ISystem::IContext
		{
		};

	public:
		InputSystem();
		virtual ~InputSystem() override;


		virtual bool init() override;
		virtual void update(IWorld* world) override;
		virtual void process(IContext* context) override;
		
		//-- called once a frame by Engine
		void	handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		void	handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		void	handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		void	handleKeyboardEvent(const SDL_KeyboardEvent& e);
		void	handleTextInputEvent(const SDL_TextInputEvent& e);

	private:
	};
}
}