#include <string>
#include <Windows.h>
#include <shobjidl.h> 
#include "MainWindow.h"
#include "DebugPrint.h"

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static REAL scaleFactor = 0.95f;
    constexpr REAL scaleFactorStep = 0.05f;
    static REAL degreeX = 0.0f;
    static REAL degreeY = 0.0f;
    constexpr REAL degreeStep = 5.0f;

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
            case L'z':
            case L'Z':
                {
                    if (scaleFactor < 50.0f) { scaleFactor += scaleFactorStep; }
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'c':
            case L'C':
                {
                    if (scaleFactor > 0.05f) { scaleFactor -= scaleFactorStep; }
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'j':
            case L'J':
                // Rotate object about y axis.
                {
                    degreeY -= degreeStep;
                    if (degreeY < -360) degreeY += 360;
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'l':
            case L'L':
                // Rotate object about y axis.
                {
                    degreeY += degreeStep;
                    if (degreeY > 360) degreeY -= 360;
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'i':
            case L'I':
                // Rotate object about x axis.
                {
                    degreeX += degreeStep;
                    if (degreeX > 360) degreeX -= 360;
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            case L'k':
            case L'K':
                // Rotate object about x axis.
                {
                    degreeX -= degreeStep;
                    if (degreeX < -360) degreeX += 360;
                    m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
                    m_objModel.SetBuffer(m_buffer);
                    InvalidateRect(m_hwnd, NULL, FALSE);
                }
                break;
            }
        }
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
/*
    case WM_LBUTTONDOWN:
        DebugPrint(L"WM_LBUTTONDOWN");
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    case WM_LBUTTONUP:
        DebugPrint(L"WM_LBUTTONUP");
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    case WM_RBUTTONDOWN:
        DebugPrint(L"WM_RBUTTONDOWN");
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

    case WM_RBUTTONUP:
        DebugPrint(L"WM_RBUTTONUP");
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
*/
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

            m_buffer.Resize(width, height);
            //m_buffer.DebugDarwRandomPicture();

            m_objModel.SetModelScale(m_buffer, scaleFactor, degreeX, degreeY);
            m_objModel.SetBuffer(m_buffer);
        }
        return 0;

    case WM_PAINT:
        {
            DebugPrint(L"WM_PAINT");
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);

            RECT rc;
            GetClientRect(m_hwnd, &rc);
            INT32 width = rc.right - rc.left;
            INT32 height = rc.bottom - rc.top;

            m_buffer.OnPaint(hdc, width, height);

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

                        const UINT32 MAX_CHARS = 1024;
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
