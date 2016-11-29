#pragma once

#include "prerequisites.hpp"
#include "engine/IDemo.h"
#include "render/CursorCamera.hpp"
#include <memory>

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::render;

//--------------------------------------------------------------------------------------------------
class Editor : public brUGE::IDemo
{
public:
	Editor();
	virtual ~Editor();

	virtual bool	init() override;
	virtual void	shutdown() override;
	virtual void	update(float dt) override;
	virtual void	render(float dt) override;
	bool			loadGameObj(const std::string& name);
	bool			loadAnimation(const std::string& name);

	virtual bool	handleMouseButtonEvent(const SDL_MouseButtonEvent& e) override;
	virtual bool	handleMouseMotionEvent(const SDL_MouseMotionEvent& e) override;
	virtual bool	handleMouseWheelEvent(const SDL_MouseWheelEvent& e) override;
	virtual bool	handleKeyboardEvent(const SDL_KeyboardEvent& e) override;

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
	//-- anim
	bool				m_activeSkinModel;
	bool				m_looped;
	bool				m_stepped;
	float				m_curFrame;
	float				m_numFrames;
	std::string			m_objName;
	std::string			m_animName;
	Handle				m_animCtrl;

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