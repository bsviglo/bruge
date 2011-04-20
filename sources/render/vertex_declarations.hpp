#pragma once

#include "prerequisites.hpp"
#include "render_common.h"
#include <vector>
#include <map>

namespace brUGE
{
namespace render
{

	//-- Provide access to the all vertex formats used in the engine. Each individual format is
	//-- declared in the xml file and loaded during engine's loading time.
	//----------------------------------------------------------------------------------------------
	class VertexDeclarations : public NonCopyable
	{
	public:
		VertexDeclarations();
		~VertexDeclarations();

		bool			init();
		VertexLayoutID	get(const char* name, const IShader& shader);

	private:
		typedef std::vector<VertexDesc>			  VertexDclr;
		typedef std::map<std::string, VertexDclr> VertexDclrMap;

		VertexDclrMap m_map;
	};

} //-- render
} //-- brUGE