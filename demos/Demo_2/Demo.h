#pragma once

#include "prerequisites.hpp"
#include "engine/IDemo.h"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::render;

namespace brUGE
{
	class Node;
}

//--------------------------------------------------------------------------------------------------
class Demo : public brUGE::IDemo
{
public:
	Demo();
	virtual ~Demo();

	virtual bool	init() override;
	virtual void	shutdown() override;
	virtual void	update(float dt) override;
	virtual void	render(float dt) override;

	virtual bool	handleMouseButtonEvent(const SDL_MouseButtonEvent& e) override;
	virtual bool	handleMouseMotionEvent(const SDL_MouseMotionEvent& e) override;
	virtual bool	handleMouseWheelEvent(const SDL_MouseWheelEvent& e) override;
	virtual bool	handleKeyboardEvent(const SDL_KeyboardEvent& e) override;

private:
	void gui();

private:
	std::vector<std::pair<mat4f, Node*>> m_collisionDescs;
	std::vector<mat4f>					 m_collisions;

	bool m_imguiActive;
	bool m_cursorVisible;
};
