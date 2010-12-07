#pragma once

#include <cassert>

namespace brUGE
{
namespace utils
{

	// Class for reference counting of inherited class.
	//----------------------------------------------------------------------------------------------
	class RefCount
	{
	public:
		RefCount() : m_count(0)
		{
		}

		RefCount(const RefCount&) : m_count(0)
		{
		}

		RefCount& operator = (const RefCount&)
		{
			return *this;
		}

		virtual ~RefCount()
		{
			assert(m_count == 0);
		}

		void addRef() const
		{
			++m_count;
		}

		void releaseRef() const
		{
			if (--m_count == 0) delete this;
		}

	private:
		mutable unsigned long m_count;
	};


	// Smart pointer with reference counting.
	//----------------------------------------------------------------------------------------------
	template <class Type>
	class Ptr
	{
	public:
		Ptr(Type* p = 0) : m_object(p)
		{
			_addRef();
		}

		Ptr(const Ptr<Type>& other) : m_object(0)
		{
			_copy(other);
		}

		~Ptr()
		{
			_release();
		}
		
		Ptr<Type>& operator = (const Type& ptr)
		{
			_release();
			m_object = &ptr;
			return *this;
		}

		Ptr<Type>& operator = (const Ptr<Type>& other)
		{
			_copy(other);
			return *this;
		}
		
		bool operator == (const Ptr<Type>& ptr) const
		{
			return (m_object == ptr.m_object);
		}

		Type* operator -> () const
		{
			return m_object;
		}

		Type& operator * () const
		{
			return *m_object;
		}

		bool operator ! () const
		{
			return !isValid();
		}

		Type* get() const
		{
			return m_object;
		}

		bool isValid() const
		{
			return (m_object != 0);
		}

		operator bool () const
		{
			return isValid();
		}
		
		void reset() const
		{
			_release();
		}
		
		// perform type conversion at down inheritance hierarchy with checking of validity.
		template <class Y>
		inline Ptr<Y> castDown() const;
		
		// perform implicit conversion.
		template <class Y>
		operator Ptr<Y> ()
		{
			return Ptr<Y>(m_object);
		}
	
	private:
		inline void _copy(const Ptr<Type>& other, bool b_addRef = true) const
		{
			_release();
			m_object = other.m_object;
			if (b_addRef)
				_addRef();
		}

		inline void _release() const
		{
			if (m_object)
			{
				m_object->releaseRef();
				m_object = 0;
			}
		}

		inline void _addRef() const 
		{
			if (m_object != 0)
			{
				m_object->addRef();
			}
		}

		mutable Type* m_object;
	};

	template<class T>
	template<class Y> Ptr<Y> Ptr<T>::castDown() const
	{
		return Ptr<Y>(dynamic_cast<Y*>(m_object));
	}

} // utils
} // brUGE
