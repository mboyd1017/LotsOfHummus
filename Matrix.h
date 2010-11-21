#ifndef __MATRIX__
#define __MATRIX__

#include <malloc.h>
#include <string.h>

#define s2i(i,j,n)((i)*(n)+(j))

template <class T, int M, int N>
class CMatrix
{

public:
	T vals[M*N];

	// constructors
	CMatrix()					{ memset(vals, 0, sizeof(T)*M*N);}
	CMatrix(T *data)			{ memcpy(vals, data, sizeof(T)*M*N); }
	CMatrix(const CMatrix &mat)	{ memcpy(vals, mat.vals, sizeof(T)*M*N); }

	//operators 
	T& operator[]		(unsigned int i)		{return vals[i];}
	const T& operator[]	(unsigned int i) const	{return vals[i];}

	CMatrix<T,M,N>& operator= (const CMatrix<T,M,N>& A1)
	{
		memcpy(vals, A1.vals, sizeof(float)*M*N);
		return (*this);
	}

	CMatrix<T,M,N> operator+(const CMatrix<T,M,N>& A1)
	{
		CMatrix<T,M,N> A2;
		for(int i = 0; i < N*N; i++)
			A2[i] = vals[i] + A1[i];
		return A2;
	}

	CMatrix<T,M,N> operator+=(const CMatrix<T,M,N>& A1)
	{
		for(int i = 0; i < N*N; i++)
			vals[i] += A1[i];
		return (*this);
	}

	CMatrix<T,M,N> operator-(const CMatrix<T,M,N>& A1)
	{
		CMatrix<T,M,N> A2;
		for(int i = 0; i < N*N; i++)
			A2[i] = vals[i] - A1[i];
		return A2;
	}

	CMatrix<T,M,N> operator-=(const CMatrix<T,M,N>& A1)
	{
		for(int i = 0; i < N*N; i++)
			vals[i] -= A1[i];
		return (*this);
	}

	CVector<T,M> operator*(const CVector<T,N>& V)
	{
		CVector<T,M> TV;
		for(int i = 0; i < M; i++)
			for(int j = 0; j < N; j++)
				TV[i] += vals[s2i(i,j,N)] * V[j];
		return TV;
	}

	CMatrix<T,M,N> operator*(const T k)
	{
		CMatrix<T,M,N> A2;
		for(int i = 0; i < N*N; i++)
			A2[i] = vals[i] * k;
		return A2;
	}


	CMatrix<T,M,N> operator*=(const T k)
	{
		for(int i = 0; i < N*N; i++)
			vals[i] *= k;
		return (*this);
	}

	// others

	void makeIdentity()
	{
		if(M == N)
		{
			for(int i = 0; i < M; i++)
				vals[i*M+i] = 1.0f;
		}
	}

	float determinant()					{return vals[0] * (vals[4] * vals[8] - vals[5] * vals[7]) - vals[1] * (vals[3] * vals[8] - vals[5] * vals[6]) + vals[2] * (vals[3] * vals[7] - vals[4] * vals[6]);}
	
	CMatrix<T,3,3> inverse()
	{
		CMatrix<T,3,3> invM;
		if(M != 3 || N != 3)
		{	
			printf("Inverting non 3X3 matrix\n");
			return invM;
		}

		invM.vals[0] = +(vals[s2i(1,1,3)] * vals[s2i(2,2,3)] - vals[s2i(1,2,3)] * vals[s2i(2,1,3)]);
		invM.vals[1] = -(vals[s2i(0,1,3)] * vals[s2i(2,2,3)] - vals[s2i(2,1,3)] * vals[s2i(0,2,3)]);
		invM.vals[2] = +(vals[s2i(0,1,3)] * vals[s2i(1,2,3)] - vals[s2i(1,1,3)] * vals[s2i(0,2,3)]);
		invM.vals[3] = -(vals[s2i(1,0,3)] * vals[s2i(2,2,3)] - vals[s2i(2,0,3)] * vals[s2i(1,2,3)]);
		invM.vals[4] = +(vals[s2i(0,0,3)] * vals[s2i(2,2,3)] - vals[s2i(2,0,3)] * vals[s2i(0,2,3)]);
		invM.vals[5] = -(vals[s2i(0,0,3)] * vals[s2i(1,2,3)] - vals[s2i(1,0,3)] * vals[s2i(0,2,3)]);
		invM.vals[6] = +(vals[s2i(1,0,3)] * vals[s2i(2,1,3)] - vals[s2i(2,0,3)] * vals[s2i(1,1,3)]);
		invM.vals[7] = -(vals[s2i(0,0,3)] * vals[s2i(2,1,3)] - vals[s2i(2,0,3)] * vals[s2i(0,1,3)]);
		invM.vals[8] = +(vals[s2i(0,0,3)] * vals[s2i(1,1,3)] - vals[s2i(1,0,3)] * vals[s2i(0,1,3)]);

		invM *= (1 / determinant());

		return invM;
	}

	CMatrix<T,N,M> transpose()
	{
		CMatrix<T,N,M> trans;
		for(int i = 0; i < M; i++)
			for(int j = 0; j < N; j++)
				trans[s2i(i,j,M)] = vals[s2i(j,i,N)];

		return trans;
	}

};

template<class T, int M, int N, int O>
inline CMatrix<T,M,N> operator*(const CMatrix<T,M,O>& A1, const CMatrix<T,O,N>& A2)
{
	CMatrix<T,M,N> A3;
	for(int i = 0; i < M; i++)
		for(int j = 0; j < N; j++)
			for(int k = 0; k < O; k++)
				A3[s2i(i,j,N)] += A1[s2i(i,k,O)] * A2[s2i(k,j,N)];
	return A3;
}


typedef CMatrix<float,3,3> CMat3X3f;
typedef CMatrix<double,3,3> CMat3X3d;

#endif