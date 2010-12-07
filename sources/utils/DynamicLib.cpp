#include "DynamicLib.h"
#include "assert.h"

namespace brUGE
{
namespace utils 
{
	//------------------------------------------
	DynamicLib::DynamicLib() : m_module(NULL)
	{

	}
	
	//------------------------------------------
	DynamicLib::~DynamicLib()
	{
		// before delete wrapper we will need to free loaded library.
		assert(m_module == NULL);
	}

	//------------------------------------------
	bool DynamicLib::load(const std::string& libName)
	{
		m_moduleName = libName;
		m_module = LoadLibrary(m_moduleName.c_str());
		return (m_module != NULL);
	}

	//------------------------------------------
	bool DynamicLib::free()
	{
		if (!FreeLibrary(m_module))
		{
			m_module = NULL;
			return false;
		}
		m_module = NULL;
		return true;		
	}

	//------------------------------------------
	void* DynamicLib::getSymbol(const char* symbol)
	{
		return (void*)GetProcAddress(m_module, symbol);
	}
	
	//------------------------------------------
	std::string DynamicLib::lastError() 
	{
		LPVOID lpMsgBuf; 
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, 
			GetLastError(), 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 
			0, 
			NULL 
			); 
		std::string ret = (char*)lpMsgBuf;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		return ret;
	}

} // utils
} // brUGE