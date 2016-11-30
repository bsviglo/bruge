#include "Console.h"

#include "console/TimingPanel.h"
#include "render/render_system.hpp"
#include "render/IRenderDevice.h"
#include "render/ITexture.h"
#include "render/IBuffer.h"
#include "render/IShader.h"
#include "render/state_objects.h"

#include "loader/ResourcesManager.h"
#include "os/FileSystem.h"
#include "utils/LogManager.h"
#include "utils/string_utils.h"

using namespace brUGE;
using namespace brUGE::utils;
using namespace brUGE::render;

// start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	#define PRINT_BUFFER_LENGTH 1024

	//-- global to this module constants.
	uint  g_consoleVMargin      = 0;
	uint  g_consoleHMargin	    = 2;
	float g_consoleBlinkTimeout = 0.5; // sec

	Color g_warningColor (1.0f, 1.0f, 0.0f, 1.0f);
	Color g_errorColor   (1.0f, 0.0f, 0.0f, 1.0f);
	Color g_commandColor (0.0f, 0.8f, 0.9f, 1.0f);
	Color g_autoCmplColor(1.0f, 1.0f, 1.0f, 1.0f);

	struct VertexPT
	{
		vec3f pos;
		vec2f texCoord;
	};
}
//--------------------------------------------------------------------------------------------------
// end unnamed namespace.


namespace brUGE
{
	DEFINE_SINGLETON(Console)

	int _prevedHandler();
	
	//---------------------------------------------------------------------------------------------
	Console::Console()
	   :	m_curHeadOfConsoleHistory(0),
	     	m_curCmdHistoryIndex(-1),		
		 	m_changedLineNumber(false),
		 	m_isVisible(false),
		 	m_heightPercent(0.4f),
		 	m_cursorPos(0),
		 	m_showCursor(true),
		 	m_font(NULL),
			m_curAutoCmplIndex(0),
			m_autoCmplEnaled(false),
		 	m_maxSymbolsPerLine(100), //-- default size in symbols of console line. In method 
									  //-- initDrawingMode it will be adjusted.
		 	m_linesPerScreen(32)
	{
		
		// TODO: reconsider.
		m_consoleHistory.reserve(100);

		//-- register console commands.
		REGISTER_CONSOLE_METHOD("help", _helpHandler, Console);
		REGISTER_CONSOLE_METHOD("consoleInfo", _currentMsgCount, Console);
		REGISTER_CONSOLE_FUNC  ("preved", _prevedHandler);
	}

	//---------------------------------------------------------------------------------------------
	Console::~Console()
	{
		INFO_MSG("De initialization console.");

		//-- delete console commands.
		for (m_cmdsMapIter = m_cmdsMap.begin(); m_cmdsMapIter != m_cmdsMap.end(); ++m_cmdsMapIter)
			delete m_cmdsMapIter->second;
	}

	//---------------------------------------------------------------------------------------------
	void Console::visible(bool flag)
	{
		if (m_isVisible == flag) return;

		if (!flag)
		{
			_commitAutoCompletion();
			m_editableLine.clear();
			m_cursorPos = 0;
		}
		
		m_isVisible = flag;
	}

	//---------------------------------------------------------------------------------------------
	void Console::initDrawingMode(const VideoMode& videoMode)
	{
		m_heightSc	  = videoMode.height;
		m_widthSc	  = videoMode.width;
		uint height	  = m_heightSc * m_heightPercent;
		uint fontSize = height / m_linesPerScreen;
		
		// TODO: move to config.

		if (fontSize < 12)
		{
			fontSize = 12;
			m_linesPerScreen = height / fontSize;
		}
		
		m_font = ResourcesManager::instance().loadFont("system/font/VeraMono", fontSize, vec2ui(32, 127));
		m_font->getDesc(m_fontDesc);

		m_maxSymbolsPerLine = (m_widthSc - g_consoleHMargin * 2) / m_fontDesc.width;
		
		_setupRender();
	}

	//-- displays the text and divides the console message into substrings with display length.
	//-- returns height.
	//---------------------------------------------------------------------------------------------
	int Console::_drawLongLine(int y, const Color& color, const std::string& text)
	{
		int linesNumber = (text.size() % m_maxSymbolsPerLine) ? 
			text.size() / m_maxSymbolsPerLine + 1 : text.size() / m_maxSymbolsPerLine;
		
		std::string tmpStr;
		for (int i = 0; i < linesNumber; ++i)
		{
			tmpStr = text.substr(i * m_maxSymbolsPerLine,
				(text.size() < i * m_maxSymbolsPerLine + m_maxSymbolsPerLine) ?
				text.size() - i * m_maxSymbolsPerLine : m_maxSymbolsPerLine);

			m_font->draw2D(
				vec2f(g_consoleHMargin, y - m_fontDesc.height * (linesNumber - i - 1)),
				color,
				tmpStr
				);
		}

		return m_fontDesc.height * linesNumber;
	}

	//-- split long string into small substrings. This function used by adding message to the console.
	//---------------------------------------------------------------------------------------------
	void Console::_splitLongLine(const Color& color, const std::string &text)
	{
		int linesNumber = (text.size() % m_maxSymbolsPerLine) ?
			text.size() / m_maxSymbolsPerLine + 1 : text.size() / m_maxSymbolsPerLine;

		for (int i = 0; i < linesNumber; ++i)
		{
			m_consoleHistory.push_back(
				ConsoleLine(text.substr (i * m_maxSymbolsPerLine, 
				   	(text.size() < i * m_maxSymbolsPerLine + m_maxSymbolsPerLine) ? 
						text.size() - i * m_maxSymbolsPerLine : m_maxSymbolsPerLine),
					color
					)
				);
		}

		m_changedLineNumber = true;
	}
	
	//---------------------------------------------------------------------------------------------
	void Console::draw()
	{
		SCOPED_TIME_MEASURER_EX("Console draw")

		if (!m_isVisible) return;
		
		if(m_curHeadOfConsoleHistory == 0 || m_changedLineNumber)
			m_curHeadOfConsoleHistory = m_consoleHistory.size();
		
		_realDraw();
		
		m_changedLineNumber = false;
	}

	//---------------------------------------------------------------------------------------------
	void Console::_setupRender()
	{
		//-- create shader and vertex layout.
		{
			m_shader = ResourcesManager::instance().loadShader("console");
			if (!m_shader)
				BR_EXCEPT("failed load shader");

			VertexDesc desc[] = 
			{
				{ 0, SEMANTIC_POSITION,	TYPE_FLOAT, 3},
				{ 0, SEMANTIC_TEXCOORD,	TYPE_FLOAT, 2}
			};
			m_vl = render::rd()->createVertexLayout(desc, 2, *m_shader.get());
		}
		
		//-- create render states.
		{
			SamplerStateDesc sDesc;
			sDesc.minMagFilter	= SamplerStateDesc::FILTER_BILINEAR;
			sDesc.wrapS		 	= SamplerStateDesc::ADRESS_MODE_WRAP;
			sDesc.wrapT		 	= SamplerStateDesc::ADRESS_MODE_WRAP;
			m_stateS = render::rd()->createSamplerState(sDesc);

			//-- prepare render states.
			DepthStencilStateDesc dsDesc;
			dsDesc.depthEnable = false;
			m_stateDS = render::rd()->createDepthStencilState(dsDesc);

			RasterizerStateDesc rDesc;
			m_stateR = render::rd()->createRasterizedState(rDesc);

			BlendStateDesc bDesc;
			bDesc.blendEnable[0] = true;
			bDesc.srcBlend		 = BlendStateDesc::BLEND_FACTOR_SRC_ALPHA;
			bDesc.destBlend		 = BlendStateDesc::BLEND_FACTOR_INV_SRC_ALPHA;
			bDesc.blendOp		 = BlendStateDesc::BLEND_OP_ADD;
			m_stateB = render::rd()->createBlendState(bDesc);
		}
				
		//-- vertices already in clip-space.
		{
			VertexPT vertices[] = 
			{
				{ //-- bottom left
					vec3f(-1.0f, 1.0f - 2 * m_heightPercent, 0.0f),
						vec2f(0.0f, 1.0f),
				},

				{ //-- top left
					vec3f(-1.0f, 1.0f, 0.0f),
					vec2f(0.0f, 0.0f),
				},

				{ //-- bottom right
					vec3f(1.0f, 1.0f - 2 * m_heightPercent, 0.0f),
					vec2f(4.0f, 1.0f),
				},

				{ //-- top right
					vec3f(1.0f, 1.0f, 0.0f),
					vec2f(4.0f, 0.0f)
				}
			};
			m_vb = render::rd()->createBuffer(IBuffer::TYPE_VERTEX, vertices, 4, sizeof(VertexPT));
		}
		
		m_texture = ResourcesManager::instance().loadTexture(
						"system\\textures\\defaultconsole.dds");
		if (!m_texture)
			BR_EXCEPT("can't create texture");
	}
	
	//---------------------------------------------------------------------------------------------
	void Console::_realDraw()
	{
		render::rd()->setDepthStencilState(m_stateDS, 0);
		render::rd()->setRasterizerState(m_stateR);
		render::rd()->setBlendState(m_stateB, NULL, 0xffffffff);

		render::rd()->setVertexLayout(m_vl);
		render::rd()->setVertexBuffer(0, m_vb.get());
		render::rd()->setShader(m_shader.get());

		m_shader->setTexture("con", m_texture.get(), m_stateS);

		render::rd()->draw(PRIM_TOPOLOGY_TRIANGLE_STRIP, 0, 4);

		static uint64 time = SDL_GetPerformanceCounter();
		uint y = m_heightSc * m_heightPercent + g_consoleVMargin;

		if (static_cast<float>((SDL_GetPerformanceCounter() - time) / SDL_GetPerformanceFrequency()) >= g_consoleBlinkTimeout)
		{
			m_showCursor = !m_showCursor;
			time = SDL_GetPerformanceCounter();
		}

		m_font->beginDraw();
		
		//-- draw edit line.
		y -= _drawLongLine(y, g_commandColor, ">" + m_editableLine);

		//-- draw auto-completion text.
		if (m_autoCmplEnaled)
		{
			m_font->draw2D
				(vec2f((m_cursorPos - m_curAutoCmplStrLen) * m_fontDesc.width + g_consoleHMargin + m_fontDesc.width,
				m_heightSc * m_heightPercent), g_autoCmplColor, m_curCmdsAutoCmpl[m_curAutoCmplIndex - 1]);
		}

		//-- draw cursor.
		if (m_showCursor)
			m_font->draw2D(vec2f(m_cursorPos * m_fontDesc.width + g_consoleHMargin + m_fontDesc.width,
				m_heightSc * m_heightPercent + g_consoleVMargin - 2), g_commandColor, "_");

		//-- draw the console history.
		for (int i = m_curHeadOfConsoleHistory - 1; (i >= 0) && (y <= m_heightSc - m_fontDesc.height); --i)
		{
			m_font->draw2D(vec2f(g_consoleHMargin, y), m_consoleHistory[i].color, m_consoleHistory[i].text);
			y -= m_fontDesc.height;
		}

		m_font->endDraw();
	}

	//---------------------------------------------------------------------------------------------
	void Console::print(const Color& color, const char* text,...)
	{
		char temp[PRINT_BUFFER_LENGTH];
		va_list ap;
		if (!text) return;
		va_start(ap, text);
		_vsnprintf_s(temp, PRINT_BUFFER_LENGTH, _TRUNCATE, text, ap);
		va_end(ap);

		_splitLongLine(color, temp);
	}
	
	//---------------------------------------------------------------------------------------------
	void Console::print(const Color& color, const std::string &text)
	{
		_splitLongLine(color, text);
	}

	//---------------------------------------------------------------------------------------------
	void Console::print(const char* text, ...)
	{
		char temp[PRINT_BUFFER_LENGTH];
		va_list ap;
		if (!text) return;
		va_start(ap, text);
		_vsnprintf_s(temp, PRINT_BUFFER_LENGTH, _TRUNCATE, text, ap);
		va_end(ap);

		_splitLongLine(g_commandColor, temp);
	}

	//---------------------------------------------------------------------------------------------
	void Console::print(const std::string &text)
	{
		print(g_commandColor, text);
	}
	
	//---------------------------------------------------------------------------------------------
	void Console::printWarning(const char* text, ...)
	{
		char temp[PRINT_BUFFER_LENGTH];
		va_list ap;
		if (!text) return;
		va_start(ap, text);
		_vsnprintf_s(temp, PRINT_BUFFER_LENGTH, _TRUNCATE, text, ap);
		va_end(ap);

		_splitLongLine(g_warningColor, temp);		
	}

	//---------------------------------------------------------------------------------------------
	void Console::printWarning(const std::string &text)
	{
		print(g_warningColor, text);
	}

	//---------------------------------------------------------------------------------------------
	void Console::printError(const char* text, ...)
	{
		char temp[PRINT_BUFFER_LENGTH];
		va_list ap;
		if (!text) return;
		va_start(ap, text);
		_vsnprintf_s(temp, PRINT_BUFFER_LENGTH, _TRUNCATE, text, ap);
		va_end(ap);

		_splitLongLine(g_errorColor, temp);
	}

	//---------------------------------------------------------------------------------------------
	void Console::printError(const std::string &text)
	{
		print(g_errorColor, text);
	}
	
	//-- add the new console command.
	//--------------------------------------------------------------------------------------------- 
	void Console::registerCommand(const std::string& name, Functor* handler)
	{
		m_cmdsMap[name] = handler;
	}
	
	//-- ToDo:
	//-- handles the keyboard input.
	//---------------------------------------------------------------------------------------------
#if 0
	bool Console::handleKey(uchar key, KeyState state, uint text)
	{
		if (!m_isVisible) return false;

		if (key == DIK_TAB && (state == KS_DOWN || state == KS_PRESSED))
		{
			if (!m_editableLine.empty())
				_doAutoCompletion(m_editableLine);
			return true;
		}
		else
		{
			if (!(state == KS_UP || state == KS_PRESSED))
				_commitAutoCompletion();
		}

		//-- Page Up
		if (key == DIK_PGUP && (state == KS_DOWN || state == KS_PRESSED))
		{
			if(m_curHeadOfConsoleHistory > 1)
				m_curHeadOfConsoleHistory--;
			return true;
		}

		//-- Page Down
		if (key == DIK_PGDN && (state == KS_DOWN || state == KS_PRESSED))
		{
			if((m_consoleHistory.size() - m_curHeadOfConsoleHistory) > 0)
				m_curHeadOfConsoleHistory++;
			return true;
		}

		//-- Backspace
		if (key == DIK_BACKSPACE && state)
		{
			if (m_cursorPos > 0)
			{
				m_editableLine.erase(m_cursorPos - 1, 1);
				--m_cursorPos;
				m_showCursor = true;
			}
			return true;
		}

		if (key == DIK_DELETE && state)
		{
			if (m_cursorPos < m_editableLine.length())
			{
				m_editableLine.erase(m_cursorPos, 1);
				m_showCursor = true;
			}
			return true;
		}

		if (key == DIK_RIGHT && state)
		{
			if (m_cursorPos < m_editableLine.length())
				++m_cursorPos;
			m_showCursor = true;
			return true;
		}

		if (key == DIK_LEFT && state)
		{
			if (m_cursorPos > 0)
				--m_cursorPos;
			m_showCursor = true;
			return true;
		}

		if (key == DIK_HOME && state == KS_DOWN)
		{
			m_cursorPos = 0;
			return true;
		}

		if (key == DIK_END && state == KS_DOWN)
		{
			m_cursorPos = m_editableLine.length();
			return true;
		}

		//-- ENTER
		if (key == DIK_RETURN && state == KS_DOWN && m_editableLine.size() > 0)
		{
			this->print (g_commandColor, ">" + m_editableLine);
			m_cmdsHistory.push_back (m_editableLine);
			m_curCmdHistoryIndex = m_cmdsHistory.size();
			this->_processCommand ();
			m_editableLine.clear ();
			m_cursorPos = 0;
			return true;
		}

		//-- UP
		if (key == DIK_UP && state)
		{
			m_curCmdHistoryIndex--;
			if (m_curCmdHistoryIndex >= (int)m_cmdsHistory.size())
				m_curCmdHistoryIndex = (int)m_cmdsHistory.size() - 1;
			if (m_curCmdHistoryIndex < 0)
				m_curCmdHistoryIndex = 0;
			if (m_cmdsHistory.size() > 0)
				m_editableLine = m_cmdsHistory[m_curCmdHistoryIndex];
			m_cursorPos = m_editableLine.length();
			return true;
		}

		//-- DOWN
		if (key == DIK_DOWN && state)
		{
			m_curCmdHistoryIndex++;
			if (m_curCmdHistoryIndex >= (int)m_cmdsHistory.size())
				m_curCmdHistoryIndex = (int)m_cmdsHistory.size() - 1;
			if (m_curCmdHistoryIndex < 0) m_curCmdHistoryIndex = 0;
			if (m_cmdsHistory.size() > 0)
				m_editableLine = m_cmdsHistory[m_curCmdHistoryIndex];
			m_cursorPos = m_editableLine.length();
			return true;
		}

		if ( 
			  ((uchar)text >= 32 && (uchar)text <=127)	&&
			  key != DIK_GRAVE							&&
			  state
		   )
		{
			m_editableLine.insert(m_cursorPos, 1, text);
			++m_cursorPos;
			m_showCursor = true;
		}

		return true;
	}
#endif
	
	//-- This function performs parsing of a console command.
	//-- Note: At first we look for command name in the registered commands list. Then we parse 
	//--	   parameters, which passed into the founded console command like that
	//--	   _processCommand(ParamList).
	//--	   Function parameters must be entered immediately after the console command and divided
	//--	   by white-spaces.
	//--
	//-- Example:
	//--		   setWireframeMode true
	//--		   playSound ambient		
	//---------------------------------------------------------------------------------------------
	void Console::_processCommand()
	{
		StrTokenizer tokenizer(m_editableLine, " ");
		std::string	token;
		CommandsMapIter	command;

		if (tokenizer.hasMoreTokens())
		{
			tokenizer.nextToken(token);
			if ((command = m_cmdsMap.find(token)) == m_cmdsMap.end())
			{
				print(g_warningColor, "Command '%s' not found", token.c_str());
				return;
			}
		}
		else
		{
			return;
		}
		
		ParamList params;
		while (tokenizer.hasMoreTokens())
		{
			tokenizer.nextToken(token);
			params.push_back(token);
		}
		
		try	
		{
			(*(command->second))(params);
		}
		catch (std::exception& e)
		{
			ConError(e.what());
		}
	}
	
	//-- generates list of auto completion and performs searching in it.
	//---------------------------------------------------------------------------------------------
	void Console::_doAutoCompletion(const std::string& namePart)
	{
		if (m_searchWorld != namePart)
		{
			m_curCmdsAutoCmpl.clear();

			m_searchWorld		= namePart;
			m_autoCmplEnaled	= true;
			m_curAutoCmplIndex	= 0;
			m_curAutoCmplStrLen = 0;

			size_t iStrLen = namePart.length();
			size_t oStrLen = 0;

			for (CommandsMapIter iter = m_cmdsMap.begin(); iter != m_cmdsMap.end(); ++iter)
			{
				oStrLen = iter->first.length();
				if ((iStrLen < oStrLen) && (iter->first.find(namePart) == 0))
					m_curCmdsAutoCmpl.push_back(iter->first.substr(iStrLen, oStrLen - iStrLen));
			}

			if (!m_curCmdsAutoCmpl.size())
				m_autoCmplEnaled = false;
		}
		
		if (m_autoCmplEnaled)
		{
			//-- repeat cycle for all auto-completion of the string.
			if (m_curAutoCmplIndex >= (int)m_curCmdsAutoCmpl.size())
			{
				m_curAutoCmplIndex = 0;
				m_cursorPos -= m_curAutoCmplStrLen;
			}

			if (m_curAutoCmplIndex != 0) m_cursorPos -= m_curAutoCmplStrLen;
			m_curAutoCmplStrLen = m_curCmdsAutoCmpl[m_curAutoCmplIndex++].length();
			m_cursorPos += m_curAutoCmplStrLen;
		}
	}
	
	//-- applies the selected text of auto-completion to the console function.
	//---------------------------------------------------------------------------------------------
	void Console::_commitAutoCompletion()
	{
		if (!m_autoCmplEnaled) return;
		
		m_searchWorld.clear();
		m_curAutoCmplStrLen = 0;
		m_autoCmplEnaled	= false;
		m_editableLine.append(m_curCmdsAutoCmpl[m_curAutoCmplIndex - 1]);
	}

	//
	//-- standard console functions.
	//---------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------
	int Console::_helpHandler()
	{
		ConPrint("To close console, press '~' key");
		std::string cmds;
		for (m_cmdsMapIter = m_cmdsMap.begin(); m_cmdsMapIter != m_cmdsMap.end(); ++m_cmdsMapIter)
			cmds += m_cmdsMapIter->first + ' ';

		ConPrint("Available commands: " + cmds);
		return 0;
	}

	//---------------------------------------------------------------------------------------------
	int Console::_currentMsgCount()
	{
		ConPrint(" * lines count		: %i", m_consoleHistory.size() + 3);
		ConPrint(" * commands count		: %i", m_cmdsMap.size());
		ConPrint(" * lines per screen	: %i", m_linesPerScreen);
		return 0;
	}
	
	// legacy of EvGenius :)
	//---------------------------------------------------------------------------------------------
	int _prevedHandler ()
	{
		ConPrint	("Preved, Medved!!!");
		ConWarning	("Preved, Medved!!!");
		ConError	("Preved, Medved!!!");
		return 0;
	}

} // brUGE