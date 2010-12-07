#pragma once

#include "prerequisites.h"
#include "math/math_types.h"
#include "math/geometry.h"

namespace brUGE
{
namespace render
{

	class Frustum
	{
	public:
		Frustum();
		~Frustum();
		
		// ¬ычисл€ет шесть плоскостей усеченной пирамиды видимости исход€
		// из текущей проекционно-видовой матрицы.
		// ѕримечание: параметр normalise отвечает за последущую нормализацию
		//			   этих плоскостей. Ќо так как она не всегда нужна, то опционально
		//			   он установлен в false.	
		//------------------------------------------
		void recalc(const mat4f& viewProj, bool normalise = false);
		
		// ѕроводит тест попадани€ 2d четырехугольника в спроецированную на плоскость ZOX
		// усеченную пирамиду видимости т.е. в трапецию лежащую в плоскости ZOX.
		//------------------------------------------
		bool testRect2d(const math::Rect2d& rect) const;
		//bool testRect3d(const Rect2d &rect) const;
		//bool testAABB(const brAABoundingBox &aabb) const;
		//bool testBB(const brBoundingBox &bb) const;
	
	public:
		math::Plane3d rightPlane;
		math::Plane3d leftPlane;
		math::Plane3d topPlane;
		math::Plane3d bottomPlane;
		math::Plane3d nearPlane;
		math::Plane3d farPlane;
	};

} // render
} // brUGE

