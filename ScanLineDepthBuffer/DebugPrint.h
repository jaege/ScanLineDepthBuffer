#pragma once

int DebugPrintVFA(const char *format, va_list argList);

int DebugPrintVFW(const wchar_t *format, va_list argList);

#ifdef UNICODE
#define DebugPrintVF  DebugPrintVFW
#else
#define DebugPrintVF  DebugPrintVFA
#endif // !UNICODE

int DebugPrintFA(const char *format, ...);

int DebugPrintFW(const wchar_t *format, ...);

#ifdef UNICODE
#define DebugPrintF  DebugPrintFW
#else
#define DebugPrintF  DebugPrintFA
#endif // !UNICODE


#ifdef UNICODE
#define DebugPrint(format, ...) DebugPrintFW(format L"\n", __VA_ARGS__)
#else
#define DebugPrint(format, ...) DebugPrintFA(format "\n", __VA_ARGS__)
#endif // !UNICODE

//#define STRINGIZE_DETAIL(x) #x
//#define STRINGIZE(x) STRINGIZE_DETAIL(x)

//#ifdef UNICODE
//#define DebugPrint(format, ...) DebugPrintFW(__FILE__ L":" \
//    STRINGIZE(__LINE__) L" " format L"\n", __VA_ARGS__)
//#else
//#define DebugPrint(format, ...) DebugPrintFA(__FILE__ ":" \
//    STRINGIZE(__LINE__) " " format "\n", __VA_ARGS__)
//#endif // !UNICODE