#ifndef _RAY_H
#define _RAY_H

#include <iostream>

#include "Point.h"
#include "Vec.h"

using std::ostream;

//#ifndef INFINITY
//#define INFINITY FLT_MAX
//#endif //INFINITY

//#ifndef EPS
//#define EPS 0.00001f
//#endif //EPS

class Ray {
public:
	Point o;
	Vec d;
	float t;

	int triInt;

	float tmin, tmax;

	Ray();

	Ray(Point orig, Vec dir);

	inline Point operator() (float t) const {
		return (o + t * d);
	}
};

ostream &operator<< (ostream &o, const Ray &r);

#endif //_RAY_H