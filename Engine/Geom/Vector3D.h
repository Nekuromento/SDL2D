#ifndef VEC3D_H_
#define VEC3D_H_

#include "common_include.h"

template<class T>
class CVector3D
{
public:
	T x;
	T y;
	T z;

	REALLY_INLINE void swap(CVector3D& other)
	{
		std::swap(x, other.x);
		std::swap(y, other.y);
		std::swap(z, other.z);
	}

	REALLY_INLINE CVector3D()
	{}

	REALLY_INLINE CVector3D(const T x, const T y, const T z) :
		x(x),
		y(y),
		z(z)
	{}

	REALLY_INLINE CVector3D operator + (const CVector3D& vec) const
	{
		return CVector3D(x + vec.x, y + vec.y, z + vec.z);
	}

	REALLY_INLINE CVector3D& operator += (const CVector3D& vec)
	{
		*this = *this + vec;
		return *this;
	}

	REALLY_INLINE CVector3D operator / (const T value) const
	{
		return CVector3D(x / value, y / value, z / value);
	}

	REALLY_INLINE CVector3D& operator /= (const CVector3D& vec)
	{
		*this = *this / vec;
		return *this;
	}

	REALLY_INLINE CVector3D operator * (const T value) const
	{
		return CVector3D(x * value, y * value, z * value);
	}

	REALLY_INLINE CVector3D& operator *= (const CVector3D& vec)
	{
		*this = *this * vec;
		return *this;
	}

	REALLY_INLINE bool operator == (const CVector3D& vec) const
	{
		return x == vec.x && y == vec.y && z == vec.z;
	}

	REALLY_INLINE bool operator != (const CVector3D& vec) const
	{
		return !(*this == vec);
	}
};

#endif /* VEC3D_H_ */