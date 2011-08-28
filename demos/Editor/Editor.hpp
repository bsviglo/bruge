#pragma once

#include "prerequisites.hpp"
#include "engine/IDemo.h"
#include "render/CursorCamera.hpp"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::render;

//--------------------------------------------------------------------------------------------------
class Editor : public brUGE::IDemo
{
public:
	Editor();
	virtual ~Editor();

	virtual bool init();
	virtual void shutdown();
	virtual void update(float dt);
	virtual void render(float dt);
	virtual bool loadGameObj(const std::string& name);
	virtual bool loadAnimation(const std::string& name);

	virtual bool handleMouseClick	(const MouseEvent& me);
	virtual bool handleMouseMove	(const MouseAxisEvent& mae);
	virtual bool handleKeyboardEvent(const KeyboardEvent& ke);

private:
	//-- UI represents visual access to the many configuration parameters.
	//------------------------------------------------------------------------------------------
	class UI : public NonCopyable
	{
	public:
		UI(Editor& editor);
		~UI();

		void update();
		void displayGameObjs();
		void displayAnimation();

	private:
		struct UIList
		{
			UIList() : m_enabled(false), m_scroll(0) { }

			bool					 m_enabled;
			int						 m_scroll;
			std::vector<std::string> m_list;
		};

		UIList			m_gameObjsList;
		UIList			m_animList;
		int				m_scroll;
		Editor&			m_self;
	};
	typedef std::unique_ptr<UI> UIPtr;
	friend class UI;

private:
	bool				m_guiActive;
	UIPtr				m_ui;
	Handle				m_gameObj;
	Handle				m_sunLight;
	vec2f				m_sunAngles;
	vec3f				m_xyz;
	mat4f				m_target;
	mat4f				m_source;
	Ptr<CursorCamera>	m_camera;
};