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
            Resize(width, height);
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

            StretchDIBits(hdc,
                          0, 0, width, height,
                          0, 0, m_buffer.width, m_buffer.height,
                          m_buffer.memory,
                          &(m_buffer.info),
                          DIB_RGB_COLORS,
                          SRCCOPY);

            EndPaint(m_hwnd, &ps);
        }
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void MainWindow::Resize(INT32 width, INT32 height)
{
    if (m_buffer.memory)
    {
        VirtualFree(m_buffer.memory, 0, MEM_RELEASE);
        m_buffer.memory = NULL;
    }

    m_buffer.width = width;
    m_buffer.height = height;
    m_buffer.pitch = m_buffer.width * BYTES_PER_PIXEL;

    m_buffer.info.bmiHeader.biSize = sizeof(m_buffer.info.bmiHeader);
    m_buffer.info.bmiHeader.biWidth = m_buffer.width;
    m_buffer.info.bmiHeader.biHeight = -m_buffer.height;
    m_buffer.info.bmiHeader.biPlanes = 1;
    m_buffer.info.bmiHeader.biBitCount = 32;
    m_buffer.info.bmiHeader.biCompression = BI_RGB;

    SIZE_T bitmapMemorySize = m_buffer.width * m_buffer.height *
        BYTES_PER_PIXEL;
    m_buffer.memory = VirtualAlloc(0, bitmapMemorySize,
                                   MEM_COMMIT, PAGE_READWRITE);

    if (m_buffer.memory)
    {
        Render();
    }
    else if (width != 0 && height != 0)
    {
        OutputDebugString(L"VirtualAlloc Failed.\n");
        std::abort();
    }
}

void MainWindow::Render()
{
    // TODO(jaege): Below code is just for fun, I will implement scan-line
    //              z-buffer algorithm here in future.

    UINT8 *row = (UINT8 *)m_buffer.memory;
    for (int y = 0; y < m_buffer.height; ++y)
    {
        UINT32 *pixel = (UINT32 *)row;
        for (int x = 0; x < m_buffer.width; ++x)
        {
            *pixel++ = (((UINT8)(x)) << 16) |  // Red
                       (((UINT8)(y)) << 8) |  // Green
                       (UINT8)(x + y);  // Blue
        }
        row += m_buffer.pitch;
    }
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
