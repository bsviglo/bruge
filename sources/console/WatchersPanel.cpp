#include "WatchersPanel.h"
#include "console/TimingPanel.h"
#include "render/Color.h"
#include "render/render_system.hpp"
#include "utils/string_utils.h"
#include "gui/imgui/imgui.h"

using namespace brUGE::render;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{

	//-- global to this module constants.
	Color g_headerColor  (1.0f, 1.0f, 0.0f);
	Color g_roColor		 (0.8f, 0.8f, 0.8f);
	Color g_rwColor		 (1.0f, 1.0f, 1.0f);

}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{
	DEFINE_SINGLETON(WatchersPanel)

	//---------------------------------------------------------------------------------------------
	WatchersPanel::WatchersPanel() : m_isVisible(false), m_scroll(0)
	{
		
	}

	//---------------------------------------------------------------------------------------------
	WatchersPanel::~WatchersPanel()
	{
		for (uint i = 0; i < m_roWatchers.size(); ++i)
		{
			delete m_roWatchers[i].m_watcher;
		}

		for (uint i = 0; i < m_rwWatchers.size(); ++i)
		{
			delete m_rwWatchers[i].m_watcher;
		}
		
		m_roWatchers.clear();
		m_rwWatchers.clear();
	}

	//---------------------------------------------------------------------------------------------
	bool WatchersPanel::init()
	{
		return true;
	}

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::update(float /*dt*/)
	{
		SCOPED_TIME_MEASURER_EX("WatchersPanel update")

		if (!m_isVisible) return;
	}

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::visualize()
	{
		SCOPED_TIME_MEASURER_EX("WatchersPanel draw")

		if (!m_isVisible) return;

		//uint height = rs().screenRes().height;
		//
		//imguiBeginScrollArea("Real-time watchers console", 10, height*0.4, 450, height*0.6-10, &m_scroll);
		//{
		//	std::string line;
		//
		//	for (uint i = 0; i < m_roWatchers.size(); ++i)
		//	{
		//		const WatcherDesc& desc = m_roWatchers[i];
		//		line = makeStr("%s = %s", desc.m_name.c_str(), desc.m_watcher->get().c_str());
		//		imguiLabel(line.c_str());
		//	}
		//
		//	for (uint i = 0; i < m_rwWatchers.size(); ++i)
		//	{
		//		const WatcherDesc& desc = m_rwWatchers[i];
		//		line = makeStr("%s = %s", desc.m_name.c_str(), desc.m_watcher->get().c_str());
		//		imguiLabel(line.c_str());
		//	}
		//}
		//imguiEndScrollArea();
	}

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::registerWatcher(
		const std::string& name, EWatcherAccess access, IWatcher* watcher)
	{
		WatcherDesc desc;
		desc.m_desc		= "";
		desc.m_name		= name;
		desc.m_watcher	= watcher;

		if (access == ACCESS_READ_ONLY)
		{
			m_roWatchers.push_back(desc);
		}
		else
		{
			m_rwWatchers.push_back(desc);
		}
	}

} // brUGE