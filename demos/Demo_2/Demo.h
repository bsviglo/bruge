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

	virtual bool init();
	virtual void shutdown();
	virtual void update(float dt);
	virtual void render(float dt);

	virtual bool handleMouseClick	(const MouseEvent& me);
	virtual bool handleMouseMove	(const MouseAxisEvent& mae);
	virtual bool handleKeyboardEvent(const KeyboardEvent& ke);

private:
	void gui();

private:
	std::vector<std::pair<mat4f, Node*>> m_collisionDescs;
	std::vector<mat4f>					 m_collisions;

	bool m_imguiActive;
	bool m_cursorVisible;
};
