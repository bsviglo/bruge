#include "engine/Engine.h"
#include "Editor.hpp"

#ifdef _DEBUG
//-- include memory leak detector.
//#include <vld.h>
#endif

//#include <crtdbg.h>
using namespace brUGE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{	
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Engine engine;
	try
	{
		engine.init(hInstance, new Editor());
		engine.run();
	}
	catch(Exception& e)
	{
		ERROR_MSG(e.getMessage().c_str());
	}
	catch(...)
	{
		ERROR_MSG("An unknown error occurred.");
	}

	return EXIT_SUCCESS;
}