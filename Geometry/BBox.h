#ifndef _BBOX_H_
#define _BBOX_H_

#include "../raytrace.h"
#include "Point.h"

class BBox {
public:
	Point pMin, pMax;

	BBox() : pMin(INFINITY, INFINITY, INFINITY), pMax(-INFINITY, -INFINITY, -INFINITY) {}
	BBox(const Point p) : pMin(p), pMax(p) {}
	BBox(const Point min, const Point max) {
		pMin = Point( __min( min.x, max.x ),
					  __min( min.y, max.y ),
					  __min( min.z, max.z ) );

		pMax = Point( __max( min.x, max.x ),
					  __max( min.y, max.y ),
					  __max( min.z, max.z ) );
	}

	bool contains(const BBox &b) const {
		return b.pMin.x > pMin.x && b.pMin.y > pMin.y && b.pMin.z > pMin.z && 
			   b.pMax.x < pMax.x && b.pMax.y < pMax.y && b.pMax.z < pMax.z;
	}
};

inline BBox Union(const BBox &b, const Point &p) {
	BBox box;

	box.pMin.x = __min(b.pMin.x, p.x);
	box.pMin.y = __min(b.pMin.y, p.y);
	box.pMin.z = __min(b.pMin.z, p.z);

	box.pMax.x = __max(b.pMax.x, p.x);
	box.pMax.y = __max(b.pMax.y, p.y);
	box.pMax.z = __max(b.pMax.z, p.z);

	return box;
}

inline BBox Union(const BBox &b1, const BBox &b2) {
	BBox box;

	box.pMin.x = __min(b1.pMin.x, b2.pMin.x);
	box.pMin.y = __min(b1.pMin.y, b2.pMin.y);
	box.pMin.z = __min(b1.pMin.z, b2.pMin.z);

	box.pMax.x = __max(b1.pMax.x, b2.pMax.x);
	box.pMax.y = __max(b1.pMax.y, b2.pMax.y);
	box.pMax.z = __max(b1.pMax.z, b2.pMax.z);

	return box;
}

#endif