#ifndef __VECTOR__
#define __VECTOR__

#define SQR(x)((x)*(x))
#define CUBE(x)((x)*(x)*(x))
#define QUAD(x)((x)*(x)*(x)*(x))

#include <math.h>

template <class T=float, int N=3>
class CVector
{
public:
	T vals[N];

	// constructors 
	CVector()					{ memset(vals, 0, sizeof(T)*N); }
	CVector(T i, T j, T k)		{ vals[0] = i; vals[1] = j; vals[2] = k; }
	CVector(T *data)			{ memcpy(vals, data, sizeof(T)*N); }
	CVector(const CVector &V)	{ memcpy(vals, V.vals, sizeof(T)*N); }

	void assign(T *data, int size)
	{
		if(N != size)
			return;
		memcpy(vals, data, sizeof(T)*N);
	}

	void assign(T i, T j, T k)			{vals[0] = i; vals[1] = j; vals[2] = k;}
	
	//operators

	T& operator[]		(unsigned int i)		{return vals[i];}
	const T& operator[]	(unsigned int i) const	{return vals[i];}

	CVector<T,N> operator-()
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = -vals[i];
		return V2;
	}

	CVector<T,N>& operator=(const CVector<T,N>& V)
	{
		memcpy(vals, V.vals, sizeof(T)*N);
		return (*this);
	}

	CVector<T,N> operator+(const CVector<T,N>& V1)
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] + V1.vals[i];
		return V2;
	}

	CVector<T,N> operator+(const T offset)		
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] + offset;
		return V2;
	}

	CVector<T,N> operator+=(const CVector<T,N>& V1)
	{
		for(int i = 0; i < N; i++)
			vals[i] += V1.vals[i];
		return (*this);
	}

	CVector<T,N> operator+=(const T offset)		
	{
		for(int i = 0; i < N; i++)
			vals[i] += offset;
		return (*this);
	}

	CVector<T,N> operator-(const CVector<T,N>& V1)
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] - V1.vals[i];
		return V2;
	}

	CVector<T,N> operator-(const T offset)		
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] - offset;
		return V2;
	}

	CVector<T,N> operator-=(const CVector<T,N>& V1)
	{
		for(int i = 0; i < N; i++)
			vals[i] -= V1.vals[i];
		return (*this);
	}

	CVector<T,N> operator-=(const T offset)		
	{
		for(int i = 0; i < N; i++)
			vals[i] -= offset;
		return (*this);
	}

	CVector<T,N> operator*(const CVector<T,N>& V1)
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] * V1.vals[i];
		return V2;
	}

	CVector<T,N> operator*(const T scale)		
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] * scale;
		return V2;
	}

	CVector<T,N> operator*=(const CVector<T,N>& V1)
	{
		for(int i = 0; i < N; i++)
			vals[i] *= V1.vals[i];
		return (*this);
	}

	CVector<T,N> operator*=(const T scale)
	{
		for(int i = 0; i < N; i++)
			vals[i] *= scale;
		return (*this);
	}

	CVector<T,N> operator/(const CVector<T,N>& V1)
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] / V1.vals[i];
		return V2;
	}

	CVector<T,N> operator/(const T scale)		
	{
		CVector<T,N> V2;
		for(int i = 0; i < N; i++)
			V2.vals[i] = vals[i] / scale;
		return V2;
	}

	// others

	T length()						
	{
		T norm = 0;
		for(int i = 0; i < N; i++)
			norm += SQR(vals[i]);
		norm = sqrt(norm);
		return norm;
	}

	T length2()						
	{
		T norm = 0;
		for(int i = 0; i < N; i++)
			norm += SQR(vals[i]);
		return norm;
	}


	CVector<T,N> normalize()				
	{
		CVector<T,N> V = (*this) / length();
		return V;
	}
};

/*
template<class T, int N>
inline CVector<T,N> operator*(const CVector<T,N> vecA, const T scale)
{
	CVector<T,N> vecB = vecA;
	vecB = vecA * scale;
	return vecB;
}
*/

template<class T, int N>
inline CVector<T,N> operator*(T scale, CVector<T,N>& vecA)
{
	CVector<T,N> vecB = vecA;
	vecB = vecA * scale;
	return vecB;
}

template<class T>
inline CVector<T,3> vecCross(const CVector<T,3>& vecA, const CVector<T,3>& vecB)		
{
	float crossVals[3];
	crossVals[0] = vecA[1]*vecB[2] - vecA[2]*vecB[1];
	crossVals[1] = vecA[2]*vecB[0] - vecA[0]*vecB[2];
	crossVals[2] = vecA[0]*vecB[1] - vecA[1]*vecB[0];
	return CVector<T,3>(crossVals);
}

template<class T, int N>
inline T vecDot(const CVector<T,N>& vecA, const CVector<T,N>& vecB)
{
	T dot = 0;
	for(int i = 0; i < N; i++)
		dot += vecA[i] * vecB[i];
	return dot;
}

typedef CVector<float,3> CVec3f;
typedef CVector<double,3> CVec3d;
typedef CVector<float,4> CVec4f;
typedef CVector<double,4> CVec4d;

#endif