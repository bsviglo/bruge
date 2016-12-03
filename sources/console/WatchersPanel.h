#pragma once

#include "prerequisites.hpp"
#include "utils/Singleton.h"
#include "utils/ArgParser.h"

#include <string>
#include <vector>

namespace brUGE
{
	
	//-- Base class of watcher. It give us opportunity to watch and change variable value
	//-- in real time.
	//----------------------------------------------------------------------------------------------
	class IWatcher
	{
	public:
		~IWatcher() { }

		virtual std::string	get() const			 = 0;				
		virtual void set(const std::string& str) = 0;
	};


	//----------------------------------------------------------------------------------------------
	template<typename T>
	class RawWatcher : public IWatcher
	{
	public:
		RawWatcher(T* value) : m_value(value) { }
		
		virtual std::string	get() const					{ return utils::parseFrom<T>(*m_value); }		
		virtual void		set(const std::string& str)	{ *m_value = utils::parseTo<T>(str); }

	private:
		T* m_value;
	};


	//----------------------------------------------------------------------------------------------
	template<typename T, typename OBJ>
	class RawObjWatcher : public IWatcher
	{
	public:
		RawObjWatcher(OBJ* obj, T OBJ::* value)
			: m_value(value), m_object(obj) { }

		virtual std::string	get() const					{ return utils::parseFrom<T>(m_object->*m_value); }		
		virtual void		set(const std::string& str)	{ m_object->*m_value = utils::parseTo<T>(str); }

	private:
		typedef T OBJ::* ObjValuePtrType;

		ObjValuePtrType m_value;
		OBJ*			m_object;
	};


	//----------------------------------------------------------------------------------------------
	template<typename T>
	class AccessorWatcher : public IWatcher
	{
	public:
		typedef T (*getter)();
		typedef void (*setter)(const T&);

		typedef std::pair<getter, setter> AccessorPair;

	public:
		AccessorWatcher(getter getFunc, setter setFunc)
			: m_accessors(std::make_pair(getFunc, setFunc)) { }

		virtual std::string	get() const					{ return utils::parseFrom<T>((*m_accessors.first)()); }		
		virtual void		set(const std::string& str)	{ (*m_accessors.second)(utils::parseTo<T>(str)); }

	private:
		AccessorPair m_accessors;
	};


	//----------------------------------------------------------------------------------------------
	template<typename T, typename OBJ>
	class AccessorObjWatcher : public IWatcher
	{
	public:
		typedef T (OBJ::* getter)() const;
		typedef void (OBJ::* setter)(const T&);

		typedef std::pair<getter, setter> AccessorPair;

	public:
		AccessorObjWatcher(OBJ* obj, getter getFunc, setter setFunc)
			: m_object(obj), m_accessors(std::make_pair(getFunc, setFunc)) { }

		virtual std::string	get() const					{ return utils::parseFrom<T>((m_object->*m_accessors.first)()); }		
		virtual void		set(const std::string& str)	{ (m_object->*m_accessors.second)(utils::parseTo<T>(str)); }

	private:
		OBJ*		 m_object;
		AccessorPair m_accessors;
	};
	

	//
	//---------------------------------------------------------------------------------------------
	class WatchersPanel : public utils::Singleton<WatchersPanel>, public NonCopyable
	{
	public:
		enum EWatcherAccess
		{
			ACCESS_READ_ONLY,
			ACCESS_READ_WRITE
		};

	public:
		WatchersPanel();
		~WatchersPanel();

		bool init();
		
		bool visible() const	{ return m_isVisible; }
		void visible(bool flag) { m_isVisible = flag; }

		void update(float dt);
		void visualize();

		void registerWatcher(const std::string& name, EWatcherAccess access, IWatcher* watcher);

	private:

		struct WatcherDesc
		{
			IWatcher*	m_watcher;
			std::string	m_name;
			std::string	m_desc;
		};

		std::vector<WatcherDesc> m_roWatchers;
		std::vector<WatcherDesc> m_rwWatchers;
		bool					 m_isVisible;
		int						 m_scroll;
	};

} // brUGE

/*
template<typename Type, typename ObjType>
brUGE::RawObjWatcher<Type, ObjType>* deduceArgs(ObjType* obj, Type ObjType::* value)
{
	return new brUGE::RawObjWatcher<Type, ObjType>(obj, value);
}
*/

#define _W_REG_ brUGE::WatchersPanel::instance().registerWatcher

#define REGISTER_RO_WATCHER(name, type, variable) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_ONLY, new RawWatcher<type>(&variable));
#define REGISTER_RW_WATCHER(name, type, variable) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_WRITE, new RawWatcher<type> obj(&variable));

#define REGISTER_RO_WATCHER_EX(name, type, getter, setter) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_ONLY, new AccessorWatcher<type>(getter, setter));
#define REGISTER_RW_WATCHER_EX(name, type, getter, setter) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_WRITE, new AccessorWatcher<type>(getter, setter));

#define REGISTER_RO_MEMBER_WATCHER(name, type, objType, member) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_ONLY, new RawObjWatcher<type, objType>(this, &objType::member));
#define REGISTER_RW_MEMBER_WATCHER(name, type, objType, member) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_WRITE, new RawObjWatcher<type, objType>(this, &objType::member));

#define REGISTER_RO_MEMBER_WATCHER_EX(name, type, objType, getter, setter) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_ONLY, new AccessorObjWatcher<type, objType>(this, getter, setter));
#define REGISTER_RW_MEMBER_WATCHER_EX(name, type, objType, getter, setter) \
	_W_REG_(name, WatchersPanel::ACCESS_READ_WRITE, new AccessorObjWatcher<type, objType>(this, getter, setter));

//#undef _W_REG_