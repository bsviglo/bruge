#include "EditorUI.hpp"
#include "Editor.hpp"
#include "render/render_common.h"
#include "render/render_system.hpp"
#include "os/FileSystem.h"
#include "gui/imgui.h"

using namespace brUGE;
using namespace brUGE::os;
using namespace brUGE::math;
using namespace brUGE::utils;
using namespace brUGE::render;
using namespace brUGE::physics;

//--------------------------------------------------------------------------------------------------
EditorUI::EditorUI(Editor& editor) : m_self(editor), m_scroll(0)
{

}

//--------------------------------------------------------------------------------------------------
EditorUI::~EditorUI()
{

}

//--------------------------------------------------------------------------------------------------
void EditorUI::update()
{
	uint width	= rs().screenRes().width;
	uint height = rs().screenRes().height;

	imguiBeginScrollArea("Editor", width-300-10, 10, 300, height-20, &m_scroll);
	{
		imguiSeparatorLine();
		imguiLabel("Models:");
		{
			imguiIndent();
			if (imguiButton("Load game object..."))
			{
				m_cl_gameObjs.m_enabled = !m_cl_gameObjs.m_enabled;
				if (m_cl_gameObjs.m_enabled)
				{
					m_cl_gameObjs.m_list.clear();
					FileSystem::instance().getFilesInDir("..\\resources\\models", m_cl_gameObjs.m_list, "xml", true);
				}
			}
			imguiUnindent();
		}

		imguiLabel("Debug:");
		{
			imguiIndent();
			if (imguiCheck("display normals", &m_self.m_enableDebugNormals))
			{
				m_self.updateShowNormals();
			}
			imguiUnindent();
		}

		imguiLabel("Sun:");
		{
			imguiIndent();
			imguiSlider("sun yaw angle", &m_self.m_sunAngles.x, 0.0f, 360.0f, 0.5f);
			imguiSlider("sun pitch angle", &m_self.m_sunAngles.y, 0.0f, 90.0f, 0.5f);
			imguiUnindent();
		}

		imguiLabel("Material:");
		{
			imguiIndent();
			imguiUnindent();
		}
		imguiLabel("Physics:");
		{
			static float mass = 0.0f;

			imguiIndent();
			imguiButton("choose node...");
			imguiSlider("mass, kg", &mass, 0.0f, 500.0f, 0.1f);
			imguiButton("choose shape type...");
			{
				imguiIndent();
				imguiUnindent();
			}
			imguiUnindent();
		}
	}
	imguiEndScrollArea();

	//-- display combo lists.
	m_cl_gameObjs.display(*this);
}

//--------------------------------------------------------------------------------------------------
void EditorUI::GameObjComboList::display(EditorUI& rootUI)
{
	if (m_enabled)
	{
		uint width	= rs().screenRes().width;
		uint height = rs().screenRes().height;

		imguiBeginScrollArea("Objects", width-520, height-10-250, 200, 250, &m_scroll);

		for (uint i = 0; i < m_list.size(); ++i)
		{
			const char* name = m_list[i].c_str();

			if (imguiItem(name))
			{
				rootUI.m_self.loadGameObj(name);
				m_enabled = false;
			}
		}
		imguiEndScrollArea();
	}
}
