#include "ui_system.hpp"
#include "render/render_common.h"
#include "SDL/SDL_clipboard.h"

namespace
{
	bool  g_MousePressed[3] = { false, false, false };
	float g_MouseWheel		= 0.0f;
}

namespace brUGE
{
namespace ui
{
	//----------------------------------------------------------------------------------------------
	System::System()
	{
	}

	//----------------------------------------------------------------------------------------------
	System::~System()
	{
	}

	//----------------------------------------------------------------------------------------------
	bool System::init(const render::VideoMode& videoMode)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab]			= SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_LeftArrow]	= SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow]	= SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow]		= SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow]	= SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp]		= SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown]	= SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home]		= SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End]			= SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Delete]		= SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace]	= SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter]		= SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape]		= SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_A]			= SDLK_a;
		io.KeyMap[ImGuiKey_C]			= SDLK_c;
		io.KeyMap[ImGuiKey_V]			= SDLK_v;
		io.KeyMap[ImGuiKey_X]			= SDLK_x;
		io.KeyMap[ImGuiKey_Y]			= SDLK_y;
		io.KeyMap[ImGuiKey_Z]			= SDLK_z;

		io.DisplaySize					= ImVec2((float)videoMode.width, (float)videoMode.height);
		io.RenderDrawListsFn			= nullptr;
		io.SetClipboardTextFn			= [] (void*, const char* text) { SDL_SetClipboardText(text); };
		io.GetClipboardTextFn			= [] (void*) -> const char* { return SDL_GetClipboardText(); };
		io.ClipboardUserData			= nullptr;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void System::tick(float dt)
	{
		ImGuiIO& io = ImGui::GetIO();
		
		io.DeltaTime = dt;
		
		// Setup inputs
		// (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
		int mx, my;
		uint32 mouseMask = SDL_GetMouseState(&mx, &my);
		//if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
		    io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		//else
		 //   io.MousePos = ImVec2(-1,-1);
		
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[0]   = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
		io.MouseDown[1]   = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2]   = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;
		
		io.MouseWheel = g_MouseWheel;
		g_MouseWheel = 0.0f;
		
		// Hide OS mouse cursor if ImGui is drawing it
		SDL_ShowCursor(io.MouseDrawCursor ? SDL_FALSE : SDL_TRUE);
		
		// Start the frame
		ImGui::NewFrame();
	}

	//----------------------------------------------------------------------------------------------
	void System::draw()
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		auto* drawData = ImGui::GetDrawData();

		void* vb = m_vb->map<void>(IBuffer::ACCESS_WRITE_DISCARD);
		void* ib = m_ib->map<void>(IBuffer::ACCESS_WRITE_DISCARD);

		if (vb && ib)
		{
			auto* vtx = static_cast<ImDrawVert*>(vb);
			auto* idx = static_cast<ImDrawIdx*>(ib);

			for (int i = 0; i < drawData->CmdListsCount; ++i)
			{
			    const ImDrawList* cmdList = drawData->CmdLists[i];

			    memcpy(vtx, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			    memcpy(idx, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			    vtx += cmdList->VtxBuffer.Size;
			    idx += cmdList->IdxBuffer.Size;
			}

			m_vb->unmap();
			m_ib->unmap();
		}

		rd()->setDepthStencilState(m_stateDS, 0);
		rd()->setRasterizerState(m_stateR);
		rd()->setBlendState(m_stateB, NULL, 0xffffffff);

		rd()->setVertexLayout(m_vl);
		rd()->setVertexBuffer(0, m_vb.get());
		rd()->setShader(m_shader.get());

		// Render command lists
		for (int i = 0; i < drawData->CmdListsCount; ++i)
		{
		    const auto* cmdList   = drawData->CmdLists[i];
		    const auto* vtxBuffer = cmdList->VtxBuffer.Data;
		    const auto* idxBuffer = cmdList->IdxBuffer.Data;

		    for (int c = 0; c < cmdList->CmdBuffer.Size; ++c)
		    {
		        const ImDrawCmd* cmd = &cmdList->CmdBuffer[c];

				rd()->setScissorRect(
						cmd.rect.x, rs().screenRes().height - (cmd.rect.y + cmd.rect.h),
						cmd.rect.w, cmd.rect.h
						);

				m_shader->setTexture("font", m_texture.get(), m_stateS);
				rd()->draw(PRIM_TOPOLOGY_TRIANGLE_LIST, 0, count * 4);


	            glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
	            glScissor((int)cmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
	            glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
		        idx_buffer += pcmd->ElemCount;
		    }
		}
		#undef OFFSETOF
	}

	//----------------------------------------------------------------------------------------------
	bool System::handleMouseButtonEvent(const SDL_MouseButtonEvent& e)
	{
		if (e.button == SDL_BUTTON_LEFT	  && e.state == SDL_PRESSED)	g_MousePressed[0] = true;
		if (e.button == SDL_BUTTON_RIGHT  && e.state == SDL_PRESSED)	g_MousePressed[1] = true;
		if (e.button == SDL_BUTTON_MIDDLE && e.state == SDL_PRESSED)	g_MousePressed[2] = true;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool System::handleMouseMotionEvent(const SDL_MouseMotionEvent& e)
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool System::handleMouseWheelEvent(const SDL_MouseWheelEvent& e)
	{
		g_MouseWheel = e.y > 0 ? +1.0f : -1.0f;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool System::handleKeyboardEvent(const SDL_KeyboardEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int key = e.keysym.sym & ~SDLK_SCANCODE_MASK;

        io.KeysDown[key]	= (e.type == SDL_KEYDOWN);
        io.KeyShift			= ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl			= ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt			= ((SDL_GetModState() & KMOD_ALT) != 0);
        io.KeySuper			= ((SDL_GetModState() & KMOD_GUI) != 0);

        return true;
	}

	//----------------------------------------------------------------------------------------------
	bool System::handleTextInputEvent(const SDL_TextInputEvent& e)
	{
		ImGui::GetIO().AddInputCharactersUTF8(e.text);
		return true;
	}
}
}
