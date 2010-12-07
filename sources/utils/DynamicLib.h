#pragma once

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

namespace brUGE
{
namespace utils
{
	// Wrapper around dynamic library.
	//----------------------------------------------------------------------------------------------
	class DynamicLib
	{
	public:
		DynamicLib();
		~DynamicLib();

		bool load(const std::string& libName);
		bool free();

		void* getSymbol(const char* symbol); 
		
		bool isLoaded() const { return m_module != NULL; }
		const std::string& getName() const { return m_moduleName; }
		std::string lastError();

	private:
		HMODULE m_module;
		std::string m_moduleName;
	};

} // utils
} // brUGE

