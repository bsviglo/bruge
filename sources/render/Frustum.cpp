#include "Frustum.h"
#include "math/Matrix4x4.h"

namespace brUGE
{
	using namespace math;

namespace render
{
	
	//------------------------------------------
	Frustum::Frustum()
	{

	}

	//------------------------------------------
	Frustum::~Frustum()
	{

	}

	// 
	//------------------------------------------
	void Frustum::recalc(const mat4f& viewProj, bool normalise/* = false */)
	{
		// –асчитываем вектор нормали дл€ правой плоскости
		rightPlane.normal[0] = viewProj[ 3] - viewProj[ 0];
		rightPlane.normal[1] = viewProj[ 7] - viewProj[ 4];
		rightPlane.normal[2] = viewProj[11] - viewProj[ 8];
		rightPlane.dist	     = viewProj[15] - viewProj[12];

		// –асчитываем вектор нормали дл€ левой плоскости
		leftPlane.normal[0] = viewProj[ 3] + viewProj[ 0];
		leftPlane.normal[1] = viewProj[ 7] + viewProj[ 4];
		leftPlane.normal[2] = viewProj[11] + viewProj[ 8];
		leftPlane.dist	    = viewProj[15] + viewProj[12];

		// –асчитываем вектор нормали дл€ нижней плоскости
		bottomPlane.normal[0] = viewProj[ 3] + viewProj[ 1];
		bottomPlane.normal[1] = viewProj[ 7] + viewProj[ 5];
		bottomPlane.normal[2] = viewProj[11] + viewProj[ 9];
		bottomPlane.dist	  = viewProj[15] + viewProj[13];

		// –асчитываем вектор нормали дл€ верхней плоскости
		topPlane.normal[0] = viewProj[ 3] - viewProj[ 1];
		topPlane.normal[1] = viewProj[ 7] - viewProj[ 5];
		topPlane.normal[2] = viewProj[11] - viewProj[ 9];
		topPlane.dist	   = viewProj[15] - viewProj[13];

		// –асчитываем вектор нормали дл€ дальней плоскости
		farPlane.normal[0] = viewProj[ 3] - viewProj[ 2];
		farPlane.normal[1] = viewProj[ 7] - viewProj[ 6];
		farPlane.normal[2] = viewProj[11] - viewProj[10];
		farPlane.dist	   = viewProj[15] - viewProj[14];

		// –асчитываем вектор нормали дл€ ближней плоскости
		nearPlane.normal[0] = viewProj[ 3] + viewProj[ 2];
		nearPlane.normal[1] = viewProj[ 7] + viewProj[ 6];
		nearPlane.normal[2] = viewProj[11] + viewProj[10];
		nearPlane.dist	    = viewProj[15] + viewProj[14];
		
		if (normalise)
		{
			rightPlane.normalise();
			leftPlane.normalise();
			topPlane.normalise();
			bottomPlane.normalise();
			nearPlane.normalise();
			farPlane.normalise();
		}
	}
	
	// 
	//------------------------------------------
	bool Frustum::testRect2d(const Rect2d &rect) const
	{
		Plane2d l;
		l.n[0] = leftPlane.n[0];
		l.n[1] = leftPlane.n[1];
		l.n[2] = leftPlane.n[3];
		Plane2d r;
		r.n[0] = rightPlane.n[0];
		r.n[1] = rightPlane.n[1];
		r.n[2] = rightPlane.n[3];
		Plane2d n;
		n.n[0] = nearPlane.n[0];
		n.n[1] = nearPlane.n[1];
		n.n[2] = nearPlane.n[3];
		Plane2d f;
		f.n[0] = farPlane.n[0];
		f.n[1] = farPlane.n[1];
		f.n[2] = farPlane.n[3];

		l.normalise();
		r.normalise();
		n.normalise();
		f.normalise();

		if (l.planeClassify(rect)	 == PC_BACK
			|| r.planeClassify(rect) == PC_BACK
			|| n.planeClassify(rect) == PC_BACK
			|| f.planeClassify(rect) == PC_BACK)
		{
			return false;
		}
		return true;
	}

} // render
} // brUGE