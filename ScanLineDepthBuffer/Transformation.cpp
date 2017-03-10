#include <cmath>
#include "Transformation.h"

Matrix4x4R Transformation::Translate(Vector3R v)
{
    REAL t[4][4] = {{1, 0, 0, v.x},
                    {0, 1, 0, v.y},
                    {0, 0, 1, v.z},
                    {0, 0, 0,   1}};
    return t;
}

Matrix4x4R Transformation::Translate(REAL x, REAL y, REAL z)
{
    REAL t[4][4] = {{1, 0, 0, x},
                    {0, 1, 0, y},
                    {0, 0, 1, z},
                    {0, 0, 0, 1}};
    return t;
}

//Matrix4x4R Transformation::Rotate(Vector3R axis, REAL angle)
//{
//    REAL lentr = 1.0f / std::sqrt(axis.y * axis.y + axis.z * axis.z);
//    REAL len = 1.0f / std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
//    REAL t[4][4] = {{0, 0, 0, 0},
//                    {0, 0, 0, 0},
//                    {0, 0, 0, 0},
//                    {0, 0, 0, 1}};
//    return t;
//}

Matrix4x4R Transformation::RotateAboutXAxis(REAL angle)
{
    constexpr REAL PI = 3.14159265358979323846f;
    REAL theta = angle * PI / 180;
    REAL costheta = std::cos(theta);
    REAL sintheta = std::sin(theta);
    REAL t[4][4] = {{1, 0, 0, 0},
                    {0, costheta, sintheta, 0},
                    {0, -sintheta, costheta, 0},
                    {0, 0, 0, 1}};
    return t;
}

Matrix4x4R Transformation::RotateAboutYAxis(REAL angle)
{
    constexpr REAL PI = 3.14159265358979323846f;
    REAL theta = angle * PI / 180;
    REAL costheta = std::cos(theta);
    REAL sintheta = std::sin(theta);
    REAL t[4][4] = {{costheta, 0, -sintheta, 0},
                    {0, 1, 0, 0},
                    {sintheta, 0, costheta, 0},
                    {0, 0, 0, 1}};
    return t;
}

Matrix4x4R Transformation::Scale(REAL s)
{
    REAL t[4][4] = {{s, 0, 0, 0},
                    {0, s, 0, 0},
                    {0, 0, s, 0},
                    {0, 0, 0, 1}};
    return t;
}

Matrix4x4R Transformation::Scale(Vector3R v)
{
    REAL t[4][4] = {{v.x,   0,   0, 0},
                    {  0, v.y,   0, 0},
                    {  0,   0, v.z, 0},
                    {  0,   0,   0, 1}};
    return t;
}

Matrix4x4R Transformation::Scale(REAL sx, REAL sy, REAL sz)
{
    REAL t[4][4] = {{sx,  0,  0, 0},
                    { 0, sy,  0, 0},
                    { 0,  0, sz, 0},
                    { 0,  0,  0, 1}};
    return t;
}

Matrix4x4R Transformation::Symmetry(bool xoy, bool yoz, bool xoz)
{
    REAL x = yoz ? -1.0f : 1.0f;
    REAL y = xoz ? -1.0f : 1.0f;
    REAL z = xoy ? -1.0f : 1.0f;
    REAL t[4][4] = {{x, 0, 0, 0},
                    {0, y, 0, 0},
                    {0, 0, z, 0},
                    {0, 0, 0, 1}};
    return t;
}
