#pragma once

#include "prerequisites.hpp"
#include "imgui/imgui.h"
#include "render/render_common.h"
#include "SDL/SDL_events.h"

namespace brUGE
{
namespace ui
{

	//-- Implements imgui interface.
	//----------------------------------------------------------------------------------------------
	class UISystem : public ISystem
	{
	public:
		UISystem();
		~UISystem();

		bool	init(const render::VideoMode& videoMode);
		void	tick(float dt);
		void	draw();

		bool	handleMouseButtonEvent(const SDL_MouseButtonEvent& e);
		bool	handleMouseMotionEvent(const SDL_MouseMotionEvent& e);
		bool	handleMouseWheelEvent(const SDL_MouseWheelEvent& e);
		bool	handleKeyboardEvent(const SDL_KeyboardEvent& e);
		bool	handleTextInputEvent(const SDL_TextInputEvent& e);

	private:
		void	setupRender();

	private:
		std::shared_ptr<render::ITexture>	m_texture;
		std::shared_ptr<render::IBuffer>	m_vb;
		std::shared_ptr<render::IBuffer>	m_ib;
		std::shared_ptr<render::IShader>	m_shader;

		render::VertexLayoutID				m_vl;
		render::DepthStencilStateID			m_stateDS;
		render::BlendStateID				m_stateB;
		render::RasterizerStateID			m_stateR;
		render::SamplerStateID				m_stateS;
	};

} // ui
} // brUGE