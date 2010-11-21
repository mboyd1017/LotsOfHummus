#include "Ray.h"

#include "Geometry.h"

#include "../Statistics/Statistics.h"


Ray::Ray() {
	//Stats()->incrNumRays();

	t = INFINITY;

	triInt = -1;

	tmin = EPS;
	tmax = INFINITY;
}

Ray::Ray(Point orig, Vec dir) {
	Stats()->incrNumRays();

	o = orig;
	d = dir;

	t = INFINITY;
	tmin = EPS;
	tmax = INFINITY;

	triInt = -1;

}

ostream &operator<< (ostream &o, const Ray &r) {
    o << "o = " << r.o << ", d = " << r.d << ", t = " << r.t;
	return o;
}