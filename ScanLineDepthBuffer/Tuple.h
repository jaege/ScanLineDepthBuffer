#pragma once

#include <initializer_list>
#include "Types.h"
#include "DebugPrint.h"

template <typename T>
struct Ternion
{
    using value_type = T;
    T x;
    T y;
    T z;
};

using Position3R = Ternion<REAL>;
using Position3I = Ternion<INT32>;
using Vector3R = Ternion<REAL>;

template <typename T, size_t N>
class Tuple
{
public:
    using value_type = T;
    Tuple() : val{ } { }
    template <typename U>
    Tuple(std::initializer_list<U> il)
    {
        if (il.size() != N)
        {
            DebugPrint(L"[ERR] Tuple size mismatch.");
        }
        int i = 0;
        for (auto it = il.begin(); it != il.end() && i < N; ++it, ++i)
            val[i] = *it;
    }
protected:
    T val[N];
};

//template <typename T>
//struct Ternion : public Tuple<T, 3>
//{
//    T &x = val[0];
//    T &y = val[1];
//    T &z = val[2];
//};

template <typename T, size_t N>
class Matrix;

template <typename T>
struct Quaternion : public Tuple<T, 4>
{
    template <typename U>
    friend Quaternion<U> operator*(const Matrix<U, 4> & lhs,
                                   const Quaternion<U> & rhs);

    using Tuple::Tuple;
    T &x = val[0];
    T &y = val[1];
    T &z = val[2];
    T &w = val[3];
};

using Vector4R = Quaternion<REAL>;