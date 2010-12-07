#pragma once

#include "prerequisites.h"
#include "Ptr.h"
#include <string>

namespace brUGE
{
namespace utils
{
	//-- Non-copyable class gives access to the file data or memory section in low-level form.
	//-- It give us opportunity to read/write to memory in very flexible style and freed us from
	//-- necessity to manage memory resource.
	//-- Data may exists in two states, with allowing internal buffer grow and without grow. E.g.
	//-- this give us to use this class in both cases as a memory holder and as a memory allocator.
	//-- That likes a file with the different access flag, read only, write or overwrite.
	//-- Note: After passing to this class pointer of memory, it deletes this pointer in its destructor.
	//---------------------------------------------------------------------------------------------
	class Data : public RefCount
	{
	public:
		Data(uint len, bool allowBufferGrowing = false);
		Data(void* ptr, uint len);
		~Data();
		
		//-----------------------------------------------------------------------------------------
		inline bool isOk()			 const	{ return m_bits != NULL; }
		inline bool	isEmptyForRead() const	{ return m_pos >= m_length; }
		inline bool	isFullToWrite()	 const	{ return m_pos >= m_length; }
		inline uint	length()		 const	{ return m_length; }
		inline uint capacity()		 const  { return m_capacity; }
		inline uint	pos()			 const	{ return m_pos; }
		inline const char* c_str()   const	{ return reinterpret_cast<char*>(m_bits); }
		inline void* ptr()			 const	{ return m_bits + m_pos; }
		inline void* ptr(uint offs)  const	{ return (offs >= m_length) ? NULL : m_bits + offs;	}
		
		//-----------------------------------------------------------------------------------------
		template<class T>
		inline bool read(T& data) const
		{
			if ((m_pos + sizeof(T) - 1) >= m_length)
			{
				return false;
			}
			else
			{
				memcpy(&data, m_bits + m_pos, sizeof(T));
				m_pos += sizeof(T);
				return true;
			}
		}

		//-----------------------------------------------------------------------------------------
		template<class T>
		inline bool write(const T& data)
		{
			if ((m_pos + sizeof(T) - 1) >= m_capacity)
			{
				if (!m_allowBuffGrowing)
				{
					return false;
				}
				else
				{
					uint needToAdd = sizeof(T) - (m_capacity - m_pos);
					reserve((needToAdd > m_capacity * 2) ? needToAdd : m_capacity * 2);
				}
			}

			memcpy(m_bits + m_pos, &data, sizeof(T));
			m_pos += sizeof(T);

			//-- accumulate new length.
			if (m_pos > m_length)
				m_length = m_pos;

			return true;
		}

		//-- read/write data from/to buffer.
		//-----------------------------------------------------------------------------------------
		uint writeBytes(const byte* ptr, uint len);
		uint readBytes (byte* ptr, uint len) const;

		//-----------------------------------------------------------------------------------------
		inline void reserve(uint newSize)
		{
			if (!m_allowBuffGrowing)   return;
			if (newSize <= m_capacity) return;

			byte* newArray = new byte[newSize];
			memcpy(newArray, m_bits, m_length);
			delete [] m_bits;

			m_bits	   = newArray;
			m_capacity = newSize;
		}

		//-----------------------------------------------------------------------------------------
		inline uint seekCur(int delta) const
		{
			m_pos += delta;

			if (m_pos > m_length)	m_pos = m_length;
			if (m_pos < 0)			m_pos = 0;

			return m_pos;
		}

		//-----------------------------------------------------------------------------------------
		inline uint seekAbs(int offs) const
		{
			m_pos = offs;

			if (m_pos > m_length)	m_pos = m_length;
			if (m_pos < 0)			m_pos = 0;

			return m_pos;
		}
		
		//-- return string ended at symbol term.
		//-----------------------------------------------------------------------------------------
		bool getAsString(std::string& str) const;
		bool getString(std::string& str, char term) const;

	private:
		byte* m_bits;
		mutable bool m_allowBuffGrowing;
		mutable uint m_capacity;
		mutable uint m_length;
		mutable uint m_pos;

	private:
		Data(const Data&);
		Data& operator = (const Data&);
	};

} // utils
} // brUGE

