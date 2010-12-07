#include "TimingPanel.h"
#include "Console.h"
#include "render/Color.h"
#include "utils/string_utils.h"
#include "loader/ResourcesManager.h"

using namespace brUGE::render;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const float g_updateInterval	= 0.1f;
	const uint  g_tabSize		    = 2;
	const uint  g_timeOffset		= 35;
	const float g_vertOffsetPercent = 0.05f;

	//-- global to this module constants.
	const Color g_headerColor(1.0f, 1.0f, 0.0f);
	const Color g_curColor	 (1.0f, 1.0f, 1.0f);
	const Color g_rowColor	 (1.0f, 1.0f, 0.0f);

	//---------------------------------------------------------------------------------------------
	inline const char* formatStr(
		std::string& out, uint level, const char* name, float timeInPercent, uint64 time
		)
	{
		uint64 ms = time / 1000;

		std::string offset1(level * g_tabSize, ' ');
		std::string offset2(g_timeOffset - (level * g_tabSize + strlen(name)), ' ');
		std::string offset3((timeInPercent < 100) ? ((timeInPercent < 10) ? 2 : 1) : 0, ' ');
		std::string offset4(((ms < 100) ? ((ms < 10) ? 2 : 1) : 0), '0');

		out = makeStr("%s%s%s %s%.2f%% %s%.3f us \n",
			offset1.c_str(), name, offset2.c_str(), offset3.c_str(),
			timeInPercent, offset4.c_str(), time * 0.001f
			);		

		return out.c_str();
	}


}
//--------------------------------------------------------------------------------------------------
//-- end unnamed namespace.

namespace brUGE
{

	DEFINE_SINGLETON(TimingPanel)

	//---------------------------------------------------------------------------------------------
	TimingPanel::TimingPanel()
		:	m_isVisible(false), m_updateTime(0.0f), m_needForceUpdate(true), m_totalFrameTime(1.0f),
			m_measuresPerUpdate(1), m_root(-1), m_curNode(-1), m_rootMeasurer(1) //-- ToDo:
	{
		//-- insert this node as the first node in list to eliminate unnecessary run-time checking.
		_insertNode(MeasureNode("parent of root", -1));

		m_root    = _insertNode(MeasureNode("frame time", 0));
		m_curNode = m_root;
	}

	//---------------------------------------------------------------------------------------------
	TimingPanel::~TimingPanel()
	{

	}

	//---------------------------------------------------------------------------------------------
	bool TimingPanel::init()
	{
		m_font = ResourcesManager::instance().loadFont("system/font/VeraMono", 12, vec2ui(32, 127));
		if (!m_font.isValid())
			return false;

		m_font->getDesc(m_fontDesc);

		return true;
	}
	
	//---------------------------------------------------------------------------------------------
	bool TimingPanel::destroy()
	{
		m_font.reset();
		return true;
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::start()
	{
		_enable(m_root);
		m_rootMeasurer.start();
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::stop()
	{
		m_rootMeasurer.stop();
		_disable();
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::draw(float /*dt*/)
	{
		if (!m_isVisible) return;

		m_offset = vec2f(0.0f, m_fontDesc.height + 1);
		m_font->beginDraw();

			m_font->draw2D(m_offset, g_headerColor, "Real-time timing console.");
			m_offset.y += 2.0f * (m_fontDesc.height * (1.0f + g_vertOffsetPercent));

			m_font->draw2D(m_offset, g_headerColor, "Stats: %.3f us (%.2f fps).",
				m_totalFrameTime * 0.001f, 1000000.0f / m_totalFrameTime
				);

			_recursiveDraw(m_root);

		m_font->endDraw();
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::update(float dt)
	{
		if (!m_isVisible) return;

		m_updateTime += dt;
		++m_measuresPerUpdate;

		if (m_updateTime >= g_updateInterval || m_needForceUpdate)
		{
			m_totalFrameTime = static_cast<float>(_getNode(m_root).time / m_measuresPerUpdate);

			_recursiveUpdate(m_root, 0);

			//-- reset counters.	
			m_updateTime = 0.0f;
			m_measuresPerUpdate = 0;
			m_needForceUpdate = false;
		}
	}

	//---------------------------------------------------------------------------------------------
	TimingPanel::MeasureNodeID TimingPanel::create(const std::string& name)
	{
		MeasureNodeID nodeID = _insertNode(MeasureNode(name, m_curNode));
		_getNode(m_curNode).childs.push_back(nodeID);
		return nodeID;
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::_recursiveDraw(TimingPanel::MeasureNodeID nodeID)
	{
		const MeasureNode& node = _getNode(nodeID);

		m_offset.y += m_fontDesc.height * (1.0f + g_vertOffsetPercent);
		m_font->draw2D(m_offset, g_rowColor, node.visual.common);

		if (node.visual.showChilds && !node.childs.empty())
		{
			for (uint i = 0; i < node.childs.size(); ++i)
			{
				_recursiveDraw(node.childs[i]);
			}

			m_offset.y += m_fontDesc.height * (1.0f + g_vertOffsetPercent);
			m_font->draw2D(m_offset, g_rowColor, node.visual.remainder);
		}
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::_recursiveUpdate(TimingPanel::MeasureNodeID nodeID, uint level)
	{
		MeasureNode& node		   = _getNode(nodeID);
		const uint64 time		   = node.time / m_measuresPerUpdate;
		const uint64 remainderTime = node.remainderTime / m_measuresPerUpdate;

		formatStr(
			node.visual.common, level, node.name.c_str(),
			(time / m_totalFrameTime) * 100.0f, time
			);

		if (!node.childs.empty())
		{
			for (uint i = 0; i < node.childs.size(); ++i)
			{
				_recursiveUpdate(node.childs[i], level + 1);
			}

			formatStr(
				node.visual.remainder, level + 1, "<remainder>",
				(remainderTime / m_totalFrameTime) * 100.0f, remainderTime
				);
		}

		//-- reset timers of the node.
		node.time = 0;
		node.remainderTime = 0;
	}

} // brUGE