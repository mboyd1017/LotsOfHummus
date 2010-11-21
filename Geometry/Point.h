#ifndef _POINT_H_
#define _POINT_H_

#include "Vec.h"

template<typename F> class PointT;
typedef PointT<float> Point;

template<typename F>
class PointT
{
public:
	typedef F Field;
	enum { DIM = 3 };

	F x, y, z;
	
	PointT() {}
	PointT(const F& ax, const F& ay, const F& az)
		: x(ax), y(ay), z(az)
	{
	}

	void assign(const F& ax, const F& ay, const F& az)
	{
		x = ax, y = ay, z = az;
	}

	const F& operator [] (std::size_t i) const
	{
		assert(i >= 0 && i < DIM);
		return reinterpret_cast<const F*>(this)[i];
	}

	F& operator [] (std::size_t i)
	{
		assert(i >= 0 && i < DIM);
		return reinterpret_cast<F*>(this)[i];
	}

	static PointT<F> origin()
	{
		return PointT<F>(0, 0, 0);
	}

	PointT<F>& operator += (const VecT<F>& v)
	{
		x += v.x, y += v.y, z += v.z;
		return *this;
	}

	PointT<F>& operator -= (const VecT<F>& v)
	{
		x -= v.x, y -= v.y, z -= v.z;
		return *this;
	}
};

template<typename F>
std::istream& operator >> (std::istream& is, PointT<F>& p)
{
	return is >> p.x >> p.y >> p.z;
}

template<typename F>
std::ostream& operator << (std::ostream& os, const PointT<F>& p)
{
	os << "(" <<  p.x << ", " << p.y << ", " << p.z << ")";
	return os;
}

template<typename F>
PointT<F> operator + (const PointT<F>& p, const VecT<F>& v)
{
	return PointT<F>(p.x + v.x, p.y + v.y, p.z + v.z);
}

template<typename F>
PointT<F> operator + (const VecT<F>& v, const PointT<F>& p)
{
	return p + v;
}

template<typename F>
PointT<F> operator - (const PointT<F>& p, const VecT<F>& v)
{
	return PointT<F>(p.x - v.x, p.y - v.y, p.z - v.z);
}

template<typename F>
VecT<F> operator - (const PointT<F>& p, const PointT<F>& q)
{
	return VecT<F>(p.x - q.x, p.y - q.y, p.z - q.z);
}

#endif