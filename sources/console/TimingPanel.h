#pragma once

#include "prerequisites.h"
#include "utils/Singleton.h"
#include "utils/Timer.h"
#include "render/Font.h"
#include <string>
#include <vector>

namespace brUGE
{
	//---------------------------------------------------------------------------------------------
	class TimingPanel : public utils::Singleton<TimingPanel>
	{
	public:

		//-----------------------------------------------------------------------------------------
		typedef int MeasureNodeID;
		typedef std::vector<MeasureNodeID> MeasureNodeIDs;

		//-- describes a node of measurement system. 
		struct MeasureNode
		{
			MeasureNode(const std::string& name_, MeasureNodeID parent_)
				: name(name_), time(0), remainderTime(0), parent(parent_) { }

			uint64		   time; //-- in microseconds(us)
			int64		   remainderTime;
			MeasureNodeID  parent;	
			MeasureNodeIDs childs;
			std::string	   name;

			struct Visual
			{
				Visual() : showChilds(true) { }

				bool		showChilds;
				std::string common;
				std::string remainder;
			};

			Visual		   visual;
		};

		//-- Describes an unique time measurer, it grabs timing between two calls of its function
		//-- start() <-> stop();
		class TimeMeasurer
		{
		public:
			TimeMeasurer(MeasureNodeID node) : m_node(node) { }
			~TimeMeasurer() { }

			inline void start()
			{
				utils::Timer& timer = utils::Timer::instance();
				TimingPanel& tmm	= TimingPanel::instance();

				tmm._enable(m_node);
				m_startTime = timer.timeInUS();
			}

			inline void stop()
			{
				utils::Timer& timer		= utils::Timer::instance();
				TimingPanel& tmm	    = TimingPanel::instance();
				MeasureNode& node	    = tmm._getNode(m_node);
				MeasureNode& parentNode = tmm._getNode(node.parent);
				const uint64 delta		= timer.timeInUS() - m_startTime;

				node.time += delta;
				node.remainderTime += delta;
				parentNode.remainderTime -= delta;
				tmm._disable();
			}

		private:
			uint64		  m_startTime;
			MeasureNodeID m_node;

		private:
			TimeMeasurer(const TimeMeasurer&);
			TimeMeasurer& operator = (const TimeMeasurer&);
		};

		//-- Provides useful extension of TimeMeasurer, which gives us opportunity to don't care about
		//-- calling method start() and stop(). They will be called automatically inside the constructor
		//-- and the destructor correspondingly of this object.
		class ScopeTimeMeasurer : public TimeMeasurer
		{
		public:
			inline ScopeTimeMeasurer(MeasureNodeID nodeID) : TimeMeasurer(nodeID)	{ start(); }
			inline ~ScopeTimeMeasurer() { stop(); }
		};
		
		//-- create new node with desired name.
		MeasureNodeID create(const std::string& name);

	public:
		TimingPanel();
		~TimingPanel();

		bool init();
		bool destroy();
		
		//-- between calls of this functions performed the time measurement.
		void start();
		void stop ();

		bool visible() const	{ return m_isVisible; }
		void visible(bool flag) { m_isVisible = flag; m_needForceUpdate = flag; }

		void draw  (float dt);
		void update(float dt);

	private:

		inline void				_enable		(MeasureNodeID nodeID)		{ m_curNode = nodeID; }
		inline void				_disable	()							{ m_curNode = _getNode(m_curNode).parent; }
		inline MeasureNodeID	_insertNode	(const MeasureNode& node)	{ m_nodes.push_back(node); return m_nodes.size() - 1; }
		inline MeasureNode&		_getNode	(MeasureNodeID nodeID)		{ return m_nodes[nodeID]; } 	

		void _recursiveDraw(MeasureNodeID nodeID);
		void _recursiveUpdate(MeasureNodeID nodeID, uint level = 0);

	private:
		typedef std::vector<MeasureNode> MeasureNodes;

		bool				m_isVisible;
		bool				m_needForceUpdate;
		vec2f				m_offset;
		float				m_totalFrameTime;
		uint				m_measuresPerUpdate;
		TimeMeasurer		m_rootMeasurer;
		float				m_updateTime;
		MeasureNodeID		m_root;
		MeasureNodeID		m_curNode;
		MeasureNodes		m_nodes;

		Ptr<render::Font>	m_font;
		render::Font::Desc	m_fontDesc;
	};

} // brUGE

#define SCOPED_TIME_MEASURER_EX(name) \
	static brUGE::TimingPanel::MeasureNodeID _TMN_ =  brUGE::TimingPanel::instance().create(name); \
	 brUGE::TimingPanel::ScopeTimeMeasurer _CTM_(_TMN_);

#define SCOPED_TIME_MEASURER() \
	static brUGE::TimingPanel::MeasureNodeID _TMN_ =  brUGE::TimingPanel::instance().create(__FUNCTION__); \
	 brUGE::TimingPanel::ScopeTimeMeasurer _CTM_(_TMN_);

#define TIME_MEASURER_START_EX(name) \
	static brUGE::TimingPanel::MeasureNodeID _TMN_ =  brUGE::TimingPanel::instance().create(name); \
	 brUGE::TimingPanel::TimeMeasurer _TM_(_TMN_); \
	_TM_.start();

#define TIME_MEASURER_STOP_EX() \
	_TM_.stop();

#define TIME_MEASURER_START() \
	static brUGE::TimingPanel::MeasureNodeID _TMN_ =  brUGE::TimingPanel::instance().create(__FUNCTION__); \
	 brUGE::TimingPanel::TimeMeasurer _TM_(_TMN_); \
	_TM_.start();

#define TIME_MEASURER_STOP() \
	_TM_.stop();
