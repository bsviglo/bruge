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

// TODO: 1) Авотоматизация проверки параметров консольных комманд. 
//		 2) Продвинутая система отрисовки консоли.
//		 3) Вынесение конфигурационных парметров консоли в конфиг.
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

		// Осуществляет настройку графического контента консоли.
		// Примечание:  Вы можете вносить информацию для отображения ДО вызова этого метода,
		//				посредвтом тех же фукнций 'printXXX' или макросов 'ConXXX' что и обычно.
		//------------------------------------------
		void initDrawingMode(const render::VideoMode& videoMode);

		bool visible() const	{ return m_isVisible; }
		void visible(bool flag);

		// Метод осуществляет обработку клавиш для введения текстовой информации в консоли.
		// Примечание:	необходимо его вызвать по мере поступления событий клавиатуры.
		// Примечание:	Используется ТОЛЬКО для ввода текста, т.е. функциональносит текстового редактора.
		//				Если же событие требует немедленной обработки, то используется внутренная фукнция для
		//				такой обработки. Например серфинг по строкам консоли, требует непосредственного получения
		//				состояния вверх вниз. Поэтому идет опрос состояния устройства о нажатии клавиши,
		//				а не получение меседжей, что может замедлить этот процесс.
		//------------------------------------------
		bool handleKey(uchar key, KeyState state, uint text);

		// Вызывается неявно движком на каждом тике.
		//------------------------------------------
		void draw();
		
		// Методы осуществляют добавление для отрисовки в консоли текстовой информации.
		//------------------------------------------
		void print		 (const char* text, ...);
		void print		 (const std::string& text);
		void print		 (const render::Color& color, const std::string& text);
		void print		 (const render::Color& color, const char* text, ...);
		void printWarning(const char* text, ...);
		void printWarning(const std::string& text);
		void printError	 (const char* text, ...);
		void printError	 (const std::string& text);

		// Регистрация функции консоли.
		//------------------------------------------
		void registerCommand(const std::string& name, Functor* handler);	

	private:
		int	 _drawLongLine(int y, const render::Color& color, const std::string& text); 
		void _splitLongLine(const render::Color& color, const std::string& text);
		void _processCommand();

		void _setupRender();
		void _realDraw();

		// методы формирования списка авто-дополенния. Поиска по этому списку и вставка в консоль.
		void _doAutoCompletion(const std::string& namePart);
		void _commitAutoCompletion();

		// Консольные комманды.
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
		std::string					m_editableLine; // содержимое текущей вводимой строки
		
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
		float						m_heightPercent;		// высота консоли
		uint						m_linesPerScreen;
		uint						m_maxSymbolsPerLine;	// максимальное количество символов в строке
	};
	
} //brUGE

// Макросы для облегчения занесения текстовых сообщения в консоль.
// Поддерживаются два типа макросов, ConPrint - принимает на вход brString
// строку или строку формата и список пораметров - с-like стиль,
// форматированные строки (работает аналогично printf).

#define ConPrint	brUGE::Console::instance().print
#define ConWarning	brUGE::Console::instance().printWarning
#define ConError	brUGE::Console::instance().printError

// Регистрация консольных комманд. Под коммандой понимается функция которая принимает
// на вход список параметров ввиде вектора строк и возвращет void.
// Консольными коммандами(функциями) могут быть как глобальные или статические функции так и
// методы класса, в этом случае кроме метода(который тоже должен соблюдать сигнатуру
// консольных комманд) необходимо также передевать имя и экземпляр этого класса.
// Note: Лучше всего регистрацию функций проводить внутри конструктора класса.
// Note: За преобразованием типов, количеством параметров и допустимостью значений
//				необходимо следить вручную из вызывающей функции.
// Note: Пример регистрации можно посмотреть в конструкторе Console.

#define REGISTER_CONSOLE_FUNC(m_name, funcName)\
	brUGE::Console::instance().registerCommand(m_name, new FunctorFunc(funcName))

#define REGISTER_CONSOLE_METHOD(m_name, funcName, className)\
	brUGE::Console::instance().registerCommand(m_name,\
		new FunctorMethod<className>(this, &className::funcName) )
