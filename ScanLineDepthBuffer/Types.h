#pragma once

typedef unsigned char UINT8;
typedef unsigned int UINT32;

typedef signed int INT32;

#ifndef DOUBLE_PRECISION
typedef float REAL;
#define REAL_MAX FLT_MAX
#define REAL_MIN FLT_MIN
#else
typedef double REAL;
#define REAL_MAX DBL_MAX
#define REAL_MIN DBL_MIN
#endif