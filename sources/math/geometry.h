#pragma once

#include <assert.h>
#include "Vector2.h"
#include "Vector3.h"

namespace brUGE
{
namespace math
{

	// Оперделяет лежит ли один прямоугольник(2D, 3D) в нутри другого.
	//------------------------------------------
	enum ERectIntersect
	{
		RI_NO = 0,
		RI_YES,
		RI_PARTIAL
	};

	// Определяет как находится прямоугольник относительно плоскости(2D, 3D).
	//------------------------------------------
	enum EPlaneClassify
	{
		PC_FRONT = 0,
		PC_BACK,
		PC_INTERSECT
	};
	
	// Линия в 3d пространстве представляется в виде
	// pos = pos0 + t*dir; - параметрический вид.
	//------------------------------------------------------------
	class Line3d
	{
	public:
		vec3f pos0;
		vec3f dir;
	};
	
	//------------------------------------------------------------
	class Rect2d{
	public:
		union
		{
			float n[4];
			struct
			{
				vec2f minCorner;
				vec2f maxCorner;
			};
			struct
			{
				float x0, y0;
				float x1, y1;
			};
		};

		Rect2d()
		{
			minCorner.setZero();
			maxCorner.setZero();
		}	

		Rect2d(const vec2f &min, const vec2f &max) : minCorner(min), maxCorner(max) {}

		vec2f centerPos() const
		{
			return (minCorner + maxCorner).scale(0.5f);
		}

		int pointInRect(const vec2f &point) const
		{
			return point[0] >= x0
				&& point[1] >= y0
				&& point[0] <= x1
				&& point[1] <= y1;
		}

		int rectInRect(const Rect2d &rect) const{
			int test	= 0;
			if (pointInRect(rect.minCorner))
			{
				++test;
			}
			if (pointInRect(rect.maxCorner))
			{
				++test;
			}
			if (pointInRect(vec2f(rect.maxCorner[0], rect.minCorner[1])))
			{
				++test;
			}
			if (pointInRect(vec2f(rect.minCorner[0], rect.maxCorner[1])))
			{
				++test;
			}

			switch (test)
			{
				case 0:
					break;
				case 1:
				case 2:
				case 3:
					return RI_PARTIAL;
				case 4:
					return RI_YES;
			}
			
			if (rect.pointInRect(minCorner))
			{
				++test;
			}
			if (rect.pointInRect(maxCorner))
			{
				++test;
			}
			if (rect.pointInRect(vec2f(maxCorner[0], minCorner[1])))
			{
				++test;
			}
			if (rect.pointInRect(vec2f(minCorner[0], maxCorner[1])))
			{
				++test;
			}

			if (test > 0)
			{
				return RI_PARTIAL;
			}
			return RI_NO;
		}			
	};

	//------------------------------------------------------------
	class Rect3d
	{
	public:
		union
		{
			float n[4];
			struct
			{
				float x0, y0;
				float x1, y1;
				float z0, z1;
			};
			struct
			{
				Rect2d rect2d;
				float z0, z1;
			};
		};
	};
	
	//------------------------------------------------------------
	class Plane2d{
	public:
		union
		{
			float n[3];
			struct
			{
				float dist;
				vec2f normal;
			};
		};

		inline void normalise()
		{
			float len = normal.length();
			assert(len);

			float invLen = 1.0f/len;

			normal[0] *= invLen;
			normal[1] *= invLen;
			dist	  *= invLen;
		}

		inline float signedDistance(const vec2f &point) const
		{
			return (normal.dot(point) + dist);
		}

		inline int planeClassify(const Rect2d &rect) const
		{
			/*
			vec2f minPoint, maxPoint;

			if (normal[0] > 0.0f){
				minPoint[0] = rect.x0;
				maxPoint[0] = rect.x1;
			}else{
				minPoint[0] = rect.x1;
				maxPoint[0] = rect.x0;
			}

			if (normal[1] > 0.0f){
				minPoint[1] = rect.y0;
				maxPoint[1] = rect.y1;
			}else{
				minPoint[1] = rect.y1;
				maxPoint[1] = rect.y0;
			}
			
			float dmin = signedDistance(minPoint);
			float dmax = signedDistance(maxPoint);

			if (dmax * dmin < 0.0f){
				return PC_INTERSECT;
			}else if (dmin){
				return PC_FRONT;
			}
			return PC_BACK;
			*/
			
			
			float d0 = signedDistance(vec2f(rect.x0, rect.y0));
			float d1 = signedDistance(vec2f(rect.x1, rect.y0));
			float d2 = signedDistance(vec2f(rect.x1, rect.y1));
			float d3 = signedDistance(vec2f(rect.x0, rect.y1));

			if (d0 * d2 < 0.0f || d1 * d3 < 0.0f)
			{
				return PC_INTERSECT;
			}
			else if (d0 > 0.0f && d1 > 0.0f) 
			{
				return PC_FRONT;
			}
			return PC_BACK;
		}
	};

	//------------------------------------------------------------
	class Plane3d
	{
	public:
		union
		{
			float n[4];
			struct
			{
				float dist;
				vec3f normal;
			};
			struct
			{
				Plane2d plane2d;
				float	  z;	
			};
		};

		inline void normalise()
		{
			float invLen = 1.0f/normal.length();

			normal[0] *= invLen;
			normal[1] *= invLen;
			normal[2] *= invLen;
			dist	  *= invLen;
		}

		inline float signedDistance(const vec3f &point) const
		{
			return (normal.dot(point) + dist);
		}
		
		// X = pos + ((d - dot(pos, normal)/dot(dir, normal))*dir
		//------------------------------------------
		inline vec3f planeIntersectLine(const Line3d &line)
		{
			float d = line.dir.dot(normal);
			assert(d && "Line collinear to plane.");
			float f = (dist - line.pos0.dot(normal)) / d;
			return (line.pos0 + line.dir.scale(f));
		}
	};

} // math
} // brUGE
