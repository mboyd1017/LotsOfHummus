#ifndef _VEC_H_
#define _VEC_H_

#include <cstddef>
#include <cassert>
#include <iostream>

template<typename F> class VecT;
typedef VecT<float> Vec;

// Element of the vector space R^3
template<typename F>
class VecT
{
public:
	typedef F Field;
	enum { DIM = 3 };
	
	F x, y, z;

	VecT() {}
	VecT(const F& ax, const F& ay, const F& az)
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

	static VecT<F> zero()
	{
		return VecT<F>(0, 0, 0);
	}

	static VecT<F> basis(int i)
	{
		assert(i >= 0 && i < DIM);
		VecT<F> v(zero());
		v[i] = 1;
		return v;
	}

	VecT<F>& operator += (const VecT<F>& v)
	{
		x += v.x, y += v.y, z += v.z;
		return *this;
	}

	VecT<F>& operator -= (const VecT<F>& v)
	{
		x -= v.x, y -= v.y, z -= v.z;
		return *this;
	}

	VecT<F>& operator *= (const F& a)
	{
		x *= a, y *= a, z *= a;
		return *this;
	}

	VecT<F>& operator /= (const F& a)
	{
		return (*this) *= (1.f/a);
	}
};

template<typename F>
std::istream& operator >> (std::istream& is, VecT<F>& v)
{
	return is >> v.x >> v.y >> v.z;
}

template<typename F>
std::ostream& operator << (std::ostream& os, const VecT<F>& v)
{
	os << "(" << v.x << ", " << v.y << ", " <<  v.z << ")";
	return os;
}

template<typename F>
inline VecT<F> operator + (const VecT<F>& u, const VecT<F>& v)
{
	return VecT<F>(u.x + v.x, u.y + v.y, u.z + v.z);
}

template<typename F>
inline VecT<F> operator - (const VecT<F>& u)
{
	return VecT<F>(-u.x, -u.y, -u.z);
}


template<typename F>
inline VecT<F> operator - (const VecT<F>& u, const VecT<F>& v)
{
	return VecT<F>(u.x - v.x, u.y - v.y, u.z - v.z);
}

template<typename F>
inline VecT<F> operator * (const VecT<F>& u, const F& a)
{
	return VecT<F>(u.x * a, u.y * a, u.z * a);
}

template<typename F>
inline VecT<F> operator * (const F&a, const VecT<F>& u)
{
	return u * a;
}

template<typename F>
inline VecT<F> operator / (const VecT<F>& u, const F&a)
{
	return u * (1.f/a);
}

template<typename F>
inline F Dot(const VecT<F>& u, const VecT<F>& v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

template<typename F>
inline VecT<F> Cross(const VecT<F>& u, const VecT<F>& v)
{
	return VecT<F>(
		u.y * v.z - u.z * v.y,
		u.z * v.x - u.x * v.z,
		u.x * v.y - u.y * v.x);
}

template<typename F>
inline F LengthSq(const VecT<F>& u) // squared length
{
	return Dot(u, u);
}

template<typename F>
inline F Length(const VecT<F>& u)
{
	return sqrt(LengthSq(u));
}

template<typename F>
VecT<F> Normalize(const VecT<F>& u)
{
	return (u / Length(u));
}


#endif