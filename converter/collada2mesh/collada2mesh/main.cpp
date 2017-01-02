#include "collada2mesh.hpp"
#include "utils/Data.hpp"
#include "SDL/SDL_timer.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace brUGE;
using namespace brUGE::utils;

//--------------------------------------------------------------------------------------------------
void showUsage()
{
	cout << "Usage: \n";
	cout << "    " << "[-i] - input file name. \n";
	cout << "    " << "[-o] - output file name. \n";
}

//--------------------------------------------------------------------------------------------------
size_t fileSize(const string& file) 
{ 
	ifstream fs; 
	fs.open(file, ios::binary); 

	assert(fs); 
	assert(fs.is_open()); 

	fs.seekg(0, ios::beg); 
	const ios::pos_type start_pos = fs.tellg(); 

	fs.seekg(0, ios::end); 
	const ios::pos_type end_pos = fs.tellg(); 

	const size_t ret_filesize (static_cast<size_t>(end_pos - start_pos)); 

	fs.close(); 
	assert(!fs.is_open()); 

	return ret_filesize; 
}

//--------------------------------------------------------------------------------------------------
std::unique_ptr<ROData> readFile(const std::string& file)
{
	std::unique_ptr<ROData> out;
	
	ifstream iFile;
	iFile.open(file, ios_base::in | ios_base::binary);
	if (iFile.is_open())
	{
		//-- read file in memory based on the file size.
		uint32 size = fileSize(file);
		byte* bytes = new byte[size];

		iFile.read((char*)bytes, size);
		if (iFile.gcount() != size)
		{
			delete [] bytes;
			return out;
		}
		
		//-- adjust holder size and make it owner of the earlier allocated memory. 
		out.reset(new ROData(bytes, size));
		
		//-- close file.
		iFile.close();
	}

	return out;
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
void validate(const ROData& iData)
{
	//-- check header.
	{
		StaticMeshFormat::Header iHeader;
		iData.read(iHeader);
		if (std::string(iHeader.m_format) != "static_mesh")
		{
			return;
		}
	}

	//-- load common info.
	StaticMeshFormat::Info iInfo;
	iData.read(iInfo);

	//-- iterate over the whole set of sub-meshes.
	for (uint i = 0; i < iInfo.m_numSubMeshes; ++i)
	{
		StaticMeshFormat::SubInfo iSubInfo;
		iData.read(iSubInfo);

		//-- read each individual vertex stream.
		for (uint j = 0; j < iSubInfo.m_numVertexStreams; ++j)
		{
			StaticMeshFormat::VertexStream iStream;
			iData.read(iStream);

			//-- read data for vertex buffer.
			std::vector<byte> bytes(iStream.m_elemSize * iSubInfo.m_numVertices);
			iData.readBytes(&bytes[0], iStream.m_elemSize * iSubInfo.m_numVertices);
		}

		//-- read data for index buffer.
		std::vector<uint16> indices(iSubInfo.m_numIndices);
		iData.readBytes(&indices[0], sizeof(uint16) * iSubInfo.m_numIndices);
	}
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
	
	//-- try to open input file.
	std::unique_ptr<ROData> iData = readFile(iFile);
	if (iData)
	{
		try
		{
			// calculate engine tick time.
			uint64 startTime = SDL_GetPerformanceCounter();

			WOData oData;

			switch (type)
			{
			case CONVERTING_TYPE_STATIC:	{	collada2staticmesh(*iData.get(), oData); break;		}
			case CONVERTING_TYPE_SKINNED:	{	collada2skinnedmesh(*iData.get(), oData); break;	}
			case CONVERTING_TYPE_ANIMATION:	{	collada2animation(*iData.get(), oData); break;		}
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
	}

	return 1;
}