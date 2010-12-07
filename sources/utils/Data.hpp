#pragma once

#include "prerequisites.h"
#include "Ptr.h"
#include <vector>
#include <string>
#include <cassert>

namespace brUGE
{
namespace utils
{
	
	//-- Fixed sized, read only, non-copyable buffer with optionally memory control. 
	//---------------------------------------------------------------------------------------------
	class ROData : public RefCount
	{
	public:
		//-- Note: isOwner - interprets assigned pointer to need memory deallocation in the destructor.
		ROData(byte* ptr, uint len, bool isOwner = true)
			: m_bytes(ptr), m_length(len), m_pos(0), m_isOwner(isOwner)
		{
			assert(ptr != 0 && len != 0);
		}
		~ROData()
		{
			if (m_isOwner)
			{
				delete [] m_bytes;
				m_bytes = 0;
			}
		}

		//-----------------------------------------------------------------------------------------
		inline bool isOk()				  const	{ return m_bytes != 0; }
		inline bool	isEmpty()			  const	{ return m_pos >= m_length; }
		inline uint	length()			  const	{ return m_length; }
		inline uint	pos()				  const	{ return m_pos; }
		inline const char* c_str()		  const	{ return reinterpret_cast<char*>(m_bytes); }
		inline const void* ptr()		  const	{ return m_bytes + m_pos; }
		inline const void* ptr(uint offs) const	{ return (offs >= m_length) ? 0 : m_bytes + offs; }

		//-----------------------------------------------------------------------------------------
		template<class T>
		inline bool read(T& data) const
		{
			if ((m_pos + sizeof(T)) <= m_length)
			{
				memcpy(&data, m_bytes + m_pos, sizeof(T));
				m_pos += sizeof(T);
				return true;
			}
			else
			{
				return false;				
			}
		}

		//-----------------------------------------------------------------------------------------
		inline uint readBytes(byte* ptr, uint len) const
		{
			assert(ptr != 0 && len != 0);

			if (m_pos >= m_length)
				return 0;

			if (m_pos + len > m_length)
				len = m_length - m_pos;

			memcpy(ptr, m_bytes + m_pos, len);
			m_pos += len;

			return len;
		}

		//-----------------------------------------------------------------------------------------
		inline uint seek(int delta) const
		{
			m_pos += delta;

			if (m_pos > m_length)	m_pos = m_length;
			if (m_pos < 0)			m_pos = 0;

			return m_pos;
		}

		//-----------------------------------------------------------------------------------------
		inline uint seekAbs(uint offs) const
		{
			m_pos = (offs > m_length) ? m_length : offs;
			return m_pos;
		}

		//-----------------------------------------------------------------------------------------
		bool getAsString(std::string& str) const
		{
			if (m_pos >= m_length)
				return false;

			str.assign(c_str() + m_pos, m_length - m_pos);
			m_pos = m_length;

			return true;
		}
		
		//-----------------------------------------------------------------------------------------
		bool getString(std::string& str, char term) const
		{
			if (m_pos >= m_length)
				return false;

			str = "";

			for (; m_pos < m_length && m_bytes[m_pos] != term; ++m_pos)
				str += m_bytes[m_pos];

			if (m_pos < m_length && m_bytes[m_pos] == term)
				++m_pos;

			//-- skip OA part of line terminator (0D,0A)
			if (term == '\r' && m_pos + 1 < m_length && m_bytes[m_pos+1] == '\n')
				++m_pos;

			return true;
		}

	protected:
		byte* m_bytes;
		bool  m_isOwner;
		mutable uint m_length;
		mutable uint m_pos;

	private:
		ROData(const ROData&);
		ROData& operator = (const ROData&);
	};


	//-- Variable sized, write only, non-copyable buffer with the own memory control.
	//-- Note: This class based on the std::vector STL container.
	//---------------------------------------------------------------------------------------------
	class WOData : public RefCount
	{
	public:
		WOData() : m_pos(0)	{ }
		WOData(uint reserveSize) : m_pos(0)	{ reserve(reserveSize); }
		~WOData() {	}

		//-----------------------------------------------------------------------------------------
		inline uint	 length()			  const	{ return m_bytes.size(); }
		inline uint	 pos()				  const	{ return m_pos; }
		inline const char* c_str()		  const	{ return reinterpret_cast<const char*>(&m_bytes[0]); }
		inline const void* ptr()		  const	{ return &m_bytes[0] + m_pos; }
		inline const void* ptr(uint offs) const	{ return (offs >= length()) ? 0 : &m_bytes[0] + offs; }
		
		//-----------------------------------------------------------------------------------------
		inline byte*	   bytes()			{ return &m_bytes[0]; }
		inline const byte* bytes() const	{ return &m_bytes[0]; }

		//-----------------------------------------------------------------------------------------
		template<class T>
		inline bool write(const T& data)
		{
			const byte* iData = reinterpret_cast<const byte*>(&data);
			m_bytes.insert(m_bytes.begin() + m_pos, iData, iData + sizeof(T));
			m_pos += sizeof(T);

			return true;
		}

		//-- write data range to the buffer.
		//-----------------------------------------------------------------------------------------
		uint writeBytes(const byte* ptr, uint len)
		{
			assert(ptr != 0 && len != 0);

			m_bytes.insert(m_bytes.begin() + m_pos, ptr, ptr + len);
			m_pos += len;

			return len;
		}

		//-----------------------------------------------------------------------------------------
		inline void reserve(uint reserveSize)
		{
			m_bytes.reserve(reserveSize);
		}

		//-----------------------------------------------------------------------------------------
		inline void truncate()
		{
			std::vector<byte> tmp(m_bytes);
			tmp.swap(m_bytes);
		}

		//-----------------------------------------------------------------------------------------
		inline uint seek(int delta) const
		{
			m_pos += delta;

			if (m_pos > length())	m_pos = length();
			if (m_pos < 0)			m_pos = 0;

			return m_pos;
		}

		//-----------------------------------------------------------------------------------------
		inline uint seekAbs(uint offs) const
		{
			m_pos = (offs <= length()) ? offs : length();
			return m_pos;
		}

	private:
		std::vector<byte> m_bytes;
		mutable uint m_pos;

	private:
		WOData(const WOData&);
		WOData& operator = (const WOData&);
	};


	//-- Fixed sized, read write, non-copyable buffer with optionally memory control. 
	//---------------------------------------------------------------------------------------------
	class RWData : public ROData
	{
	public:
		RWData(byte* ptr, uint len, bool isOwner = true) : ROData(ptr, len, isOwner) { }
		~RWData() {	}

		//-----------------------------------------------------------------------------------------
		inline void* ptr()		    { return m_bytes + m_pos; }
		inline void* ptr(uint offs) { return (offs >= m_length) ? 0 : m_bytes + offs; }

		//-----------------------------------------------------------------------------------------
		template<class T>
		inline bool write(const T& data)
		{
			if ((m_pos + sizeof(T)) <= m_length)
			{
				memcpy(m_bytes + m_pos, &data, sizeof(T));
				m_pos += sizeof(T);
				return true;
			}
			else
			{
				return false;				
			}
		}

		//-----------------------------------------------------------------------------------------
		uint writeBytes(const byte* ptr, uint len)
		{
			assert(ptr != 0 && len != 0);

			if (m_pos >= m_length)
				return 0;

			if (m_pos + len > m_length)
				len = m_length - m_pos;

			memcpy(m_bytes + m_pos, ptr, len);
			m_pos += len;

			return len;
		}
	};

} // utils
} // brUGE