#pragma once

#include "Types.h"

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