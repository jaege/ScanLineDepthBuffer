﻿#include <string>
#include <Windows.h>
#include <shobjidl.h>
#include <chrono>  // high_resolution_clock
using Clock = std::chrono::high_resolution_clock;
#include "MainWindow.h"
#include "DebugPrint.h"

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static OffscreenBuffer buffer;

    static REAL scaleFactor = 0.95f;
    constexpr REAL scaleFactorStep = 0.05f;

    static REAL degreeX = 0.0f;
    static REAL degreeY = 0.0f;
    constexpr REAL degreeStep = 5.0f;

    static REAL shiftX = 0.0f;
    static REAL shiftY = 0.0f;
    constexpr REAL shiftStep = 10.0f;

    switch (uMsg)
    {
    //case WM_CLOSE:
    //    if (MessageBox(m_hwnd, L"Really quit?", L"Quit", MB_OKCANCEL) == IDOK)
    //    {
    //        DestroyWindow(m_hwnd);
    //    }
    //    // Else: User canceled. Do nothing.
    //    return 0;

    case WM_CHAR:
        {
            wchar_t ch = static_cast<wchar_t>(wParam);
            DebugPrint(L"WM_CHAR: %c", ch);
            switch (ch)
            {
            case L'x': case L'X':
                // Reset object
                {
                    scaleFactor = 0.95f;
                    degreeX = 0.0f;
                    degreeY = 0.0f;
                    shiftX = 0.0f;
                    shiftY = 0.0f;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'z': case L'Z':
                // Zoom in
                {
                    if (scaleFactor < 50.0f) { scaleFactor += scaleFactorStep; }
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'c': case L'C':
                // Zoom out
                {
                    if (scaleFactor > 0.05f) { scaleFactor -= scaleFactorStep; }
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'j': case L'J':
                // Rotate object about y axis.
                {
                    degreeY -= degreeStep;
                    if (degreeY < -360) degreeY += 360;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'l': case L'L':
                // Rotate object about y axis.
                {
                    degreeY += degreeStep;
                    if (degreeY > 360) degreeY -= 360;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'i': case L'I':
                // Rotate object about x axis.
                {
                    degreeX += degreeStep;
                    if (degreeX > 360) degreeX -= 360;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'k': case L'K':
                // Rotate object about x axis.
                {
                    degreeX -= degreeStep;
                    if (degreeX < -360) degreeX += 360;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'a': case L'A':
                // Move object left.
                {
                    shiftX -= shiftStep;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'd': case L'D':
                // Move object right.
                {
                    shiftX += shiftStep;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'w': case L'W':
                // Move object up.
                {
                    shiftY -= shiftStep;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L's': case L'S':
                // Move object down.
                {
                    shiftY += shiftStep;
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            }
        }
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    //case WM_LBUTTONDOWN:
    //    DebugPrint(L"WM_LBUTTONDOWN");
    //    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    //case WM_LBUTTONUP:
    //    DebugPrint(L"WM_LBUTTONUP");
    //    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    //case WM_RBUTTONDOWN:
    //    DebugPrint(L"WM_RBUTTONDOWN");
    //    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    //case WM_RBUTTONUP:
    //    DebugPrint(L"WM_RBUTTONUP");
    //    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    case WM_DESTROY:
        DebugPrint(L"WM_DESTROY");
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        {
            DebugPrint(L"WM_SIZE");
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            INT32 width = rc.right - rc.left;
            INT32 height = rc.bottom - rc.top;
            buffer.Resize(width, height);
        }
        return 0;

    case WM_PAINT:
        {
            DebugPrint(L"WM_PAINT");
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);

            //m_buffer.DebugDarwRandomPicture();

            auto t1 = Clock::now();
            m_objModel.GetBuffer(buffer, scaleFactor, degreeX, degreeY, shiftX, shiftY);
            auto t2 = Clock::now();
            REAL deltaT = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000.0f;

            RECT rc;
            GetClientRect(m_hwnd, &rc);
            INT32 width = rc.right - rc.left;
            INT32 height = rc.bottom - rc.top;
            buffer.OnPaint(hdc, width, height);

            rc.top += 10;
            rc.left += 10;
            rc.right -= 10;
            SetTextColor(hdc, Color::WHITE.GetColorCode());
            SetBkMode(hdc, TRANSPARENT);
            constexpr WCHAR *description = L"W A S D: move\nI J K L: rotate\n"
                                           L"Z C: zoom\nX: reset";
            DrawText(hdc, description, -1, &rc, DT_TOP | DT_LEFT | DT_NOCLIP);

            constexpr UINT32 MAX_CHARS = 100;
            WCHAR strbuf[MAX_CHARS];
            swprintf(strbuf, MAX_CHARS, L"%.3f ms\n%.3f fps", deltaT, 1000.0f / deltaT);
            DrawText(hdc, strbuf, -1, &rc, DT_TOP | DT_RIGHT | DT_NOCLIP);

            EndPaint(m_hwnd, &ps);
        }
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void MainWindow::OpenObjFile()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                                COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pFileOpen;

        hr = CoCreateInstance(__uuidof(FileOpenDialog), NULL, CLSCTX_ALL,
                              IID_PPV_ARGS(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            COMDLG_FILTERSPEC filter = {L"Obj files (*.obj)", L"*.obj"};
            pFileOpen->SetFileTypes(1, &filter);

            pFileOpen->SetTitle(L"请选择要打开的 obj 文件");

            // BUG(jaege): The following line is very slow. It takes about
            //     5 seconds after choose file in the dialog to continue in
            //     debug compliation.
            hr = pFileOpen->Show(NULL);

            if (SUCCEEDED(hr))
            {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr))
                    {
                        DebugPrint(L"[INF] Open obj file: %s", pszFilePath);

                        m_objModel.LoadFromObjFile(pszFilePath);

                        constexpr UINT32 MAX_CHARS = 1024;
                        WCHAR s_buffer[MAX_CHARS];
                        WCHAR windowTitle[MAX_CHARS];
                        GetWindowText(m_hwnd, windowTitle,
                                      GetWindowTextLength(m_hwnd) + 1);
                        swprintf(s_buffer, MAX_CHARS, L"%s - %s",
                                 windowTitle, pszFilePath);
                        SetWindowText(m_hwnd, s_buffer);

                        CoTaskMemFree(pszFilePath);
                    }
                    else
                    {
                        DebugPrint(L"[WRN] pItem->GetDisplayName Failed.");
                        std::abort();
                    }
                    pItem->Release();
                }
                else
                {
                    DebugPrint(L"[WRN] pFileOpen->GetResult Failed.");
                    std::abort();
                }
            }
            else
            {
                DebugPrint(L"[WRN] pFileOpen->Show Failed.");
                std::abort();
            }
            pFileOpen->Release();
        }
        else
        {
            DebugPrint(L"[WRN] CoCreateInstance Failed.");
            std::abort();
        }
        CoUninitialize();
    }
    else
    {
        DebugPrint(L"[WRN] CoInitializeEx Failed.");
        std::abort();
    }
}