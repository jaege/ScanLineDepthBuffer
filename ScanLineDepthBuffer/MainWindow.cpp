#include <string>
#include <Windows.h>
#include <shobjidl.h> 
#include "MainWindow.h"

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
        OutputDebugString(L"WM_DESTROY\n");
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        {
            OutputDebugString(L"WM_SIZE\n");
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            LONG width = rc.right - rc.left;
            LONG height = rc.bottom - rc.top;
            m_buffer.Resize(width, height);
        }
        return 0;

    case WM_PAINT:
        {
            OutputDebugString(L"WM_PAINT\n");
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
                        OutputDebugString(pszFilePath);
                        OutputDebugString(L"\n");
                        // TODO(jaege): Load file to m_objModel.

                        m_objModel.LoadFromObjFile(pszFilePath);


                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
}
