#pragma once

#include "prerequisites.hpp"
#include "imgui/imgui.h"
#include "SDL/SDL_events.h"

namespace brUGE
{
namespace ui
{

	//-- Implements imgui interface.
	//----------------------------------------------------------------------------------------------
	class System : public NonCopyable
	{
	public:
		System();
		~System();

		bool	init(const render::VideoMode& videoMode);
		void	tick(float dt);
		void	draw();

		bool	handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		bool	handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		bool	handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		bool	handleKeyboardEvent(const SDL_KeyboardEvent& e);
		bool	handleTextInputEvent(const SDL_TextInputEvent& e);

	private:
		Ptr<IBuffer>		m_vb;
		Ptr<IBuffer>		m_ib;
		Ptr<Material>		m_material;
		RenderOps			m_geomROPs;

		DepthStencilStateID m_stateDS;
		BlendStateID		m_stateB;
		RasterizerStateID	m_stateR;
		RasterizerStateID	m_stateR_scissor;
		SamplerStateID		m_stateS;
	};

} // ui
} // brUGE