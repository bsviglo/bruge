#pragma once

#include "prerequisites.hpp"
#include "Functors.hpp"

#include "render/render_common.h"
#include "render/Color.h"
#include "render/Font.h"
#include "utils/Singleton.h"

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

		//-- ToDo:
		// ����� ������������ ��������� ������ ��� �������� ��������� ���������� � �������.
		// ����������:	���������� ��� ������� �� ���� ����������� ������� ����������.
		// ����������:	������������ ������ ��� ����� ������, �.�. ���������������� ���������� ���������.
		//				���� �� ������� ������� ����������� ���������, �� ������������ ���������� ������� ���
		//				����� ���������. �������� ������� �� ������� �������, ������� ����������������� ���������
		//				��������� ����� ����. ������� ���� ����� ��������� ���������� � ������� �������,
		//				� �� ��������� ��������, ��� ����� ��������� ���� �������.
		//------------------------------------------
		//bool handleKey(uchar key, KeyState state, uint text);

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


	//-- It's very useful in case when we want to add to the console simple accessor to the global
	//-- variable.
	//----------------------------------------------------------------------------------------------
	template<typename T>
	class FunctorValue : public Functor
	{
	public:
		FunctorValue(T* value)	:	m_value(value) { }

		virtual void operator() (const ParamList& params)
		{
			if (params.size() == 0)
			{
				brUGE::Console::instance().printWarning(utils::parseFrom(*m_value));
			}
			else if (params.size() == 1)
			{
				*m_value = utils::parseTo<T>(params[0]);
			}
			else
			{
				throw std::runtime_error("0 or 1 arg is expected.");
			}
		}

	private:
		T* m_value;
	};

	//-- It's very useful in case when we want to add to the console simple accessor to the class
	//-- member variable.
	//----------------------------------------------------------------------------------------------
	template<typename T, typename OBJ>
	class FunctorObjValue : public Functor
	{
	public:
		FunctorObjValue(OBJ* obj, T OBJ::* value)	:	m_value(value), m_object(obj) { }

		virtual void operator() (const ParamList& params)
		{
			if (params.size() == 0)
			{
				brUGE::Console::instance().printWarning(utils::parseFrom(m_object->*m_value));
			}
			else if (params.size() == 1)
			{
				m_object->*m_value = utils::parseTo<T>(params[0]);
			}
			else
			{
				throw std::runtime_error("0 or 1 arg is expected.");
			}
		}

	private:
		typedef T OBJ::* ObjValuePtrType;

		ObjValuePtrType m_value;
		OBJ*			m_object;
	};
	
} //brUGE


#define ConPrint	brUGE::Console::instance().print
#define ConWarning	brUGE::Console::instance().printWarning
#define ConError	brUGE::Console::instance().printError


#define REGISTER_CONSOLE_VALUE(m_name, type, value) \
	brUGE::Console::instance().registerCommand(m_name, new FunctorValue<type>(&value))

#define REGISTER_CONSOLE_MEMBER_VALUE(m_name, type, value, className)	\
	brUGE::Console::instance().registerCommand(m_name,			\
		new FunctorObjValue<type, className>(this, &className::value))

#define REGISTER_CONSOLE_FUNC(m_name, func)	\
	brUGE::Console::instance().registerCommand(m_name, new FunctorFunc(func))

#define REGISTER_CONSOLE_METHOD(m_name, func, className)		\
	brUGE::Console::instance().registerCommand(m_name,			\
		new FunctorMethod<className>(this, &className::func))
