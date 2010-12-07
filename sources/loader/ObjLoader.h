#pragma once

#include "prerequisites.h"

namespace brUGE
{
	namespace utils
	{
		class ROData;
	}

	namespace render
	{
		class Mesh;
	}

	//
	//----------------------------------------------------------------------------------------------
	class ObjLoader
	{
	public:
		Ptr<render::Mesh> load(const utils::ROData& data);
	};

} // end brUGE
