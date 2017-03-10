#pragma once

#include "Types.h"
#include "DebugPrint.h"

template <typename T>
struct Quaternion;

/*
 * an N*N matrix
 */
template <typename T, size_t N>
class Matrix
{
    template <typename U, size_t M>
    friend Matrix<U, M> operator*(const Matrix<U, M>& lhs, const Matrix<U, M>& rhs);
    template <typename U>
    friend Quaternion<U> operator*(const Matrix<U, 4>& lhs, const Quaternion<U>& rhs);

public:
    Matrix() : m_val{ } { }
    Matrix(T (&val)[N][N])  // implicit constructor
    {
        for (auto i = 0; i < N; ++i)
            for (auto j = 0; j < N; ++j)
                m_val[i][j] = val[i][j];
    }

private:
    T m_val[N][N];
};

template <typename T, size_t N>
Matrix<T, N> operator*(const Matrix<T, N> &lhs, const Matrix<T, N> &rhs)
{
    Matrix<T, N> m{ };
    for (auto i = 0; i < N; ++i)
        for (auto j = 0; j < N; ++j)
            for (auto k = 0; k < N; ++k)
            {
                m.m_val[i][j] += lhs.m_val[i][k] * rhs.m_val[k][j];
            }
    return m;
}

template <typename T>
Quaternion<T> operator*(const Matrix<T, 4> &lhs, const Quaternion<T> &rhs)
{
    Quaternion<T> q{ };
    for (auto i = 0; i < 4; ++i)
        for (auto j = 0; j < 4; ++j)
        {
            q.val[i] += lhs.m_val[i][j] * rhs.val[j];
        }
    return q;
}

template <typename T>
using Matrix4x4 = Matrix<T, 4>;

using Matrix4x4R = Matrix4x4<REAL>;
using Matrix4x4I = Matrix4x4<INT32>;

template <typename T>
using Matrix3x3 = Matrix<T, 3>;

using Matrix3x3R = Matrix3x3<REAL>;
using Matrix3x3I = Matrix3x3<INT32>;