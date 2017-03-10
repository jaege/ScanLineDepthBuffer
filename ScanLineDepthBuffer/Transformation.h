#pragma once

#include "Tuple.h"
#include "Matrix.h"

class Transformation
{
public:
    static Matrix4x4R Translate(Vector3R v);
    static Matrix4x4R Translate(REAL x, REAL y, REAL z);

    //static Matrix4x4R Rotate(Vector3R axis, REAL angle);

    static Matrix4x4R RotateAboutXAxis(REAL angle);
    static Matrix4x4R RotateAboutYAxis(REAL angle);

    static Matrix4x4R Scale(REAL s);
    static Matrix4x4R Scale(Vector3R v);
    static Matrix4x4R Scale(REAL sx, REAL sy, REAL sz);

    static Matrix4x4R Symmetry(bool xoy, bool yoz, bool xoz);
};