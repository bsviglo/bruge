#include "WatchersPanel.h"
#include "console/TimingPanel.h"
#include "render/Color.h"
#include "loader/ResourcesManager.h"
#include "utils/string_utils.h"

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
	WatchersPanel::WatchersPanel() : m_isVisible(false)
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
		m_font = ResourcesManager::instance().loadFont("system/font/VeraMono", 12, vec2ui(32, 127));
		if (!m_font.isValid())
			return false;

		m_font->getDesc(m_fontDesc);
		
		return true;
	}

	//---------------------------------------------------------------------------------------------
	bool WatchersPanel::destroy()
	{
		m_font.reset();
		return true;
	}

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::update(float /*dt*/)
	{
		SCOPED_TIME_MEASURER_EX("WatchersPanel update")

		if (!m_isVisible) return;
	}

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::draw(float /*dt*/)
	{
		SCOPED_TIME_MEASURER_EX("WatchersPanel draw")

		if (!m_isVisible) return;

		uint  total  = 0;
		vec2f curPos(0.0f, m_fontDesc.height + 1);
		vec2f offset(0.0f, m_fontDesc.height + m_fontDesc.height * 0.2f);
		std::string line;

		m_font->beginDraw();

		m_font->draw2D(curPos, g_headerColor, "Real-time watchers console.");
		curPos += offset;
	
		for (uint i = 0; i < m_roWatchers.size(); ++i, ++total, curPos += offset)
		{
			const WatcherDesc& desc = m_roWatchers[i];
			line = makeStr(" %2d. %s = %s", total, desc.m_name.c_str(), desc.m_watcher->get().c_str());
			m_font->draw2D(curPos, g_roColor, line);
		}

		for (uint i = 0; i < m_rwWatchers.size(); ++i, ++total, curPos += offset)
		{
			const WatcherDesc& desc = m_rwWatchers[i];
			line = makeStr(" %d %s = %s", total, desc.m_name.c_str(), desc.m_watcher->get().c_str());
			m_font->draw2D(curPos, g_rwColor, line);
		}

		m_font->endDraw();
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

	//---------------------------------------------------------------------------------------------
	void WatchersPanel::_setupRender()
	{
		// ToDo:
	}

} // brUGE