#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "../raytrace.h"
#include "../BRDF/BRDF.h"

#include "Point.h"
#include "Vec.h"
#include "BBox.h"
#include "Normal.h"
#include "Transform.h"
#include "Intersection.h"

class Shape {
public:
	BRDF brdf;
	Transform T;
	BBox box;

	virtual bool intersect(Ray &r, Intersection &inter) const = 0;
	virtual void calcShadingParam(const Intersection &inter, const Camera &cam, ShadingParam& sp) const = 0;

private:

};


class TriMesh : public Shape {
public:
	struct Vertex {
		Point pos;
		Normal normal;
	};

	struct Face {
		int vertexId[3];
		int texCoordId[3];
	};

	struct TexCoord {
		float texCoord[2];
	};

	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::vector<TexCoord> texCoords;

	TriMesh() {}

	~TriMesh() {}

	bool intersect(Ray &r, Intersection &inter) const;

	bool intersectFace(Ray &r, Intersection &inter, int face) const;

	void calcShadingParam(const Intersection &inter, const Camera &cam, ShadingParam &sp) const;

private:

};

class Triangle : public Shape {
public:
	TriMesh *mesh;
	unsigned int face;

	Triangle() {
		mesh = 0;
		face = 0;
	}

	Triangle(TriMesh *m, unsigned int f) {
		assign(m, f);
	}

	~Triangle() {}

	bool intersect(Ray &r, Intersection &inter) const;

	void calcShadingParam(const Intersection &inter, const Camera &cam, ShadingParam &sp) const;

	void assign(TriMesh *m, unsigned int f) {
		mesh = m;
		face = f;

		box = Union( BBox( mesh->vertices[mesh->faces[face].vertexId[0]].pos, 
						   mesh->vertices[mesh->faces[face].vertexId[1]].pos ), 
					 mesh->vertices[mesh->faces[face].vertexId[2]].pos );
		
		//Pad the bounding box of the triangle to avoid voxelization degeneracies
		float eps = Length(box.pMax - box.pMin) * 0.001f;
		Vec vEps(eps, eps, eps);
		box = Union(box, (box.pMin - vEps));
		box = Union(box, (box.pMax + vEps));
	}

private:

};

#endif //_GEOMETRY_H