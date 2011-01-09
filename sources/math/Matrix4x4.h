#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include <cassert>

namespace brUGE
{
namespace math
{
	// Describe row-major 4x4 matrix.
	//
	//  side	[0	| 1  | 2  | 3]
	//  up		[4	| 5  | 6  | 7]
	//  dir		[8	| 9  |10  |11]
	//  pos		[12 |13  |14  |15]
	//------------------------------------------------------------
	template<class T>
	class Matrix4x4
	{
	public:
		inline Matrix4x4()																	{ setZero(); }
		inline explicit Matrix4x4(const T* const lt)										{ set(lt); }
		inline Matrix4x4(const Matrix4x4<T>& lt)											{ set(lt); }

		inline Matrix4x4(
			T v00, T v01, T v02, T v03,
			T v10, T v11, T v12, T v13,
			T v20, T v21, T v22, T v23,
			T v30, T v31, T v32, T v33)
		{ 
			m00=v00; m01=v01; m02=v02; m03=v03; 
			m10=v10; m11=v11; m12=v12; m13=v13; 
			m20=v20; m21=v21; m22=v22; m23=v23; 
			m30=v30; m31=v31; m32=v32; m33=v33; 
		}

		inline Matrix4x4(Vector4<T> row0, Vector4<T> row1, Vector4<T> row2, Vector4<T> row3)
		{ 
			m00=row0.x; m01=row0.y; m02=row0.z; m03=row0.w; 
			m10=row1.x; m11=row1.y; m12=row1.z; m13=row1.w; 
			m20=row2.x; m21=row2.y; m22=row2.z; m23=row2.w; 
			m30=row3.x; m31=row3.y; m32=row3.z; m33=row3.w; 
		}

		inline Matrix4x4<T>&	operator =  	(const Matrix4x4<T>& lt)					{ set(lt); return *this; }
		inline Matrix4x4<T>&	operator =  	(const T* lt)								{ set(lt); return *this; }

		inline T&				operator [] 	(unsigned int i)							{ assert(i < 16);			return data[i]; }
		inline const T&			operator [] 	(unsigned int i) const						{ assert(i < 16);			return data[i]; }
		inline T&				operator () 	(unsigned int i, unsigned int j)			{ assert(i < 4 && j < 4);	return data[i * 4 + j]; }		
		inline const T&			operator () 	(unsigned int i, unsigned int j) const		{ assert(i < 4 && j < 4);	return data[i * 4 + j]; }

		// указатель на данные матрицы.
		//inline operator const T*() const		{ return data; }
		
		// multiply this matrix left by lt.
		inline Matrix4x4<T>& preMultiply		(const Matrix4x4<T>& lt) 					{ return (*this = mult(lt, *this)); }
		// multiply this matrix right by rt.	
		inline Matrix4x4<T>& postMultiply		(const Matrix4x4<T>& rt) 					{ return (*this = mult(*this, rt)); }

		inline void transpose					();
		inline void invert						();
		inline void setIdentity					();
		inline void setZero						();

		inline void set							(const Matrix4x4<T>& lt);
		inline void set							(const T* lt);

		inline Matrix4x4<T> getTranspose		() const 									{ Matrix4x4<T> m = *this; m.transpose(); return m; }
		inline Matrix4x4<T> getInverted			() const 									{ Matrix4x4<T> m = *this; m.invert(); return m; }

		inline void setTranslation				(const Vector3<T>& translation);
		inline void setTranslation				(float x, float y, float z);
		inline void setScale					(const Vector3<T>& scale);
		inline void setScale					(float x, float y, float z);
		inline void setRotateX					(float radianAngle);
		inline void setRotateY					(float radianAngle);
		inline void setRotateZ					(float radianAngle);
		// TODO: требует еще тестирование. Возмножно неправильная работа.
		inline void setRotateYPR				(const Vector3<T>& radianAngles);
		inline void setRotateYPR				(float radianYaw, float radianPitch, float radianRoll);

		inline void setLookAt					(const Vector3<T>& pos, const Vector3<T>& dir, const Vector3<T>& up);
		inline void setPerspectiveProj			(float gradFovy, float aspect, float zNear, float zFar);
		// TODO: разобраться как она правильно работает.
		inline void setOrthogonalProj			(float width, float height, float zNear, float zFar);
		inline void getRow						(uint i, Vector3<T>& vec) const;
		inline void getRow						(uint i, Vector4<T>& vec) const;

		inline Matrix4x4<T>& preRotateX			(float radianAngle)							{ Matrix4x4<T> m; m.setRotateX(radianAngle); 	return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postRotateX		(float radianAngle)							{ Matrix4x4<T> m; m.setRotateX(radianAngle); 	return ( *this = mult(*this, m)); }
		inline Matrix4x4<T>& preRotateY			(float radianAngle) 						{ Matrix4x4<T> m; m.setRotateY(radianAngle); 	return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postRotateY		(float radianAngle) 						{ Matrix4x4<T> m; m.setRotateY(radianAngle); 	return ( *this = mult(*this, m)); }
		inline Matrix4x4<T>& preRotateZ			(float radianAngle) 						{ Matrix4x4<T> m; m.setRotateZ(radianAngle); 	return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postRotateZ		(float radianAngle) 						{ Matrix4x4<T> m; m.setRotateZ(radianAngle); 	return ( *this = mult(*this, m)); }

		inline Matrix4x4<T>& preTranslation		(float x, float y, float z)					{ Matrix4x4<T> m; m.setTranslation(x,y,z);		return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postTranslation	(float x, float y, float z)					{ Matrix4x4<T> m; m.setTranslation(x,y,z);		return ( *this = mult(*this, m)); }
		inline Matrix4x4<T>& preTranslation		(const Vector3<T>& tr)						{ Matrix4x4<T> m; m.setTranslation(tr); 		return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postTranslation	(const Vector3<T>& tr)						{ Matrix4x4<T> m; m.setTranslation(tr); 		return ( *this = mult(*this, m)); }

		inline Matrix4x4<T>& preScale			(float x, float y, float z)					{ Matrix4x4<T> m; m.setScale(x,y,z); 			return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postScale			(float x, float y, float z)					{ Matrix4x4<T> m; m.setScale(x,y,z); 			return ( *this = mult(*this, m)); }
		inline Matrix4x4<T>& preScale			(const Vector3<T>& scl)						{ Matrix4x4<T> m; m.setScale(scl); 				return ( *this = mult(m, *this)); }
		inline Matrix4x4<T>& postScale			(const Vector3<T>& scl)						{ Matrix4x4<T> m; m.setScale(scl); 				return ( *this = mult(*this, m)); }

		inline Vector3<T>	applyToPoint		(const Vector3<T>& point) const;
		inline Vector4<T>	applyToPoint		(const Vector4<T>& point) const;
		inline Vector3<T>	applyToVector		(const Vector3<T>& vec) const;

		inline const Vector3<T>& applyToUnitAxis(uint axis) const							{ assert(axis < 3); return *reinterpret_cast<const Vector3<T>* >(&data[axis * 4]); }
		inline const Vector3<T>& applyToOrigin	() const									{ return *reinterpret_cast<const Vector3<T>* >(&data[4 * 3]); }

	public:
		enum { X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2 };

		union
		{
			struct
			{
				T m00,m01,m02,m03;
				T m10,m11,m12,m13;
				T m20,m21,m22,m23;
				T m30,m31,m32,m33;
			};

			T data[16];
		};
	};

	template <class T>
	inline void Matrix4x4<T>::set(const Matrix4x4<T>& lt)
	{
		m00=lt.m00; m01=lt.m01; m02=lt.m02; m03=lt.m03; 
		m10=lt.m10; m11=lt.m11; m12=lt.m12; m13=lt.m13; 
		m20=lt.m20; m21=lt.m21; m22=lt.m22; m23=lt.m23; 
		m30=lt.m30; m31=lt.m31; m32=lt.m32; m33=lt.m33; 
	}

	template <class T>
	inline void Matrix4x4<T>::set(const T* lt)
	{
		assert(lt);
		m00=lt[0 ]; m01=lt[1 ]; m02=lt[2 ]; m03=lt[3 ]; 
		m10=lt[4 ]; m11=lt[5 ]; m12=lt[6 ]; m13=lt[7 ]; 
		m20=lt[8 ]; m21=lt[9 ]; m22=lt[10]; m23=lt[11]; 
		m30=lt[12]; m31=lt[13]; m32=lt[14]; m33=lt[15];
	}

	template <class T>
	inline Matrix4x4<T> mult(const Matrix4x4<T>& l, const Matrix4x4<T>& r)
	{
		Matrix4x4<T> m;
		m.m00 = l.m00*r.m00 + l.m01*r.m10 + l.m02*r.m20 + l.m03*r.m30;
		m.m10 = l.m10*r.m00 + l.m11*r.m10 + l.m12*r.m20 + l.m13*r.m30;
		m.m20 = l.m20*r.m00 + l.m21*r.m10 + l.m22*r.m20 + l.m23*r.m30;
		m.m30 = l.m30*r.m00 + l.m31*r.m10 + l.m32*r.m20 + l.m33*r.m30;
		m.m01 = l.m00*r.m01 + l.m01*r.m11 + l.m02*r.m21 + l.m03*r.m31;
		m.m11 = l.m10*r.m01 + l.m11*r.m11 + l.m12*r.m21 + l.m13*r.m31;
		m.m21 = l.m20*r.m01 + l.m21*r.m11 + l.m22*r.m21 + l.m23*r.m31;
		m.m31 = l.m30*r.m01 + l.m31*r.m11 + l.m32*r.m21 + l.m33*r.m31;
		m.m02 = l.m00*r.m02 + l.m01*r.m12 + l.m02*r.m22 + l.m03*r.m32;
		m.m12 = l.m10*r.m02 + l.m11*r.m12 + l.m12*r.m22 + l.m13*r.m32;
		m.m22 = l.m20*r.m02 + l.m21*r.m12 + l.m22*r.m22 + l.m23*r.m32;
		m.m32 = l.m30*r.m02 + l.m31*r.m12 + l.m32*r.m22 + l.m33*r.m32;
		m.m03 = l.m00*r.m03 + l.m01*r.m13 + l.m02*r.m23 + l.m03*r.m33;
		m.m13 = l.m10*r.m03 + l.m11*r.m13 + l.m12*r.m23 + l.m13*r.m33;
		m.m23 = l.m20*r.m03 + l.m21*r.m13 + l.m22*r.m23 + l.m23*r.m33;
		m.m33 = l.m30*r.m03 + l.m31*r.m13 + l.m32*r.m23 + l.m33*r.m33;
		return m;
	}
	
	template <class T>
	inline void Matrix4x4<T>::getRow(uint i, Vector3<T>& vec) const
	{
		vec.x = data[i*4 + 0]; vec.y = data[i*4 + 1]; vec.z = data[i*4 + 2]; 
	}

	template <class T>
	inline void Matrix4x4<T>::getRow(uint i, Vector4<T>& vec) const
	{
		vec.x = data[i*4 + 0]; vec.y = data[i*4 + 1];
		vec.z = data[i*4 + 2]; vec.w = data[i*4 + 3]; 
	}

	template <class T>
	inline void Matrix4x4<T>::transpose()
	{
		Matrix4x4<T> m;
		m.m00=m00;	m.m01=m10;	m.m02=m20; 	m.m03=m30;
		m.m10=m01;	m.m11=m11;	m.m12=m21; 	m.m13=m31;
		m.m20=m02;	m.m21=m12;	m.m22=m22; 	m.m23=m32;
		m.m30=m03;	m.m31=m13;	m.m32=m23; 	m.m33=m33;
		m00=m.m00;	m01=m.m01;	m02=m.m02;	m03=m.m03;
		m10=m.m10;	m11=m.m11;	m12=m.m12;	m13=m.m13;
		m20=m.m20;	m21=m.m21;	m22=m.m22;	m23=m.m23;
		m30=m.m30;	m31=m.m31;	m32=m.m32;	m33=m.m33;
	}

	template <class T>
	inline void Matrix4x4<T>::invert()
	{
		T				tmp[12];
		Matrix4x4<T>	m = *this;

		/* calculate pairs for first 8 elements (cofactors) */
		tmp[0] = m.m22 * m.m33;
		tmp[1] = m.m32 * m.m23;
		tmp[2] = m.m12 * m.m33;
		tmp[3] = m.m32 * m.m13;
		tmp[4] = m.m12 * m.m23;
		tmp[5] = m.m22 * m.m13;
		tmp[6] = m.m02 * m.m33;
		tmp[7] = m.m32 * m.m03;
		tmp[8] = m.m02 * m.m23;
		tmp[9] = m.m22 * m.m03;
		tmp[10]= m.m02 * m.m13;
		tmp[11]= m.m12 * m.m03;

		/* calculate first 8 elements (cofactors) */
		m00 = tmp[0]*m.m11 + tmp[3]*m.m21 + tmp[ 4]*m.m31;
		m00-= tmp[1]*m.m11 + tmp[2]*m.m21 + tmp[ 5]*m.m31;
		m01 = tmp[1]*m.m01 + tmp[6]*m.m21 + tmp[ 9]*m.m31;
		m01-= tmp[0]*m.m01 + tmp[7]*m.m21 + tmp[ 8]*m.m31;
		m02 = tmp[2]*m.m01 + tmp[7]*m.m11 + tmp[10]*m.m31;
		m02-= tmp[3]*m.m01 + tmp[6]*m.m11 + tmp[11]*m.m31;
		m03 = tmp[5]*m.m01 + tmp[8]*m.m11 + tmp[11]*m.m21;
		m03-= tmp[4]*m.m01 + tmp[9]*m.m11 + tmp[10]*m.m21;
		m10 = tmp[1]*m.m10 + tmp[2]*m.m20 + tmp[ 5]*m.m30;
		m10-= tmp[0]*m.m10 + tmp[3]*m.m20 + tmp[ 4]*m.m30;
		m11 = tmp[0]*m.m00 + tmp[7]*m.m20 + tmp[ 8]*m.m30;
		m11-= tmp[1]*m.m00 + tmp[6]*m.m20 + tmp[ 9]*m.m30;
		m12 = tmp[3]*m.m00 + tmp[6]*m.m10 + tmp[11]*m.m30;
		m12-= tmp[2]*m.m00 + tmp[7]*m.m10 + tmp[10]*m.m30;
		m13 = tmp[4]*m.m00 + tmp[9]*m.m10 + tmp[10]*m.m20;
		m13-= tmp[5]*m.m00 + tmp[8]*m.m10 + tmp[11]*m.m20;

		/* calculate pairs for second 8 elements (cofactors) */
		tmp[ 0] = m.m20*m.m31;
		tmp[ 1] = m.m30*m.m21;
		tmp[ 2] = m.m10*m.m31;
		tmp[ 3] = m.m30*m.m11;
		tmp[ 4] = m.m10*m.m21;
		tmp[ 5] = m.m20*m.m11;
		tmp[ 6] = m.m00*m.m31;
		tmp[ 7] = m.m30*m.m01;
		tmp[ 8] = m.m00*m.m21;
		tmp[ 9] = m.m20*m.m01;
		tmp[10] = m.m00*m.m11;
		tmp[11] = m.m10*m.m01;

		/* calculate second 8 elements (cofactors) */
		m20 = tmp[ 0]*m.m13 + tmp[ 3]*m.m23 + tmp[ 4]*m.m33;
		m20-= tmp[ 1]*m.m13 + tmp[ 2]*m.m23 + tmp[ 5]*m.m33;
		m21 = tmp[ 1]*m.m03 + tmp[ 6]*m.m23 + tmp[ 9]*m.m33;
		m21-= tmp[ 0]*m.m03 + tmp[ 7]*m.m23 + tmp[ 8]*m.m33;
		m22 = tmp[ 2]*m.m03 + tmp[ 7]*m.m13 + tmp[10]*m.m33;
		m22-= tmp[ 3]*m.m03 + tmp[ 6]*m.m13 + tmp[11]*m.m33;
		m23 = tmp[ 5]*m.m03 + tmp[ 8]*m.m13 + tmp[11]*m.m23;
		m23-= tmp[ 4]*m.m03 + tmp[ 9]*m.m13 + tmp[10]*m.m23;
		m30 = tmp[ 2]*m.m22 + tmp[ 5]*m.m32 + tmp[ 1]*m.m12;
		m30-= tmp[ 4]*m.m32 + tmp[ 0]*m.m12 + tmp[ 3]*m.m22;
		m31 = tmp[ 8]*m.m32 + tmp[ 0]*m.m02 + tmp[ 7]*m.m22;
		m31-= tmp[ 6]*m.m22 + tmp[ 9]*m.m32 + tmp[ 1]*m.m02;
		m32 = tmp[ 6]*m.m12 + tmp[11]*m.m32 + tmp[ 3]*m.m02;
		m32-= tmp[10]*m.m32 + tmp[ 2]*m.m02 + tmp[ 7]*m.m12;
		m33 = tmp[10]*m.m22 + tmp[ 4]*m.m02 + tmp[ 9]*m.m12;
		m33-= tmp[ 8]*m.m12 + tmp[11]*m.m22 + tmp[ 5]*m.m02;

		/* calculate determinant */
		T det = (m.m00*m00+m.m10*m01+m.m20*m02+m.m30*m03);
		if (fabs(det) < 0.0000001f) assert(0);	

		//divide the cofactor-matrix by the determinant.
		T idet=(T)1.0/det;
		m00*=idet; m01*=idet; m02*=idet; m03*=idet;
		m10*=idet; m11*=idet; m12*=idet; m13*=idet;
		m20*=idet; m21*=idet; m22*=idet; m23*=idet;
		m30*=idet; m31*=idet; m32*=idet; m33*=idet;
	}

	template <class T>
	inline void Matrix4x4<T>::setIdentity()
	{
		m00=1;	m01=0;	m02=0;	m03=0;
		m10=0;	m11=1;	m12=0;	m13=0;
		m20=0;	m21=0;	m22=1;	m23=0;
		m30=0;	m31=0;	m32=0;	m33=1;
	}

	template <class T>
	inline void Matrix4x4<T>::setZero()
	{
		m00=0;	m01=0;	m02=0;	m03=0;
		m10=0;	m11=0;	m12=0;	m13=0;
		m20=0;	m21=0;	m22=0;	m23=0;
		m30=0;	m31=0;	m32=0;	m33=0;
	}

	template <class T>
	inline void Matrix4x4<T>::setTranslation(float x, float y, float z)
	{
		setIdentity();
		m30 = x; m31 = y; m32 = z;
	}

	template <class T>
	inline void Matrix4x4<T>::setTranslation(const Vector3<T>& t)
	{
		setIdentity();
		m30 = t.x; m31 = t.y; m32 = t.z;
	}

	template <class T>
	inline void Matrix4x4<T>::setScale(float x, float y, float z)									 
	{
		setIdentity();
		m00 = x; m11 = y; m22 = z;
	}

	template <class T>
	inline void Matrix4x4<T>::setScale(const Vector3<T>& s)							 
	{
		setIdentity();
		m00 = s.x; m11 = s.y; m22 = s.z;
	}

	template <class T>
	inline void Matrix4x4<T>::setRotateX(float radianAngle)
	{
		setIdentity();
		float cosAngle = cos(radianAngle);
		float sinAngle = sin(radianAngle);

		m11 = cosAngle;  m12 = sinAngle;
		m21 = -sinAngle; m22 = cosAngle;
	}

	template <class T>
	inline void Matrix4x4<T>::setRotateY(float radianAngle)
	{
		setIdentity();
		float cosAngle = cos(radianAngle);
		float sinAngle = sin(radianAngle);
		
		m00 = cosAngle; m02 = -sinAngle;
		m20 = sinAngle; m22 = cosAngle;
	}

	template <class T>
	inline void Matrix4x4<T>::setRotateZ(float radianAngle)
	{
		setIdentity();
		float cosAngle = cos(radianAngle);
		float sinAngle = sin(radianAngle);

		m00 = cosAngle;  m01 = sinAngle;
		m10 = -sinAngle; m11 = cosAngle;
	}

	template <class T>
	inline void Matrix4x4<T>::setRotateYPR(float radianYaw, float radianPitch, float radianRoll)
	{
		float sX, cX, sY, cY, sZ, cZ;
		sX = sin(radianPitch); sY = sin(radianYaw); sZ = sin(radianRoll);
		cX = cos(radianPitch); cY = cos(radianYaw); cZ = cos(radianRoll);
		
		setIdentity();
		m00 = cZ*cY;				m01 = -sZ*cY;				m02 = sY;
		m10 = cZ*sY*sX + sZ*cX;		m11 = -sZ*sY*sX + cZ*cX;	m12 = -cY*sX;
		m20 = -cZ*sY*cX + sZ*sX;	m21 = sZ*sY*cX + cZ*sX;		m22 = cY*cX;
	}

	template <class T>
	inline void Matrix4x4<T>::setRotateYPR(const Vector3<T>& radianAngles)
	{
		float sX, cX, sY, cY, sZ, cZ;
		sX = sin(radianAngles.y); sY = sin(radianAngles.x); sZ = sin(radianAngles.z);
		cX = cos(radianAngles.y); cY = cos(radianAngles.x); cZ = cos(radianAngles.z);
		
		setIdentity();
		m00 = cZ*cY;				m01 = -sZ*cY;				m02 = sY;
		m10 = cZ*sY*sX + sZ*cX;		m11 = -sZ*sY*sX + cZ*cX;	m12 = -cY*sX;
		m20 = -cZ*sY*cX + sZ*sX;	m21 = sZ*sY*cX + cZ*sX;		m22 = cY*cX;
	}

	template <class T>
	inline void Matrix4x4<T>::setLookAt(const Vector3<T>& pos, const Vector3<T>& dir, const Vector3<T>& up)
	{
		/*
		zaxis = normal(dir)
		xaxis = normal(cross(Up, zaxis))
		yaxis = cross(zaxis, xaxis)

		xaxis.x           yaxis.x           zaxis.x          0
		xaxis.y           yaxis.y           zaxis.y          0
		xaxis.z           yaxis.z           zaxis.z          0
		-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye) l
		*/

		Vector3<T> zAx = dir.getNormalized();
		Vector3<T> xAx = (up.cross(zAx)).getNormalized();
		Vector3<T> yAx = zAx.cross(xAx);

		m00=xAx.x;			m01=yAx.x;			m02=zAx.x;			m03=0;
		m10=xAx.y;			m11=yAx.y;			m12=zAx.y;			m13=0;
		m20=xAx.z;			m21=yAx.z;			m22=zAx.z;			m23=0;
		m30=-xAx.dot(pos);	m31=-yAx.dot(pos);	m32=-zAx.dot(pos);	m33=1;
	}

	template <class T>
	inline void Matrix4x4<T>::setPerspectiveProj(float gradFovy, float aspect, float zNear, float zFar)
	{
		//xScale     0		      0              0
		//	0      yScale         0              0
		//	0        0        zf/(zf-zn)         1
		//	0        0       -zn*zf/(zf-zn)      0
		//
		//where:
		//	yScale = cot(fovY/2)
		//	xScale = yScale / aspect ratio
		
		setZero();
		float f = 1.0f / tanf(degToRad(gradFovy * 0.5f));
		assert(f != 0);

		m00 = f / aspect;
		m11 = f;
		m22 = zFar/(zFar-zNear);
		m32 = -zFar*zNear/(zFar-zNear);
		m23 = 1.0f;
	}

	template <class T>
	inline void Matrix4x4<T>::setOrthogonalProj(float width, float height, float zNear, float zFar)
	{
		// 2/w   0        0       0
		// 0     2/h      0       0
		// 0     0    1/(zn-zf)   0
		// 0     0    zn/(zn-zf)  l

		setZero();
		float inv_d = 1.0f / (zNear - zFar);
		
		m00 = 2.0f / width;
		m11 = 2.0f / height;
		m22 = 1.0f / inv_d;
		m32 = zNear / inv_d;
		m33 = 1.0f;
	}
	
	template <class T>
	inline Vector3<T>	Matrix4x4<T>::applyToPoint(const Vector3<T>& ip) const
	{
		Vector3<T> op;
		op.x = ip.x * m00 + ip.y * m10 + ip.z * m20 + m30;
		op.y = ip.x * m01 + ip.y * m11 + ip.z * m21 + m31;
		op.z = ip.x * m02 + ip.y * m12 + ip.z * m22 + m32;
		return op;
	}

	template <class T>
	inline Vector4<T>	Matrix4x4<T>::applyToPoint(const Vector4<T>& ip) const
	{
		Vector4<T> op;
		op.x = ip.x * m00 + ip.y * m10 + ip.z * m20 + ip.w * m30;
		op.y = ip.x * m01 + ip.y * m11 + ip.z * m21 + ip.w * m31;
		op.z = ip.x * m02 + ip.y * m12 + ip.z * m22 + ip.w * m32;
		op.w = ip.x * m03 + ip.y * m13 + ip.z * m23 + ip.w * m33;
		return op;
	}

	template <class T>
	inline Vector3<T>	Matrix4x4<T>::applyToVector(const Vector3<T>& iv) const
	{
		Vector3<T> ov;
		ov.x = iv.x * m00 + iv.y * m10 + iv.z * m20;
		ov.y = iv.x * m01 + iv.y * m11 + iv.z * m21;
		ov.z = iv.x * m02 + iv.y * m12 + iv.z * m22;
		return ov;
	}

} // math
} // brUGE
