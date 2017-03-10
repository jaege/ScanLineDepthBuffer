#include <stdio.h>
#include <wchar.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "DebugPrint.h"

static constexpr int MAX_ERROR_MESSAGE_LENGTH = 1024;

int DebugPrintVFA(const char * format, va_list argList)
{
    static char s_buffer[MAX_ERROR_MESSAGE_LENGTH];

    int ret = vsnprintf(s_buffer, MAX_ERROR_MESSAGE_LENGTH, format, argList);
    OutputDebugStringA(s_buffer);
    return ret;
}

int DebugPrintVFW(const wchar_t * format, va_list argList)
{
    static wchar_t s_buffer[MAX_ERROR_MESSAGE_LENGTH];

    int ret = vswprintf(s_buffer, MAX_ERROR_MESSAGE_LENGTH, format, argList);
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

int DebugPrintFW(const wchar_t * format, ...)
{
    va_list argList;
    va_start(argList, format);
    int ret = DebugPrintVFW(format, argList);
    va_end(argList);
    return ret;
}