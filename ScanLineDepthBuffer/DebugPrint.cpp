#include <stdio.h>
#include <wchar.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "DebugPrint.h"

int DebugPrintVFA(const char *format, va_list argList)
{
    const UINT32 MAX_CHARS = 1024;
    static char s_buffer[MAX_CHARS];

    int ret = vsnprintf(s_buffer, MAX_CHARS, format, argList);
    OutputDebugStringA(s_buffer);
    return ret;
}

int DebugPrintVFW(const wchar_t *format, va_list argList)
{
    const UINT32 MAX_CHARS = 1024;
    static wchar_t s_buffer[MAX_CHARS];

    int ret = vswprintf(s_buffer, MAX_CHARS, format, argList);
    OutputDebugStringW(s_buffer);
    return ret;
}

int DebugPrintFA(const char * format, ...)
{
    va_list argList;
    va_start(argList, format);
    int ret = DebugPrintVFA(format, argList);
    va_end(argList);
    return ret;
}

int DebugPrintFW(const wchar_t *format, ...)
{
    va_list argList;
    va_start(argList, format);
    int ret = DebugPrintVFW(format, argList);
    va_end(argList);
    return ret;
}