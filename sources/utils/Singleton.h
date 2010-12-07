#pragma once

#include <cassert>

//
// Template class for creating non-copyable single-instance global classes.
//--------------------------------------------------------------------------------------------------
namespace brUGE
{
namespace utils
{

	template<class T>
	class Singleton
	{
	public:
		Singleton()
		{
			assert(!m_instance && "instance of class is already created.");
			m_instance = static_cast<T*>(this);
		}

		~Singleton()
		{
			assert(m_instance && "instance of class already destroyed.");
			m_instance = 0;
		}

		static T &instance()
		{
			assert(m_instance && "trying to get instance of the singleton class before its creation.");
			return *m_instance;
		}

	protected:
		static T* m_instance;

	private:
		// make non-copyable.
		Singleton(const Singleton&);
		Singleton& operator = (const Singleton&);
	};

} // utils

#define DEFINE_SINGLETON(className)\
	template<> className *utils::Singleton< className >::m_instance = 0;

} // brUGE
