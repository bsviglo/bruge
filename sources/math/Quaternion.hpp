#pragma once

#include "math_types.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix4x4.hpp"
#include <cmath>
#include <cassert>

namespace brUGE
{
namespace math
{
	
	//----------------------------------------------------------------------------------------------
	class Quaternion
	{
	public:
		inline Quaternion()												{ set(0, 0, 0, 1); }
		inline explicit Quaternion(const vec4f& q)						{ set(q.x, q.y, q.z, q.w); }
		inline explicit Quaternion(const mat4f& mat4)					{ set(mat4); }
		inline Quaternion(float angle, const vec3f& axis)				{ set(angle, axis); }
		inline Quaternion(float yaw, float pitch, float roll)			{ set(yaw, pitch, roll); }
		inline Quaternion(const vec3f& v, float w)						{ set(v.x, v.y, v.z, w); }
		inline Quaternion(float x, float y, float z, float w)			{ set(x, y, z, w); }

		inline Quaternion&	operator =	(const Quaternion& rht)			{ set(rht.x, rht.y, rht.z, rht.w); return *this; }
		inline float&		operator []	(unsigned int i)				{ assert(i<4); return q[i]; }
		inline const float& operator []	(unsigned int i) const			{ assert(i<4); return q[i]; }

		inline bool			operator ==	(const Quaternion& rht) const	{ (x==rht.x && y==rht.y && z==rht.z && w==rht.w) ? true : false; }
		inline bool			operator !=	(const Quaternion& rht) const	{ return !(*this == rht); }

		inline void			operator *=	(float rht)						{ w *= rht; x *= rht; y *= rht; z *= rht; }
		inline void			operator +=	(const Quaternion& rht)			{ w += rht.w; x += rht.x; y += rht.y; z += rht.z; }
		inline void			operator -=	(const Quaternion& rht)			{ w -= rht.w; x -= rht.x; y -= rht.y; z -= rht.z; }
		inline void			operator *=	(const Quaternion& rht);
	
		inline void			set(float x, float y, float z, float w)		{ this->x = x; this->y = y; this->z = z; this->w = w; }
		inline void			set(const vec3f& v, float w)				{ set(v.x, v.y, v.z, w); }	
		inline void			set(const vec3f& v)							{ set(v.x, v.y, v.z, 1.0f); }

		inline void			set(float angle, const vec3f& axis);
		inline void			set(float yaw, float pitch, float roll);
		inline void			set(const mat4f& mat);

		inline void			conjugate()									{ x = -x; y = -y; z = -z; }
		inline Quaternion	getConjugated() const						{ Quaternion q = *this; q.conjugate(); return q; }
		inline void			normalize();
		inline Quaternion	getNormalized() const						{ Quaternion q = *this; q.normalize(); return q; }
		
		
		inline vec3f		rotate(const vec3f& point) const;
		inline mat4f		toMat4() const;
		
	public:
		union { struct { float x, y, z, w; }; struct { float q[4]; }; };
	};
	
	//-- Spherical linear interpolation (Slerp).
	inline Quaternion slerp(const Quaternion& from, const Quaternion& to, float t);
	//-- Linear interpolation (Lerp).
	inline Quaternion lerp (const Quaternion& from, const Quaternion& to, float t);
	//-- restore fourths component of the normalized quaternion.
	inline void renormalize(Quaternion& q);
	
	//-- operators.
	inline Quaternion operator + (const Quaternion& lft, const Quaternion& rht);
	inline Quaternion operator - (const Quaternion& lft, const Quaternion& rht);
	inline Quaternion operator * (const Quaternion& lft, const Quaternion& rht);

	//----------------------------------------------------------------------------------------------
	inline void renormalize(Quaternion& q)
	{
		float len = 1 - q.x*q.x - q.y*q.y - q.z*q.z;

		if (len < EPSILON)	q.w = 0;
		else				q.w = -sqrtf(len);
	}

	//----------------------------------------------------------------------------------------------
	inline Quaternion operator + (const Quaternion& lft, const Quaternion& rht)
	{
		return Quaternion(lft.x + rht.x, lft.y + rht.y, lft.z + rht.z, lft.w + rht.w);
	}

	//----------------------------------------------------------------------------------------------
	inline Quaternion operator - (const Quaternion& lft, const Quaternion& rht)
	{
		return Quaternion(lft.x - rht.x, lft.y - rht.y, lft.z - rht.z, lft.w - rht.w);
	}

	//----------------------------------------------------------------------------------------------
	inline Quaternion operator * (const Quaternion& lft, const Quaternion& rht)
	{
		return Quaternion(
			lft.y * rht.z - lft.z * rht.y + lft.w * rht.x + lft.x * rht.w,
			lft.z * rht.x - lft.x * rht.z + lft.w * rht.y + lft.y * rht.w,
			lft.x * rht.y - lft.y * rht.x + lft.w * rht.z + lft.z * rht.w,
			lft.w * rht.w - lft.x * rht.x - lft.y * rht.y - lft.z * rht.z
			);
	}

	//----------------------------------------------------------------------------------------------
	inline void	Quaternion::operator *=	(const Quaternion& rht)
	{
		x = y * rht.z - z * rht.y + w * rht.x + x * rht.w;
		y = z * rht.x - x * rht.z + w * rht.y + y * rht.w;
		z = x * rht.y - y * rht.x + w * rht.z + z * rht.w;
		w = w * rht.w - x * rht.x - y * rht.y - z * rht.z;
	}
	
	//----------------------------------------------------------------------------------------------
	inline void Quaternion::set(float yaw, float pitch, float roll)
	{
		yaw	  *= 0.5f;
		pitch *= 0.5f;
		roll  *= 0.5f;

		float cx = cosf(yaw);
		float cy = cosf(pitch);
		float cz = cosf(roll);
		float sx = sinf(yaw);
		float sy = sinf(pitch);
		float sz = sinf(roll);

		float cc = cx * cx;
		float cs = cx * sz;
		float sc = sx * cz;
		float ss = sx * sz;

		x = cy * sc - sy * cs;
		y = cy * ss + sy * cc;
		z = cy * cs - sy * sc;
		w = cy * cc + sy * ss;
	}

	//----------------------------------------------------------------------------------------------
	inline void Quaternion::set(float angle, const vec3f& axis)
	{
		float sine   = sinf(angle * 0.5f);
		float cosine = cosf(angle * 0.5f);

		x = axis.x * sine;
		y = axis.y * sine;
		z = axis.z * sine;
		w = cosine;
	}

	//----------------------------------------------------------------------------------------------
	inline void Quaternion::set(const mat4f& mat)
	{
		float s, q[4];
		int	  i, j, k;
		int	  nxt[3] = {1,2,0};

		float trace = mat(0,0) + mat(1,1) + mat(2,2);

		if (trace > 0.0f)
		{
			s = sqrtf(trace + 1.0f);
			w = s / 2.0f;
			s = 0.5f / s;
			x = (mat(2,1) - mat(1,2)) * s;
			y = (mat(0,2) - mat(2,0)) * s;
			z = (mat(1,0) - mat(0,1)) * s;
		}
		else
		{
			i = 0;

			if (mat(1,1) > mat(0,0))	i = 1;
			if (mat(2,2) > mat(i,i))	i = 2;

			j = nxt[i];
			k = nxt[j];

			s = sqrtf((mat(i,i) - (mat(j,j) + mat(k,k))) + 1.0f);

			q[i] = s * 0.5f;

			if (s != 0.0f)
				s = 0.5f / s;

			q[3] = (mat(k,j) - mat(j,k)) * s;
			q[j] = (mat(j,i) + mat(i,j)) * s;
			q[k] = (mat(k,i) + mat(i,k)) * s;

			x = q[0];
			y = q[1];
			z = q[2];
			w = q[3];
		}
	}

	//----------------------------------------------------------------------------------------------
	inline void Quaternion::normalize()
	{
		float lenSq = x * x + y * y + z * z;

		if (lenSq > 1.0f - EPSILON)
		{
			float invLen = 1.0f / lenSq;

			x *= invLen;
			y *= invLen;
			z *= invLen;
			w = 0.0f;
		}
		else
		{
			w = sqrtf(1.0f - lenSq);
		}
	}

	//----------------------------------------------------------------------------------------------
	inline mat4f Quaternion::toMat4() const
	{
		mat4f mat;

		//-- 1st row
		mat[ 0] = 1.0f - 2.0f * (y * y + z * z);
		mat[ 1] = 2.0f * (x * y - w * z);
		mat[ 2] = 2.0f * (x * z + w * y);
		mat[ 3] = 0.0f;

		//-- 2nd row
		mat[ 4] = 2.0f * (x * y + w * z);
		mat[ 5] = 1.0f - 2.0f * (x * x + z * z);
		mat[ 6] = 2.0f * (y * z - w * x);
		mat[ 7] = 0.0f;

		//-- 3rd row
		mat[ 8] = 2.0f * (x * z - w * y);
		mat[ 9] = 2.0f * (y * z + w * x);
		mat[10] = 1.0f - 2.0f * (x * x + y * y);
		mat[11] = 0.0f;

		//-- 4th row
		mat[12] = 0.0f;
		mat[13] = 0.0f;
		mat[14] = 0.0f;
		mat[15] = 1.0f;

		return mat;
	}

	//----------------------------------------------------------------------------------------------
	inline vec3f Quaternion::rotate(const vec3f& point) const
	{
		Quaternion p(point, 0.0f);
		Quaternion qConj(-x, -y, -z, w);

		p = (*this * p * qConj);

		return vec3f(p.x, p.y, p.z);
	}

	//----------------------------------------------------------------------------------------------
	inline Quaternion slerp(const Quaternion& from, const Quaternion& to, float t)
	{
		Quaternion result;
		float cosom, absCosom, sinom, omega, scale0, scale1;

		cosom    = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
		absCosom = fabs(cosom);

		if ((1.0f - absCosom) > EPSILON)
		{
			omega  = acosf(absCosom);
			sinom  = 1.0f / sinf(omega);
			scale0 = sinf((1.0f - t) * omega) * sinom;
			scale1 = sinf(t * omega) * sinom;
		}
		else
		{
			scale0 = 1.0f - t;
			scale1 = t;
		}

		scale1 = (cosom >= 0.0f) ? scale1 : -scale1;

		result.x = scale0 * from.x + scale1 * to.x;
		result.y = scale0 * from.y + scale1 * to.y;
		result.z = scale0 * from.z + scale1 * to.z;
		result.w = scale0 * from.w + scale1 * to.w;

		return result;
	}

	//----------------------------------------------------------------------------------------------
	inline Quaternion lerp(const Quaternion& from, const Quaternion& to, float t)
	{
		Quaternion result;
		float cosom, scale0, scale1;

		cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

		scale0 = 1.0f - t;
		scale1 = (cosom >= 0.0f) ? t : -t;

		result.x = scale0 * from.x + scale1 * to.x;
		result.y = scale0 * from.y + scale1 * to.y;
		result.z = scale0 * from.z + scale1 * to.z;
		result.w = scale0 * from.w + scale1 * to.w;

		result.normalize();

		return result;
	}

} //-- math
} //-- brUGE
