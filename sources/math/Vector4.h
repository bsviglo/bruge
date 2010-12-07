#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include <math.h>
#include <assert.h>

namespace brUGE
{
namespace math
{
	template<class T>
	class Vector4
	{
	public:
		inline Vector4()														{ set(0,0,0,0); }
		inline Vector4(const Vector3<T>& v3, T w)								{ set(v3.x, v3.y, v3.z, w); }
		inline Vector4(T x, T y, T z, T w)										{ set(x,y,z,w); }
		inline explicit Vector4(const T* const data)							{ assert(data); set(data[0], data[1], data[2], data[3]); }

		inline void				operator *=		(const Vector4<T>& rt)			{ x*=rt.x; y*=rt.y; z*=rt.z; w*=rt.w; }
		inline void 			operator +=		(const Vector4<T>& rt)			{ x+=rt.x; y+=rt.y; z+=rt.z; w+=rt.w; }
		inline void 			operator -=		(const Vector4<T>& rt)			{ x-=rt.x; y-=rt.y; z-=rt.z; w-=rt.w; }
			
		inline Vector4<T>&		operator = 		(const Vector4<T>& rt)			{ set(rt.x, rt.y, rt.z, rt.w); return *this; }
		inline Vector4<T>&		operator = 		(const T* rt)					{ assert(rt); set(rt[0], rt[1], rt[2], rt[3]); return *this; }
		inline const T&			operator[] 		(unsigned int i) const			{ assert(i<4); return data[i]; }
		inline T&				operator[] 		(unsigned int i)				{ assert(i<4); return data[i]; }

		inline bool				operator ==		(const Vector4<T>& rt)			{ return (x==rt.x && y==rt.y && z==rt.z && w==rt.w) ? true : false; }
		inline bool				operator !=		(const Vector4<T>& rt)			{ return !(*this == rt);  }

		inline void				set				(T x, T y, T z, T w)			{ this->x=x; this->y=y; this->z=z; this->w=w; }
		inline float			dot				(const Vector4<T>& rt) const;
		inline float			length			() const;
		inline Vector4<T>		scale			(T scale) const;
		inline void				normalize		();
		inline Vector4<T>		getNormalized	() const						{ Vector4<T> v=*this; v.normalize(); return v; }
		inline void				setZero			()								{ set(0,0,0,0); }

		inline Vector2<T>		toVec2			() const						{ return Vector2<T>(x,y); }
		inline Vector3<T>		toVec3			() const						{ return Vector3<T>(x,y,z); }

	public:
		union{ struct{ T x,y,z,w; }; T data[4];	};
	};

	template <class T>
	inline Vector4<T> operator + (const Vector4<T>& l, const Vector4<T>& r)
	{
		return Vector4<T>(l.x+r.x, l.y+r.y, l.z+r.z, l.w+r.w);
	}

	template <class T>
	inline Vector4<T> operator - (const Vector4<T>& l, const Vector4<T>& r)
	{
		return Vector4<T>(l.x-r.x, l.y-r.y, l.z-r.z, l.w-r.w);
	}

	template <class T>
	inline Vector4<T> operator * (const Vector4<T>& l, const Vector4<T>& r)
	{
		return Vector4<T>(l.x*r.x, l.y*r.y, l.z*r.z, l.w*r.w);
	}

	template <class T>
	inline float Vector4<T>::dot(const Vector4<T>& r) const
	{
		return (x*r.x + y*r.y + z*r.z + w*r.w);
	}

	template <class T>
	inline float Vector4<T>::length() const
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}

	template <class T>
	inline void Vector4<T>::normalize()
	{
		float len = length();
		if (!len) return;

		float invL = 1 / len;
		x *= invL; y *= invL; z *= invL; w *= invL;
	}

	template <class T>
	inline Vector4<T> Vector4<T>::scale(T scale) const
	{
		return Vector4<T>(x*scale, y*scale, z*scale, w*scale);
	}

} // math
} // brUGE