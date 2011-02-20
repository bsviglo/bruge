#pragma once

#include "prerequisites.h"
#include "math/math_types.h"
#include "math/Plane.hpp"

namespace brUGE
{
namespace render
{

	//----------------------------------------------------------------------------------------------
	class Frustum
	{
	public:
		Frustum();
		~Frustum();
		
		void update(const mat4f& viewProj);
		bool testAABB(const AABB& aabb) const;
	
	public:
		Plane m_planes[6];
	};

} // render
} // brUGE

