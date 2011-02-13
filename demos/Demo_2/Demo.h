#pragma once

#include "prerequisites.h"
#include "engine/IDemo.h"
#include "render/FreeCamera.h"

using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::render;

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
	Ptr<FreeCamera>	m_camera;

	std::vector<std::pair<mat4f, Node*>> m_collisionDescs;
	std::vector<mat4f>					 m_collisions;

	struct imguiInput
	{
		int  mx, my;
		byte button;
		int  scroll;
	};
	imguiInput	m_imguiInput;
	bool		m_imguiActive;
};
