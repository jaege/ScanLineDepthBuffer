#ifndef UNICODE
#define UNICODE
#endif

#define ASSERTION_ENABLED

#include <Windows.h>
#include "DebugPrint.h"
#include "MainWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"扫描线 Z-Buffer 算法演示", WS_OVERLAPPEDWINDOW))
    {
        DebugPrint(L"[ERR] Fail to create MainWindow.");
        std::abort();
    }

    win.OpenObjFile();

    ShowWindow(win.Window(), nCmdShow);
    
    UpdateWindow(win.Window());

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}