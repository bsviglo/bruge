#pragma once

namespace brUGE
{
namespace utils
{
	// Com object wrapper.
	//----------------------------------------------------------------------------------------------
	template<class COMType>
	class ComPtr
	{
	public:
		ComPtr(COMType* ptr = 0) : m_object(ptr)
		{

		}

		ComPtr(const ComPtr<COMType>& other) : m_object(0)
		{
			_copy(other);
		}

		~ComPtr()
		{
			_release();
		}

		ComPtr<COMType>& operator = (const ComPtr<COMType>& other)
		{
			if (m_object != other.m_object)
				_copy(other);
			return *this;
		}

		// Сравнение двух смарт поинтеров.
		bool operator == (const ComPtr<COMType>& ptr) const
		{
			return (m_object == ptr.m_object);
		}

		COMType* operator -> () const
		{
			return m_object;
		}

		COMType& operator * () const
		{
			return *m_object;
		}

		COMType** operator & () const
		{
			return &m_object;
		}

		bool operator ! () const
		{
			return !isValid();
		}

		COMType* get() const
		{
			return m_object;
		}

		// Освободит умный указатель он владения объектом.
		COMType* release() const
		{
			COMType* ptr = m_object;
			m_object = NULL;
			return ptr;
		}

		// release com object.
		void reset() const
		{
			_release();
		}
		
		operator void*() const
		{
			return m_object;
		}
		
		operator COMType* () const
		{
			return m_object;
		}

		bool isValid() const
		{
			return (m_object != NULL);
		}

		operator bool () const
		{
			return isValid();
		}

	private:
		inline void _copy(const ComPtr<COMType>& other, bool b_addRef = true) const
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
				m_object->Release();
				m_object = NULL;
			}
		}

		inline void _addRef() const 
		{
			if (m_object != NULL)
			{
				m_object->AddRef();
			}
		}

		mutable COMType* m_object;
	};

} // utils	
} // brUGE
