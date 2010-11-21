#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <math.h>
#include <iostream>

#include "Vec.h"
#include "Point.h"
#include "Ray.h"

using std::ostream;

template<typename F> class TransformT;
typedef TransformT<float> Transform;
typedef TransformT<double> Transformd;

template<typename F>
class TransformT
{
public:
//	typedef Field F;
	F e[4][4];
	F inv[4][4];

	TransformT() {
		(*this) = TransformT::Identity();
	}

	TransformT(
		const F& e00, const F& e01, const F& e02, const F& e03,
		const F& e10, const F& e11, const F& e12, const F& e13,
		const F& e20, const F& e21, const F& e22, const F& e23,
		const F& e30, const F& e31, const F& e32, const F& e33)
	{
		assign(
			e00, e01, e02, e03,
			e10, e11, e12, e13,
			e20, e21, e22, e23,
			e30, e31, e32, e33);
	}

	TransformT(const F m[4][4]) {
		assign(
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	TransformT(const F m[4][4], const F mInv[4][4]) {
		e[0][0] = m[0][0], e[0][1] = m[0][1], e[0][2] = m[0][2], e[0][3] = m[0][3];
		e[1][0] = m[1][0], e[1][1] = m[1][1], e[1][2] = m[1][2], e[1][3] = m[1][3];
		e[2][0] = m[2][0], e[2][1] = m[2][1], e[2][2] = m[2][2], e[2][3] = m[2][3];
		e[3][0] = m[3][0], e[3][1] = m[3][1], e[3][2] = m[3][2], e[3][3] = m[3][3];

		inv[0][0] = mInv[0][0], inv[0][1] = mInv[0][1], inv[0][2] = mInv[0][2], inv[0][3] = mInv[0][3];
		inv[1][0] = mInv[1][0], inv[1][1] = mInv[1][1], inv[1][2] = mInv[1][2], inv[1][3] = mInv[1][3];
		inv[2][0] = mInv[2][0], inv[2][1] = mInv[2][1], inv[2][2] = mInv[2][2], inv[2][3] = mInv[2][3];
		inv[3][0] = mInv[3][0], inv[3][1] = mInv[3][1], inv[3][2] = mInv[3][2], inv[3][3] = mInv[3][3];
	}

	void assign(
		const F& e00, const F& e01, const F& e02, const F& e03,
		const F& e10, const F& e11, const F& e12, const F& e13,
		const F& e20, const F& e21, const F& e22, const F& e23,
		const F& e30, const F& e31, const F& e32, const F& e33)
	{
		e[0][0] = e00, e[0][1] = e01, e[0][2] = e02, e[0][3] = e03;
		e[1][0] = e10, e[1][1] = e11, e[1][2] = e12, e[1][3] = e13;
		e[2][0] = e20, e[2][1] = e21, e[2][2] = e22, e[2][3] = e23;
		e[3][0] = e30, e[3][1] = e31, e[3][2] = e32, e[3][3] = e33;

		calculateInverse();
	}

	const F& operator () (int row, int col) const
	{
		assert(row >= 0 && row < 4 && col >=0 && col < 4);
		return e[row][col];
	}

	F& operator () (int row, int col)
	{
		assert(row >= 0 && row < 4 && col >=0 && col < 4);
		return e[row][col];
	}

	TransformT<F> getInverse() {
		return TransformT<F>(inv, e);
	}

	void calculateInverse() {
		using namespace std;

		F a[4][4];
		F b[4][4];

		a[0][0] = e[0][0], a[0][1] = e[0][1], a[0][2] = e[0][2], a[0][3] = e[0][3];
		a[1][0] = e[1][0], a[1][1] = e[1][1], a[1][2] = e[1][2], a[1][3] = e[1][3];
		a[2][0] = e[2][0], a[2][1] = e[2][1], a[2][2] = e[2][2], a[2][3] = e[2][3];
		a[3][0] = e[3][0], a[3][1] = e[3][1], a[3][2] = e[3][2], a[3][3] = e[3][3];

		b[0][0] = 1, b[0][1] = 0, b[0][2] = 0, b[0][3] = 0;
		b[1][0] = 0, b[1][1] = 1, b[1][2] = 0, b[1][3] = 0;
		b[2][0] = 0, b[2][1] = 0, b[2][2] = 1, b[2][3] = 0;
		b[3][0] = 0, b[3][1] = 0, b[3][2] = 0, b[3][3] = 1;

		unsigned int r, c;
		unsigned int cc;
		unsigned int rowMax; // Points to max abs value row in this column
		unsigned int row;
		float tmp;

		// Go through columns
		for (c=0; c<4; c++)
		{

			// Find the row with max value in this column
			rowMax = c;
			for (r=c+1; r<4; r++)
			{
				if (fabs(a[c][r]) > fabs(a[c][rowMax]))
				{
					rowMax = r;
				}
			}

			// If the max value here is 0, we can't invert.  Return identity.
			if (a[c][rowMax] == 0.0F) {
				inv[0][0] = 1, inv[0][1] = 0, inv[0][2] = 0, inv[0][3] = 0;
				inv[1][0] = 0, inv[1][1] = 1, inv[1][2] = 0, inv[1][3] = 0;
				inv[2][0] = 0, inv[2][1] = 0, inv[2][2] = 1, inv[2][3] = 0;
				inv[3][0] = 0, inv[3][1] = 0, inv[3][2] = 0, inv[3][3] = 1;

				cerr << "TransformT::calculateInverse(): inverse matrix does not exist." << endl;

				return;
			}

			// Swap row "rowMax" with row "c"
			for (cc=0; cc<4; cc++)
			{
				tmp = a[cc][c];
				a[cc][c] = a[cc][rowMax];
				a[cc][rowMax] = tmp;
				tmp = b[cc][c];
				b[cc][c] = b[cc][rowMax];
				b[cc][rowMax] = tmp;
			}

			// Now everything we do is on row "c".
			// Set the max cell to 1 by dividing the entire row by that value
			tmp = a[c][c];
			for (cc=0; cc<4; cc++)
			{
				a[cc][c] /= tmp;
				b[cc][c] /= tmp;
			}

			// Now do the other rows, so that this column only has a 1 and 0's
			for (row = 0; row < 4; row++)
			{
				if (row != c)
				{
					tmp = a[c][row];
					for (cc=0; cc<4; cc++)
					{
						a[cc][row] -= a[cc][c] * tmp;
						b[cc][row] -= b[cc][c] * tmp;
					}
				}
			}

		}

		inv[0][0] = b[0][0], inv[0][1] = b[0][1], inv[0][2] = b[0][2], inv[0][3] = b[0][3];
		inv[1][0] = b[1][0], inv[1][1] = b[1][1], inv[1][2] = b[1][2], inv[1][3] = b[1][3];
		inv[2][0] = b[2][0], inv[2][1] = b[2][1], inv[2][2] = b[2][2], inv[2][3] = b[2][3];
		inv[3][0] = b[3][0], inv[3][1] = b[3][1], inv[3][2] = b[3][2], inv[3][3] = b[3][3];

	}

	static TransformT<F> Identity()
	{
		static TransformT<F> i(
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1);
		return i;
	}

	VecT<F> operator() (const VecT<F>& v) const
	{
		return VecT<F>(
			e[0][0] * v.x + e[0][1] * v.y + e[0][2] * v.z,
			e[1][0] * v.x + e[1][1] * v.y + e[1][2] * v.z,
			e[2][0] * v.x + e[2][1] * v.y + e[2][2] * v.z);
	}

	PointT<F> operator() (const PointT<F>& p) const
	{
		F w = e[3][0] * p.x + e[3][1] * p.y + e[3][2] * p.z + e[3][3];
		assert(w != 0);
		if (w != 1.f)
			w = 1.f/w;
		PointT<F> q(
			w * (e[0][0] * p.x + e[0][1] * p.y + e[0][2] * p.z + e[0][3]),
			w * (e[1][0] * p.x + e[1][1] * p.y + e[1][2] * p.z + e[1][3]),
			w * (e[2][0] * p.x + e[2][1] * p.y + e[2][2] * p.z + e[2][3]));
		return q;
	}

	Ray operator() (const Ray &r) const {
		Ray ray = r;

		ray.o = (*this)(r.o);
		ray.d = (*this)(r.d);

		return ray;
	}

	TransformT<F> operator * (const TransformT<F>& t) const
	{
		TransformT<F> r;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j)
			{
				r.e[i][j] = 0;
				r.inv[i][j] = 0;
				for (int k = 0; k < 4; ++k)
				{
					r.e[i][j] += e[i][k] * t.e[k][j];
					r.inv[i][j] += t.inv[i][k] * inv[k][j];
				}
			}
		}
		return r;
	}

};

template<typename F>
inline void Transpose(F m[4][4], F t[4][4]) {
	t[0][0] = m[0][0], t[0][1] = m[1][0], t[0][2] = m[2][0], t[0][3] = m[3][0];
	t[1][0] = m[0][1], t[1][1] = m[1][1], t[1][2] = m[2][1], t[1][3] = m[3][1];
	t[2][0] = m[0][2], t[2][1] = m[1][2], t[2][2] = m[2][2], t[2][3] = m[3][2];
	t[3][0] = m[0][3], t[3][1] = m[1][3], t[3][2] = m[2][3], t[3][3] = m[3][3];
}

template<typename F> inline 
TransformT<F> Inverse(const TransformT<F> &T) {
	TransformT<F> M;

	M.e[0][0] = T.inv[0][0], M.e[0][1] = T.inv[0][1], M.e[0][2] = T.inv[0][2], M.e[0][3] = T.inv[0][3];
	M.e[1][0] = T.inv[1][0], M.e[1][1] = T.inv[1][1], M.e[1][2] = T.inv[1][2], M.e[1][3] = T.inv[1][3];
	M.e[2][0] = T.inv[2][0], M.e[2][1] = T.inv[2][1], M.e[2][2] = T.inv[2][2], M.e[2][3] = T.inv[2][3];
	M.e[3][0] = T.inv[3][0], M.e[3][1] = T.inv[3][1], M.e[3][2] = T.inv[3][2], M.e[3][3] = T.inv[3][3];

	M.inv[0][0] = T.e[0][0], M.inv[0][1] = T.e[0][1], M.inv[0][2] = T.e[0][2], M.inv[0][3] = T.e[0][3];
	M.inv[1][0] = T.e[1][0], M.inv[1][1] = T.e[1][1], M.inv[1][2] = T.e[1][2], M.inv[1][3] = T.e[1][3];
	M.inv[2][0] = T.e[2][0], M.inv[2][1] = T.e[2][1], M.inv[2][2] = T.e[2][2], M.inv[2][3] = T.e[2][3];
	M.inv[3][0] = T.e[3][0], M.inv[3][1] = T.e[3][1], M.inv[3][2] = T.e[3][2], M.inv[3][3] = T.e[3][3];

	return M;
}

template<typename F> inline 
TransformT<F> LookAt(const PointT<F> &pos, const PointT<F> &look, const VecT<F> &up) {
	F m[4][4];

	VecT<F> dir   = Normalize(look - pos);
	VecT<F> right = Cross(dir, Normalize(up));
	VecT<F> newUp = Cross(right, dir);

	m[0][0] = right.x;
	m[1][0] = right.y;
	m[2][0] = right.z;
	m[3][0] = 0.;

	m[0][1] = newUp.x;
	m[1][1] = newUp.y;
	m[2][1] = newUp.z;
	m[3][1] = 0.;

	m[0][2] = -dir.x;
	m[1][2] = -dir.y;
	m[2][2] = -dir.z;
	m[3][2] = 0.;

	m[0][3] = pos.x;
	m[1][3] = pos.y;
	m[2][3] = pos.z;
	m[3][3] = 1;

	return TransformT<F>(m).getInverse();
}

template<typename F> inline 
TransformT<F> Translate(VecT<F> t) {
	F m[4][4], inv[4][4];

	m[0][0] = 1, m[0][1] = 0, m[0][2] = 0, m[0][3] = t.x;
	m[1][0] = 0, m[1][1] = 1, m[1][2] = 0, m[1][3] = t.y;
	m[2][0] = 0, m[2][1] = 0, m[2][2] = 1, m[2][3] = t.z;
	m[3][0] = 0, m[3][1] = 0, m[3][2] = 0, m[3][3] = 1;

	inv[0][0] = 1, inv[0][1] = 0, inv[0][2] = 0, inv[0][3] = -t.x;
	inv[1][0] = 0, inv[1][1] = 1, inv[1][2] = 0, inv[1][3] = -t.y;
	inv[2][0] = 0, inv[2][1] = 0, inv[2][2] = 1, inv[2][3] = -t.z;
	inv[3][0] = 0, inv[3][1] = 0, inv[3][2] = 0, inv[3][3] = 1;

	return TransformT<F>(m, inv);
}

template<typename F> inline 
TransformT<F> Rotate(float degrees, const VecT<F> &axis) {
	Vec a = Normalize(axis);
	float sine = sinf(DegToRad(degrees));
	float cosine = cosf(DegToRad(degrees));
	
	F m[4][4], inv[4][4];

	m[0][0] = a.x * a.x + (1.f - a.x * a.x) * cosine;
	m[0][1] = a.x * a.y * (1.f - cosine) - a.z * sine;
	m[0][2] = a.x * a.z * (1.f - cosine) + a.y * sine;
	m[0][3] = 0;

	m[1][0] = a.x * a.y * (1.f - cosine) + a.z * sine;
	m[1][1] = a.y * a.y + (1.f - a.y * a.y) * cosine;
	m[1][2] = a.y * a.z * (1.f - cosine) - a.x * sine;
	m[1][3] = 0;

	m[2][0] = a.x * a.z * (1.f - cosine) - a.y * sine;
	m[2][1] = a.y * a.z * (1.f - cosine) + a.x * sine;
	m[2][2] = a.z * a.z + (1.f - a.z * a.z) * cosine;
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;

	Transpose(m, inv);

	return TransformT<F>(m, inv);
}

template<typename F>
inline TransformT<F> Scale(F x, F y, F z) {
	F m[4][4], inv[4][4];

	m[0][0] = x, m[0][1] = 0, m[0][2] = 0, m[0][3] = 0;
	m[1][0] = 0, m[1][1] = y, m[1][2] = 0, m[1][3] = 0;
	m[2][0] = 0, m[2][1] = 0, m[2][2] = z, m[2][3] = 0;
	m[3][0] = 0, m[3][1] = 0, m[3][2] = 0, m[3][3] = 1;

	inv[0][0] = 1 / x, inv[0][1] = 0,     inv[0][2] = 0,     inv[0][3] = 0;
	inv[1][0] = 0,     inv[1][1] = 1 / y, inv[1][2] = 0,     inv[1][3] = 0;
	inv[2][0] = 0,     inv[2][1] = 0,     inv[2][2] = 1 / z, inv[2][3] = 0;
	inv[3][0] = 0,     inv[3][1] = 0,     inv[3][2] = 0,     inv[3][3] = 1;

	return TransformT<F>(m, inv);
}

template<typename F> inline 
Transform Perspective(F fov, F n, F f) {
	// Perform projective divide
	F invDenom = 1.f/(f-n);
	F persp[4][4];

	persp[0][0] = 1, persp[0][1] = 0, persp[0][2] = 0,            persp[0][3] = 0;
	persp[1][0] = 0, persp[1][1] = 1, persp[1][2] = 0,            persp[1][3] = 0;
	persp[2][0] = 0, persp[2][1] = 0, persp[2][2] = f * invDenom, persp[2][3] = -f * n * invDenom;
	persp[3][0] = 0, persp[3][1] = 0, persp[3][2] = 1,            persp[3][3] = 0;

	// Scale to canonical viewing volume
	float invTanAng = 1.f / tanf(DegToRad(fov) / 2.f);
	return Scale(invTanAng, invTanAng, 1.0f) * Transform(persp);
}

template<typename F>
inline ostream &operator<< (ostream &o, const TransformT<F> &T) {
	o << T.e[0][0] << "\t" << T.e[0][1] << "\t" << T.e[0][2] << "\t" << T.e[0][3] << "\n";
	o << T.e[1][0] << "\t" << T.e[1][1] << "\t" << T.e[1][2] << "\t" << T.e[1][3] << "\n";
	o << T.e[2][0] << "\t" << T.e[2][1] << "\t" << T.e[2][2] << "\t" << T.e[2][3] << "\n";
	o << T.e[3][0] << "\t" << T.e[3][1] << "\t" << T.e[3][2] << "\t" << T.e[3][3] << "\n\n";

	o << T.inv[0][0] << "\t" << T.inv[0][1] << "\t" << T.inv[0][2] << "\t" << T.inv[0][3] << "\n";
	o << T.inv[1][0] << "\t" << T.inv[1][1] << "\t" << T.inv[1][2] << "\t" << T.inv[1][3] << "\n";
	o << T.inv[2][0] << "\t" << T.inv[2][1] << "\t" << T.inv[2][2] << "\t" << T.inv[2][3] << "\n";
	o << T.inv[3][0] << "\t" << T.inv[3][1] << "\t" << T.inv[3][2] << "\t" << T.inv[3][3] << "\n" << endl;

	return o;
}

#endif