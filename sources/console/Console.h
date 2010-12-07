#pragma once

#include "prerequisites.h"
#include "Functors.h"

#include "render/render_common.h"
#include "render/Color.h"
#include "render/Font.h"
#include "utils/Singleton.h"
#include "control/input_listener.h"

#include <stdarg.h>
#include <map>
#include <vector>

// TODO: 1) �������������� �������� ���������� ���������� �������. 
//		 2) ����������� ������� ��������� �������.
//		 3) ��������� ���������������� ��������� ������� � ������.
//

namespace brUGE
{
	//	
	//------------------------------------------------------------
	class Console : public utils::Singleton<Console>
	{
	public:
		Console();
		~Console();

		// ������������ ��������� ������������ �������� �������.
		// ����������:  �� ������ ������� ���������� ��� ����������� �� ������ ����� ������,
		//				���������� ��� �� ������� 'printXXX' ��� �������� 'ConXXX' ��� � ������.
		//------------------------------------------
		void initDrawingMode(const render::VideoMode& videoMode);

		bool visible() const	{ return m_isVisible; }
		void visible(bool flag);

		// ����� ������������ ��������� ������ ��� �������� ��������� ���������� � �������.
		// ����������:	���������� ��� ������� �� ���� ����������� ������� ����������.
		// ����������:	������������ ������ ��� ����� ������, �.�. ���������������� ���������� ���������.
		//				���� �� ������� ������� ����������� ���������, �� ������������ ���������� ������� ���
		//				����� ���������. �������� ������� �� ������� �������, ������� ����������������� ���������
		//				��������� ����� ����. ������� ���� ����� ��������� ���������� � ������� �������,
		//				� �� ��������� ��������, ��� ����� ��������� ���� �������.
		//------------------------------------------
		bool handleKey(uchar key, KeyState state, uint text);

		// ���������� ������ ������� �� ������ ����.
		//------------------------------------------
		void draw();
		
		// ������ ������������ ���������� ��� ��������� � ������� ��������� ����������.
		//------------------------------------------
		void print		 (const char* text, ...);
		void print		 (const std::string& text);
		void print		 (const render::Color& color, const std::string& text);
		void print		 (const render::Color& color, const char* text, ...);
		void printWarning(const char* text, ...);
		void printWarning(const std::string& text);
		void printError	 (const char* text, ...);
		void printError	 (const std::string& text);

		// ����������� ������� �������.
		//------------------------------------------
		void registerCommand(const std::string& name, Functor* handler);	

	private:
		int	 _drawLongLine(int y, const render::Color& color, const std::string& text); 
		void _splitLongLine(const render::Color& color, const std::string& text);
		void _processCommand();

		void _setupRender();
		void _realDraw();

		// ������ ������������ ������ ����-����������. ������ �� ����� ������ � ������� � �������.
		void _doAutoCompletion(const std::string& namePart);
		void _commitAutoCompletion();

		// ���������� ��������.
		//------------------------------------------
		int _helpHandler	();
		int _currentMsgCount();

	private:
		struct ConsoleLine
		{
			ConsoleLine(const std::string& text_, const render::Color& color_) : text(text_), color(color_) { }
			std::string	text;
			render::Color color;
		};

		typedef std::map<std::string, Functor*>	CommandsMap;
		typedef CommandsMap::iterator			CommandsMapIter;
		typedef std::vector<std::string>		CommandHistory;
		typedef std::vector<ConsoleLine>		ConsoleHistory;
		typedef std::vector<std::string>		CommandsAutoCompletion;
		

		CommandsMap					m_cmdsMap;
		CommandsMapIter				m_cmdsMapIter;
		CommandHistory				m_cmdsHistory;
		ConsoleHistory				m_consoleHistory;
		CommandsAutoCompletion		m_curCmdsAutoCmpl;	
		
		bool						m_autoCmplEnaled;
		int							m_curAutoCmplIndex;
		int							m_curAutoCmplStrLen;
		int							m_curHeadOfConsoleHistory;
		uint						m_cursorPos;
		bool						m_changedLineNumber;
		int							m_curCmdHistoryIndex;
		bool						m_isVisible;
		bool						m_showCursor;
		std::string					m_searchWorld;
		std::string					m_editableLine; // ���������� ������� �������� ������
		
		// render section:
		Ptr<render::IBuffer>		m_vb;
		Ptr<render::IShader>		m_shader;
		Ptr<render::ITexture>		m_texture;

		render::VertexLayoutID		m_vl;
		render::SamplerStateID		m_stateS;
		render::DepthStencilStateID m_stateDS;
		render::RasterizerStateID	m_stateR;
		render::BlendStateID		m_stateB;

		Ptr<render::Font>			m_font;
		render::Font::Desc			m_fontDesc;
		uint						m_heightSc;
		uint						m_widthSc;
		float						m_heightPercent;		// ������ �������
		uint						m_linesPerScreen;
		uint						m_maxSymbolsPerLine;	// ������������ ���������� �������� � ������
	};
	
} //brUGE

// ������� ��� ���������� ��������� ��������� ��������� � �������.
// �������������� ��� ���� ��������, ConPrint - ��������� �� ���� brString
// ������ ��� ������ ������� � ������ ���������� - �-like �����,
// ��������������� ������ (�������� ���������� printf).

#define ConPrint	brUGE::Console::instance().print
#define ConWarning	brUGE::Console::instance().printWarning
#define ConError	brUGE::Console::instance().printError

// ����������� ���������� �������. ��� ��������� ���������� ������� ������� ���������
// �� ���� ������ ���������� ����� ������� ����� � ��������� void.
// ����������� ����������(���������) ����� ���� ��� ���������� ��� ����������� ������� ��� �
// ������ ������, � ���� ������ ����� ������(������� ���� ������ ��������� ���������
// ���������� �������) ���������� ����� ���������� ��� � ��������� ����� ������.
// Note: ����� ����� ����������� ������� ��������� ������ ������������ ������.
// Note: �� ��������������� �����, ����������� ���������� � ������������� ��������
//				���������� ������� ������� �� ���������� �������.
// Note: ������ ����������� ����� ���������� � ������������ Console.

#define REGISTER_CONSOLE_FUNC(m_name, funcName)\
	brUGE::Console::instance().registerCommand(m_name, new FunctorFunc(funcName))

#define REGISTER_CONSOLE_METHOD(m_name, funcName, className)\
	brUGE::Console::instance().registerCommand(m_name,\
		new FunctorMethod<className>(this, &className::funcName) )
