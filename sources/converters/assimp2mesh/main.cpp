#include "prerequisites.hpp"
#include "utils/Data.hpp"
#include "SDL/SDL_timer.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace brUGE;
using namespace brUGE::math;
using namespace brUGE::utils;

namespace brUGE
{
	//--------------------------------------------------------------------------------------------------
	void assimp2staticmesh(const aiScene& scene, WOData& oData);
	//void assimp2skinnedmesh(const aiScene& scene, WOData& oData);
	//void assimp2animation(const aiScene& scene, WOData& oData);
}

//--------------------------------------------------------------------------------------------------
void showUsage()
{
	cout << "Usage: \n";
	cout << "    " << "[-i] - input file name. \n";
	cout << "    " << "[-o] - output file name. \n";
	cout << "    " << "[-t] - type of the conversion static/skinned/animation. \n";
}

//--------------------------------------------------------------------------------------------------
void writeFile(const std::string& file, const WOData& data)
{
	ofstream oFile;
	oFile.open(file.c_str(), ios_base::trunc | ios_base::binary | ios_base::out);
	if (oFile.is_open())
		oFile.write((const char*)data.ptr(0), data.length());
	oFile.close();
}

//--------------------------------------------------------------------------------------------------
enum EConvertingType
{
	CONVERTING_TYPE_STATIC,
	CONVERTING_TYPE_SKINNED,
	CONVERTING_TYPE_ANIMATION
};

//--------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	EConvertingType type = CONVERTING_TYPE_STATIC;

	//-- check initial arguments.
	if (argc < 4)
	{
		showUsage();
		return 1;
	}

	//-- parse parameters.
	std::string iFile, oFile;
	for (int i = 0; i < argc; ++i)
	{
		if (string("-i") == argv[i])
		{
			iFile = argv[++i];
		}
		else if (string("-o") == argv[i])
		{
			oFile = argv[++i];
		}
		else if (string("-t") == argv[i])
		{
			++i;
			if (string("static") == argv[i])
			{
				type = CONVERTING_TYPE_STATIC;
			}
			else if (string("skinned") == argv[i])
			{
				type = CONVERTING_TYPE_SKINNED;
			}
			else if (string("animation") == argv[i])
			{
				type = CONVERTING_TYPE_ANIMATION;
			}
			else
			{
				showUsage();
				return 1;
			}
		}
	}

	if (iFile.empty() || oFile.empty())
	{
		showUsage();
		return 1;
	}
	
	//-- try to convert imported file
	try
	{
		// calculate engine tick time.
		uint64 startTime = SDL_GetPerformanceCounter();

		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(
			iFile, aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality
		);

		if (!scene)
			throw importer.GetErrorString();

		WOData oData;
		 
		switch (type)
		{
		case CONVERTING_TYPE_STATIC:	{	assimp2staticmesh	(*scene, oData); break;		}
		//case CONVERTING_TYPE_SKINNED:	{	assimp2skinnedmesh	(*scene, oData); break;		}
		//case CONVERTING_TYPE_ANIMATION:	{	assimp2animation	(*scene, oData); break;		}
		default:
			return 1;
		}

		writeFile(oFile, oData);

		uint64 endTime = SDL_GetPerformanceCounter();
		double elapsedTime = static_cast<double>(endTime - startTime) / SDL_GetPerformanceFrequency();

		cout << "Input file '" << iFile << "' has been successfully converted for " << elapsedTime << " seconds. \n";
	}
	catch(const char* e)
	{
		cout << "Converting failed: " << e << "\n";
	}
	catch(...)
	{
		cout << "Converting failed: Unspecified error.\n";
	}

	return 1;
}

