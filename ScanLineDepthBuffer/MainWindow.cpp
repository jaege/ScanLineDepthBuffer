#include <string>
#include <Windows.h>
#include <shobjidl.h> 
#include "MainWindow.h"
#include "DebugPrint.h"

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    //case WM_CLOSE:
    //    if (MessageBox(m_hwnd, L"Really quit?", L"Quit", MB_OKCANCEL) == IDOK)
    //    {
    //        DestroyWindow(m_hwnd);
    //    }
    //    // Else: User canceled. Do nothing.
    //    return 0;

    case WM_DESTROY:
        DebugPrint(L"WM_DESTROY");
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        {
            DebugPrint(L"WM_SIZE");
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            LONG width = rc.right - rc.left;
            LONG height = rc.bottom - rc.top;
            m_buffer.Resize(width, height);
            // TODO(jaege): use m_objModel set m_buffer
            m_objModel.SetModelScale(width, height, 0.95);
            //m_buffer.DebugDarwRandomPicture();
            m_objModel.SetBuffer(m_buffer);
            m_buffer.DebugDrawBoundingRect(m_objModel.GetBoundingRect(), BLUE);
        }
        return 0;

    case WM_PAINT:
        {
            DebugPrint(L"WM_PAINT");
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);

            RECT rc;
            GetClientRect(m_hwnd, &rc);
            LONG width = rc.right - rc.left;
            LONG height = rc.bottom - rc.top;

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
            //     5 seconds after choose file in the dialog to continue.
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
                        // TODO(jaege): Load file to m_objModel.

                        m_objModel.LoadFromObjFile(pszFilePath);

                        const UINT32 MAX_CHARS = 1024;
                        WCHAR s_buffer[MAX_CHARS];
                        WCHAR windowTitle[MAX_CHARS];
                        GetWindowText(m_hwnd, windowTitle, GetWindowTextLength(m_hwnd) + 1);
                        swprintf(s_buffer, MAX_CHARS, L"%s - %s", windowTitle, pszFilePath);
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
