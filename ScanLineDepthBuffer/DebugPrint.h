#pragma once

// TODO(jaege): consider rewrite DebugPrint with intializer_list.

// TODO(jaege): consider add debug chanel or debug level support.

// TODO(jaege): consider add error code support.
//class ErrorCode
//{
//public:
//    ErrorCode(int code);
//    std::string msg() const;
//private:
//    // Some code<->message tables like
//    //     MSG1   0x01
//    //     MSG2   0x02  ...
//};
//DebugPrint(ErrorCode(45), ...);
//DebugPrint(ErrorCode(ErrorCode::SOME_MESSAGE_NAME), ...);
//DebugPrint(ErrorCode::SOME_MESSAGE_NAME, ...);

#ifdef NDEBUG

#define DebugPrint(format, ...) ((void)0)

#else

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

#endif // !NDEBUG