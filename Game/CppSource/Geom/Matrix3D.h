#ifndef MATRIX3D_H_
#define MATRIX3D_H_

#include "Vector3D.h"
#include "PooledObject.h"

#include <string>

template<class T>
class CMatrix3D : public CPooledObject
{
public:
	T m[3][3];
	CVector3D<T> t;

	void swap(CMatrix3D& other)
	{
		std::swap(t, other.t);
		T tmp[3][3];
		memcpy(tmp, m, sizeof(m));
		memcpy(m, other.m, sizeof(m));
		memcpy(other.m, tmp, sizeof(m));
	}

	REALLY_INLINE CMatrix3D()
	{
	}

	REALLY_INLINE CMatrix3D(bool identity)
	{
		if (identity)
		{
			identify();
		}
	}

	REALLY_INLINE void identify()
	{
		t.x = t.y = t.z = 0;
		m[0][0] = m[1][1] = m[2][2] = 1;
		m[0][1] = m[1][0] = m[0][2] = m[2][0] = m[2][1] = m[1][2] = 0;
	}

	REALLY_INLINE CVector3D<T> transformVector(const CVector3D<T>& vec) const
	{
		return CVector3D<T>(m[0][0] * vec.x + m[1][0] * vec.y + m[2][0] * vec.z + t.x,
		                    m[0][1] * vec.x + m[1][1] * vec.y + m[2][1] * vec.z + t.y,
		                    m[0][2] * vec.x + m[1][2] * vec.y + m[2][2] * vec.z + t.z);
	}

	REALLY_INLINE CVector3D<T> rotateVector(const CVector3D<T>& vec) const
	{
		return CVector3D<T>(m[0][0] * vec.x + m[1][0] * vec.y + m[2][0] * vec.z,
		                    m[0][1] * vec.x + m[1][1] * vec.y + m[2][1] * vec.z,
		                    m[0][2] * vec.x + m[1][2] * vec.y + m[2][2] * vec.z);
	}

	REALLY_INLINE CMatrix3D operator * (const CMatrix3D& mat) const
	{
		CMatrix3D result;
		result.m[0][0] = m[0][0] * mat.m[0][0] + m[0][1] * mat.m[1][0] + m[0][2] * mat.m[2][0];
		result.m[0][1] = m[0][0] * mat.m[0][1] + m[0][1] * mat.m[1][1] + m[0][2] * mat.m[2][1];
		result.m[0][2] = m[0][0] * mat.m[0][2] + m[0][1] * mat.m[1][2] + m[0][2] * mat.m[2][2];

		result.m[1][0] = m[1][0] * mat.m[0][0] + m[1][1] * mat.m[1][0] + m[1][2] * mat.m[2][0];
		result.m[1][1] = m[1][0] * mat.m[0][1] + m[1][1] * mat.m[1][1] + m[1][2] * mat.m[2][1];
		result.m[1][2] = m[1][0] * mat.m[0][2] + m[1][1] * mat.m[1][2] + m[1][2] * mat.m[2][2];

		result.m[2][0] = m[2][0] * mat.m[0][0] + m[2][1] * mat.m[1][0] + m[2][2] * mat.m[2][0];
		result.m[2][1] = m[2][0] * mat.m[0][1] + m[2][1] * mat.m[1][1] + m[2][2] * mat.m[2][1];
		result.m[2][2] = m[2][0] * mat.m[0][2] + m[2][1] * mat.m[1][2] + m[2][2] * mat.m[2][2];

		result.t = mat.transformVector(t);

		return result;
	}

	REALLY_INLINE CMatrix3D& operator *= (const CMatrix3D& otherMatrix)
	{
		*this = *this * otherMatrix;
		return *this;
	}

	REALLY_INLINE void scale(const T scaleFactor)
	{
		scaleRotation(scaleFactor);
		scaleTranslation(scaleFactor);
	}

	REALLY_INLINE void scaleRotation(const T scaleFactor)
    {
		m[0][0] *= scaleFactor;
		m[0][1] *= scaleFactor;
		m[0][2] *= scaleFactor;
		m[1][0] *= scaleFactor;
		m[1][1] *= scaleFactor;
		m[1][2] *= scaleFactor;
		m[2][0] *= scaleFactor;
		m[2][1] *= scaleFactor;
		m[2][2] *= scaleFactor;
    }

	REALLY_INLINE void scaleTranslation(const T scaleFactor)
	{
		t.x *= scaleFactor;
		t.y *= scaleFactor;
		t.z *= scaleFactor;
	}

	REALLY_INLINE void setRotationX(const T angle,
	                                const bool resetTranslation = true,
	                                const bool setZeros = true)
	{
        if (resetTranslation)
			t.x = t.y = t.z = 0;
		if (setZeros)
			m[0][1] = m[0][2] = m[1][0] = m[2][0] = 0;

		m[0][0] = 1;
        m[1][1] = m[2][2] = cosine(angle);
        m[1][2] = -(m[2][1] = sine(angle));
	}

	REALLY_INLINE void setRotationY(const T angle,
	                                const bool resetTranslation = true,
	                                const bool setZeros = true)
	{
        if (resetTranslation)
			t.x = t.y = t.z = 0;
		if (setZeros)
			m[0][1] = m[1][2] = m[1][0] = m[2][1] = 0;

		m[1][1] = 1;
        m[0][0] = m[2][2] = cosine(angle);
        m[2][0] = -(m[0][2] = sine(angle));
	}

	REALLY_INLINE void setRotationZ(const T angle,
	                                const bool resetTranslation = true,
	                                const bool setZeros = true)
	{
        if (resetTranslation)
			t.x = t.y = t.z = 0;
		if (setZeros)
			m[0][2] = m[1][2] = m[2][0] = m[2][1] = 0;

		m[2][2] = 1;
        m[0][0] = m[1][1] = cosine(angle);
        m[0][1] = -(m[1][0] = sine(angle));
	}

	REALLY_INLINE void setTranslation(const CVector3D<T>& translation)
	{
		t = translation;
	}

	REALLY_INLINE bool operator != (const CMatrix3D& mat) const
	{
		return !(*this == mat);
	}

	REALLY_INLINE bool operator == (const CMatrix3D& mat) const
	{
		return isRotationSame(mat) && isTranslationSame(mat);
	}

	bool isRotationSame(const CMatrix3D& mat) const
	{
        return  m[0][0] == mat.m[0][0] &&
	        m[0][1] == mat.m[0][1] &&
	        m[0][2] == mat.m[0][2] &&
	        m[1][0] == mat.m[1][0] &&
	        m[1][1] == mat.m[1][1] &&
	        m[1][2] == mat.m[1][2] &&
	        m[2][0] == mat.m[2][0] &&
	        m[2][1] == mat.m[2][1] &&
	        m[2][2] == mat.m[2][2];
	}

	bool isTranslationSame(const CMatrix3D& mat) const
	{
		return t == mat.t;
	}
};

#endif /* MATRIX3D_H_ */