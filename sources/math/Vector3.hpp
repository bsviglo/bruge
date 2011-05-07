#pragma once

#include "Vector2.hpp"
#include "Vector4.hpp"
#include <cmath>
#include <cassert>

namespace brUGE
{
namespace math
{
	template<class T>
	class Vector3
	{
	public:
		inline Vector3()														{ set(0,0,0); }
		inline Vector3(T x, T y, T z)											{ set(x,y,z); }
		inline explicit Vector3(const T* const data)							{ assert(data); x = data[0]; y = data[1]; z = data[2]; }
		
		inline void				operator *=		(T scale)						{ x*=scale; y*=scale; z*=scale; }
		inline void				operator /=		(const Vector3<T>& rt)			{ x/=rt.x; y/=rt.y; z/=rt.z; }
		inline void				operator *=		(const Vector3<T>& rt)			{ x*=rt.x; y*=rt.y; z*=rt.z; }
		inline void 			operator +=		(const Vector3<T>& rt)			{ x+=rt.x; y+=rt.y; z+=rt.z; }
		inline void 			operator -=		(const Vector3<T>& rt)			{ x-=rt.x; y-=rt.y; z-=rt.z; }
		
		inline Vector3<T>&		operator =		(const Vector3<T>& lt)			{ set(lt.x, lt.y, lt.z); return *this; }
		inline Vector3<T>&		operator =		(const T* lt)					{ assert(lt); set(lt[0], lt[1], lt[2]); return *this; }
		inline const T&			operator [] 	(uint i) const					{ assert(i<3); return data[i]; }
		inline T&				operator [] 	(uint i)						{ assert(i<3); return data[i]; }

		inline bool				operator ==		(const Vector3<T>& r)			{ return (x==r.x && y==r.y && z==r.z) ? true : false;  }
		inline bool				operator !=		(const Vector3<T>& r)			{ return !(*this == r);  }
		
		inline void				set				(T x, T y, T z)					{ this->x=x; this->y=y; this->z=z; }
		inline void				setZero			()								{ set(0,0,0); }
		inline Vector3<T>		cross			(const Vector3<T>& rt) const;
		inline float			dot				(const Vector3<T>& rt) const;
		inline float			length			() const;
		inline float			flatDist		(const Vector3<T>& rt) const;
		inline Vector3<T>		scale			(T scale) const;
		inline void				normalize		();
		inline Vector3<T>		getNormalized	() const 						{ Vector3<T> v=*this; v.normalize(); return v; }

		inline Vector2<T>		toVec2			() const 						{ return Vector2<T>(x,y); }
		inline Vector4<T>		toVec4			() const 						{ return Vector4<T>(x,y,z,1); }
	public:
		union{ struct{ T x,y,z; }; T data[3]; };
	};

	template<typename T>
	inline Vector3<T> lerp(const Vector3<T>& from, const Vector3<T>& to, T blend)
	{
		Vector3<T> out;
		out.x = from.x * (1.0f - blend) + to.x * blend;
		out.y = from.y * (1.0f - blend) + to.y * blend;
		out.z = from.z * (1.0f - blend) + to.z * blend;
		return out;
	}

	template <class T>
	inline Vector3<T> operator + (const Vector3<T>& lt, const Vector3<T>& rt)
	{
		return Vector3<T>(lt.x+rt.x, lt.y+rt.y, lt.z+rt.z);
	}
	
	template <class T>
	inline Vector3<T> operator - (const Vector3<T>& lt, const Vector3<T>& rt)
	{
		return Vector3<T>(lt.x-rt.x, lt.y-rt.y, lt.z-rt.z);
	}
	
	template <class T>
	inline Vector3<T> operator * (const Vector3<T>& lt, const Vector3<T>& rt)
	{
		return Vector3<T>(lt.x*rt.x, lt.y*rt.y, lt.z*rt.z);
	}

	template <class T>
	inline float Vector3<T>::dot(const Vector3<T>& rt) const
	{
		return (x*rt.x + y*rt.y + z*rt.z);
	}

	template <class T>
	inline Vector3<T> Vector3<T>::cross(const Vector3<T>& rt) const
	{
		return Vector3<T>(y*rt.z - z*rt.y, z*rt.x - x*rt.z, x*rt.y - y*rt.x);
	}

	template <class T>
	inline float Vector3<T>::length() const
	{
		return sqrtf(x*x + y*y + z*z);
	}

	template <class T>
	inline float Vector3<T>::flatDist(const Vector3<T>& rt) const
	{
		T deltaX = this->x - rt.x;
		T deltaZ = this->z - rt.z;
		return sqrtf(deltaX*deltaX + deltaZ*deltaZ);
	}

	template <class T>
	inline void Vector3<T>::normalize()
	{
		float len = length();
		if (!len) return;

		float invL = 1.0f / len;
		x *= invL; y *= invL; z *= invL;
	}

	template <class T>
	inline Vector3<T> Vector3<T>::scale(T scale) const
	{
		return Vector3<T>(x*scale, y*scale, z*scale);
	}

} // math
} // brUGE