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
		
		// ��������� ����� ���������� ��������� �������� ��������� ������
		// �� ������� �����������-������� �������.
		// ����������: �������� normalise �������� �� ���������� ������������
		//			   ���� ����������. �� ��� ��� ��� �� ������ �����, �� �����������
		//			   �� ���������� � false.	
		//------------------------------------------
		void recalc(const mat4f& viewProj, bool normalise = false);
		
		// �������� ���� ��������� 2d ���������������� � ��������������� �� ��������� ZOX
		// ��������� �������� ��������� �.�. � �������� ������� � ��������� ZOX.
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

