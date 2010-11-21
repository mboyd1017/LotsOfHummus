#ifndef _NORMAL_H_
#define _NORMAL_H_

#include "Vec.h"

// A normal vector n satisfiess that if <n, v> = 0 then <T(n), T(v)> = 0, 
// where <-,-> is the inner product, v is a vector, and T is a linear
// transformation. In practice this means that T(n) = transp(inv(T)) * n
// if we treat T as square matrix and n as a column matrix.
// However the linear transformations we're going to deal with
// when working with normals are rotations and uniform-scaling, i.e.,
// T such that T = transp(inv(T)).
// Hence, we will just define Normal{f|d} as conventional vectors.


typedef VecT<float> Normal;

#endif