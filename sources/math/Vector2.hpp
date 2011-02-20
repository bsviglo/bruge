#pragma once

#include "Vector3.hpp"
#include "Vector4.hpp"
#include <cmath>
#include <cassert>

namespace brUGE
{
namespace math
{
	template<class T>
	class Vector2
	{
	public:
		inline Vector2()														{ set(0,0); }
		inline Vector2(T x, T y)												{ set(x,y); }
		inline explicit Vector2(const T* const data)							{ assert(data); set(data[0], data[1]); }
		
		inline void				operator *=		(T scale)						{ x*=scale; y*=scale; }
		inline void				operator *=		(const Vector2<T>& rt)			{ x*=rt.x; y*=rt.y; }
		inline void 			operator +=		(const Vector2<T>& rt)			{ x+=rt.x; y+=rt.y; }
		inline void 			operator -=		(const Vector2<T>& rt)			{ x-=rt.x; y-=rt.y; }

		inline Vector2<T>&		operator =		(const Vector2<T>& rt)			{ set(rt.x, rt.y); return *this; }
		inline Vector2<T>&		operator =		(const T* rt)					{ assert(r); set(rt[0], rt[1]); return *this; }
		inline const T&			operator[]		(unsigned int i) const			{ assert(i<2); return data[i]; }
		inline T&				operator[]		(unsigned int i)				{ assert(i<2); return data[i]; }

		inline bool				operator ==		(const Vector2<T>& rt)			{ return (x==rt.x && y==rt.y) ? true : false;  }
		inline bool				operator !=		(const Vector2<T>& rt)			{ return !(*this == rt);  }
		
		inline void				set				(T x, T y)						{ this->x=x; this->y=y; }
		inline float			dot				(const Vector2<T>& rt) const	{ return x*rt.x + y*rt.y; }
		inline float			length			() const						{ return sqrt(x*x + y*y); }
		inline Vector2<T>		scale			(T scale) const					{ return Vector2(x*scale, y*scale); }
		inline void				normalize		();
		inline Vector2<T>		getNormalized	() const						{ Vector2<T> v=*this; v.normalize(); return v; }
		
		inline void				setZero			()								{ set(0,0); }

		inline Vector3<T>		toVec3			() const						{ return Vector3<T>(x,y,0); }
		inline Vector4<T>		toVec4			() const						{ return Vector4<T>(x,y,0,1); }

	public:
		union{ struct{ T x,y; }; struct{ T u,v; }; T data[2]; };
	};
	
	template <class T>
	inline Vector2<T> operator + (const Vector2<T>& l, const Vector2<T>& r)
	{
		return Vector2<T>(l.x+r.x, l.y+r.y);
	}

	template <class T>
	inline Vector2<T> operator - (const Vector2<T>& l, const Vector2<T>& r)
	{
		return Vector2<T>(l.x-r.x, l.y-r.y);
	}

	template <class T>
	inline Vector2<T> operator * (const Vector2<T>& l, const Vector2<T>& r)
	{
		return Vector2<T>(l.x*r.x, l.y*r.y);
	}

	template <class T>
	inline void Vector2<T>::normalize()
	{
		float len = length();
		if (!len) return;

		float invL = 1 / len;
		x *= invL; y *= invL;
	}

} // math
} // brUGE