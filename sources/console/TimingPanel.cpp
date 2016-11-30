#include "TimingPanel.h"
#include "Console.h"
#include "utils/string_utils.h"
#include "loader/ResourcesManager.h"
#include "gui/imgui.h"

using namespace brUGE::render;
using namespace brUGE::utils;

//-- start unnamed namespace.
//--------------------------------------------------------------------------------------------------
namespace
{
	const float g_updateInterval = 0.2f;

	//---------------------------------------------------------------------------------------------
	inline void formatStr(std::string& out, float timeInPercent, uint64 time)
	{
		float ms = time * 1000;

		std::string offset1((timeInPercent < 100) ? ((timeInPercent < 10) ? 2 : 1) : 0, ' ');
		std::string offset2(((ms < 100) ? ((ms < 10) ? 2 : 1) : 0), '0');

		out = makeStr("%s%.2f%% %s%.3f us", offset1.c_str(), timeInPercent, offset2.c_str(), time);		
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
			m_measuresPerUpdate(1), m_root(-1), m_curNode(-1), m_rootMeasurer(1), //-- ToDo:
			m_scroll(0)
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
	void TimingPanel::visualize()
	{
		if (!m_isVisible) return;

		uint height = rs().screenRes().height;

		imguiBeginScrollArea("Real-time timing console", 10, height*0.4, 400, height*0.6-10, &m_scroll);
		{
			imguiLabel(
				makeStr("Stats: %.3f us (%.2f fps).",
				m_totalFrameTime * 1000.0f, 1.0f / m_totalFrameTime).c_str()
				);

			_recursiveVisualize(m_root);
		}
		imguiEndScrollArea();
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
	void TimingPanel::_recursiveVisualize(TimingPanel::MeasureNodeID nodeID)
	{
		MeasureNode& node = _getNode(nodeID);

		if (node.childs.empty())
		{
			imguiKeyValue(node.name.c_str(), node.visual.common.c_str());
		}
		else
		{
			imguiCollapse(node.name.c_str(), node.visual.common.c_str(), &node.visual.showChilds);
			if (node.visual.showChilds)
			{
				imguiIndent();
				for (uint i = 0; i < node.childs.size(); ++i)
				{
					_recursiveVisualize(node.childs[i]);
				}
				imguiKeyValue("<remainder>", node.visual.remainder.c_str());
				imguiUnindent();
			}
		}
	}

	//---------------------------------------------------------------------------------------------
	void TimingPanel::_recursiveUpdate(TimingPanel::MeasureNodeID nodeID, uint level)
	{
		MeasureNode& node		   = _getNode(nodeID);
		const float time		   = node.time / m_measuresPerUpdate;
		const float remainderTime = node.remainderTime / m_measuresPerUpdate;

		formatStr(node.visual.common, (time / m_totalFrameTime) * 100.0f, time);

		if (!node.childs.empty())
		{
			for (uint i = 0; i < node.childs.size(); ++i)
			{
				_recursiveUpdate(node.childs[i], level + 1);
			}

			formatStr(
				node.visual.remainder, (remainderTime / m_totalFrameTime) * 100.0f, remainderTime
				);
		}

		//-- reset timers of the node.
		node.time = 0;
		node.remainderTime = 0;
	}

} // brUGE