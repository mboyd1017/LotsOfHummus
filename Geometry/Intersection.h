#ifndef _INTERSECTION_H
#define _INTERSECTION_H

#include "Point.h"
#include "Normal.h"

class Shape;

struct Intersection {
	const Shape *object;
	Point p;
	Normal n;
	float texCoord[2];

	int triInt;

	union
	{
		struct {
			int face;
			float u, v;
		} triMesh;
	} objectInfo; // the object should be able to 

public:
	Intersection() {
		object = NULL;
		triInt = -1;
		objectInfo.triMesh.face = -1;
	}
};

#endif //_INTERSECTION_H