#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include "MainWindow.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"…®√Ëœﬂ Z-Buffer À„∑®—› æ",
                    WS_OVERLAPPEDWINDOW))
    {
        OutputDebugString(L"Error: Fail to create MainWindow.");
        return 0;
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