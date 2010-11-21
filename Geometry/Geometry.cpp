#include "Geometry.h"
#include "Ray.h"

#include "../Statistics/Statistics.h"


bool 
TriMesh::intersect(Ray &r, Intersection &inter) const {
	bool hasIntersection = false;

	for (unsigned int i = 0; i < faces.size(); i++) {
		if (intersectFace(r, inter, i)) {
			hasIntersection = true;
		}
	}

	return hasIntersection;
}

typedef VecT<double> Vecd;
bool 
TriMesh::intersectFace(Ray &r, Intersection &inter, int face) const {
	Stats()->incrNumIntersectionTests();

	const Point& p1 = vertices[faces[face].vertexId[0]].pos;
	const Point& p2 = vertices[faces[face].vertexId[1]].pos;
	const Point& p3 = vertices[faces[face].vertexId[2]].pos;
	const Vecd d(r.d.x, r.d.y, r.d.z);

	Vecd e1 = Vecd(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	Vecd e2 = Vecd(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
	Vecd s1 = Cross(d, e2);
	double divisor = Dot(s1, e1);
	if (fabs(divisor) < 1e-20) {
		return false;
	}

	double invDivisor = 1.0 / divisor;

	Vecd s = Vecd(r.o.x - p1.x, r.o.y - p1.y, r.o.z - p1.z);
	double b1 = Dot(s, s1) * invDivisor;
	if (b1 < 0. || b1 > 1.) {
		return false;
	}

	Vecd s2 = Cross(s, e1);
	double b2 = Dot(d, s2) * invDivisor;
	if (b2 < 0. || b1+b2 > 1.) {
		return false;
	}

	double t = Dot(e2, s2) * invDivisor;
	if (t < r.tmin || t > r.tmax || t > r.t) {
		return false;
	}

#ifdef BACKFACE_CULL
	Normal n = Normalize(b1 * vertices[faces[face].vertexId[1]].normal +
						 b2 * vertices[faces[face].vertexId[2]].normal + 
						 (1 - b1 - b2) * vertices[faces[face].vertexId[0]].normal);

	Vec dir = Normalize(r.d);
	if (Dot(dir, n) > 0.0001f) {
		return false;
	}

	inter.n = n;
#else
	inter.n = Normalize((float)b1 * vertices[faces[face].vertexId[1]].normal +
						(float)b2 * vertices[faces[face].vertexId[2]].normal + 
						(float)(1 - b1 - b2) * vertices[faces[face].vertexId[0]].normal);
#endif

	if (texCoords.size()) {
		inter.texCoord[0] = (float)(b1 * texCoords[faces[face].texCoordId[1]].texCoord[0] +
							b2 * texCoords[faces[face].texCoordId[2]].texCoord[0] + 
							(1 - b1 - b2) * texCoords[faces[face].texCoordId[0]].texCoord[0]);

		inter.texCoord[1] = (float)(b1 * texCoords[faces[face].texCoordId[1]].texCoord[1] +
							b2 * texCoords[faces[face].texCoordId[2]].texCoord[1] + 
							(1 - b1 - b2) * texCoords[faces[face].texCoordId[0]].texCoord[1]);
	}

	inter.object = this;
	r.t = (float)t;
	inter.p = r.o + (float)t * r.d;

	inter.objectInfo.triMesh.face = face;
	inter.objectInfo.triMesh.u = (float)b1;
	inter.objectInfo.triMesh.v = (float)b2;

	Stats()->incrNumIntersections();

	return true;
}

template<int R, int C>
class Mat
{
public:
	float e[R][C];

	float& operator () (int i, int j)	
	{
		assert(i >= 0 && i < R && j >= 0 && j < C);
		return e[i][j];
	}
};

template<int R1, int C1, int C2>
void mul(Mat<R1, C2>& out, const Mat<R1, C1>& a, const Mat<C1, C2>& b)
{
	for (int i = 0; i < R1; ++i)
	{
		for (int j = 0; j < C2; ++j)
		{
			out.e[i][j] = 0;
			for (int k = 0; k < C1; ++k)
				out.e[i][j] += a.e[i][k] * b.e[k][j];
		}
	}
}

void TriMesh::calcShadingParam(const Intersection &inter, const Camera &cam, ShadingParam &sp) const
{
	// No texture?
	if (texCoords.size() == 0)
	{
		sp.s = sp.t = 0;
		sp.dsdx = sp.dsdy = sp.dtdx = sp.dtdy = 0;
		return;
	}

	// Calculate derivatives: we have several transforms here:
	// T: texture space (s, t) -> parameter space (u, v)
	// S: parameter space (u, v) -> world space (x, y, z)
	// W: world space (x, y, z) -> camera space (x', y', z')
	// P: camera space (x', y', z') -> unit projection space (p, q)
	// P': unit post-projection space (p, q) -> device scaled post-project space (p', q')
	//
	// Want to calculate D[inv(P'PWST]] = 

#if 0 // not needed
	float u = inter.objectInfo.triMesh.u;
	float v = inter.objectInfo.triMesh.v;
#endif
	const Face& f= faces[inter.objectInfo.triMesh.face];
	Point p = cam.WorldToCam(inter.p);

	// Calculate D[inv(T)] where:
	//   (s, t) = inv(T)(u, v) = (s0 + u(s1-s0) + v(s2-s0), t0 + u(t1-t0) + v(t2-t0))
	// so D[inv(T)] = {{s1-s0, s2-s0}, {t1-t0, t2-t0}}.
	Mat<2, 2> DTi;
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			DTi(i, j) = texCoords[f.texCoordId[1 + j]].texCoord[i] - texCoords[f.texCoordId[0]].texCoord[i];

	// Calculate D[P']
	Mat<2, 2> DP2 = {{
		{cam.xfactor, 0},
		{0, cam.yfactor}
	}};

	// Calculate D[P]
	Mat<2, 3> DP = {{
		{-1.f/p.z, 0, p.x/(p.z*p.z)},
		{0, -1.f/p.z, p.y/(p.z*p.z)}
	}};

	// Calculate D[W]
	Mat<3, 3> DW;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			DW(i, j) = cam.WorldToCam(i, j);

	// Calculate D[S]
	Mat<3, 2> DS;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 2; ++j)
			DS(i, j) = vertices[f.vertexId[1 + j]].pos[i] - vertices[f.vertexId[0]].pos[i];


	// Calculate D[P'PWS] = D[P'] * D[P] * D[W] * D[S], and then invert to get D[inv(P'PWS)]
	Mat<2, 3> t1;
	mul(t1, DP2, DP);
	Mat<2, 3> t2;
	mul(t2, t1, DW);
	Mat<2, 2> t3;
	mul(t3, t2, DS);

	float invd = 1.f / (t3(0, 0) * t3(1, 1) - t3(0, 1) * t3(1, 0));
	Mat<2, 2> DPPWSi = {{
		{t3(1, 1) * invd, -t3(0, 1) * invd},
		{-t3(1, 0) * invd, t3(0, 0) * invd}
	}};

	// Multiply D[inv(P'PWS)] and D[inv(T)] to get D[inv(P'PWST)];
	Mat<2, 2> final;
	mul(final, DTi, DPPWSi);

	sp.s = inter.texCoord[0];
	sp.t = inter.texCoord[1];
	sp.dsdx = final(0, 0);
	sp.dsdy = final(0, 1);
	sp.dtdx = final(1, 0);
	sp.dtdy = final(1, 1);
}

bool 
Triangle::intersect(Ray &r, Intersection &inter) const {
	return mesh->intersectFace(r, inter, face);
}

void Triangle::calcShadingParam(const Intersection &inter, const Camera &cam, ShadingParam &sp) const
{
	return mesh->calcShadingParam(inter, cam, sp);
}

