#pragma once

#include "prerequisites.hpp"
#include <vector>

class Editor;

//-- UI represents visual access to the many configuration Editor parameters.
//----------------------------------------------------------------------------------------------
class EditorUI : public brUGE::NonCopyable
{
public:
	EditorUI(Editor& editor);
	~EditorUI();

	void update();

private:
	struct GameObjComboList
	{
		GameObjComboList() : m_enabled(false), m_scroll(0) { }
		void display(EditorUI& rootUI);

		bool					 m_enabled;
		int						 m_scroll;
		std::vector<std::string> m_list;
	};

private:
	GameObjComboList	m_cl_gameObjs;
	int					m_scroll;
	Editor&				m_self;
};