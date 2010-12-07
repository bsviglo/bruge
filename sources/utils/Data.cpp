#include "Data.h"
#include "assert.h"

namespace brUGE
{
namespace utils
{

	//------------------------------------------
	Data::Data(uint len, bool allowBufferGrowing/* = false*/)
		:	m_bits(NULL),
			m_pos(0),
			m_length(len),
			m_capacity(len),
			m_allowBuffGrowing(allowBufferGrowing)
	{
		assert(len > 0 && "invalid configuration parameters.");
		m_bits = new byte[len];
	}

	//------------------------------------------
	Data::Data(void* ptr, uint len)
		:	m_bits(NULL),
			m_pos(0),
			m_length(len),
			m_capacity(len),
			m_allowBuffGrowing(false)
	{
		assert((ptr != NULL && len > 0) && "invalid configuration parameters.");
		m_bits = static_cast<byte*>(ptr);
	}

	//------------------------------------------
	Data::~Data()
	{
		delete[] m_bits;
		m_bits = NULL;
	}

	//------------------------------------------
	uint Data::writeBytes(const byte* ptr, uint len)
	{
		if (!m_allowBuffGrowing)
		{
			if (m_pos >= m_length)
				return 0;

			if (m_pos + len > m_length)
				len = m_length - m_pos;
		}
		else
		{
			if (m_pos + len > m_capacity)
			{
				uint needToAdd = len - (m_capacity - m_pos);
				reserve((needToAdd > m_capacity * 2) ? needToAdd : m_capacity * 2);
			}
		}

		memcpy(m_bits + m_pos, ptr, len);		
		m_pos += len;
	
		//-- accumulate new length.
		if (m_pos > m_length)
			m_length = m_pos;

		return len;
	}

	//------------------------------------------
	uint Data::readBytes(byte* ptr, uint len) const
	{
		if (m_pos >= m_length)
			return 0;

		if (m_pos + len > m_length)
			len = m_length - m_pos;

		memcpy(ptr, m_bits + m_pos, len);
		m_pos += len;

		return len;
	}
	
	//------------------------------------------
	bool Data::getAsString(std::string& str) const
	{
		if (m_pos >= m_length)
			return false;

		str = "";

		while(m_pos < m_length)
			str += m_bits[m_pos++];

		return true;
	}

	//------------------------------------------
	bool Data::getString(std::string& str, char term) const
	{
		if (m_pos >= m_length)
			return false;

		str = "";

		while(m_pos < m_length && m_bits[m_pos] != term)
			str += m_bits[m_pos++];

		if (m_pos < m_length && m_bits[m_pos] == term)
			++m_pos;

		//-- skin OA part of line terminator (0D,0A)
		if (term == '\r' && m_pos + 1 < m_length && m_bits[m_pos+1] == '\n')
			++m_pos;

		return true;
	}

} // utils
} // brUGE

