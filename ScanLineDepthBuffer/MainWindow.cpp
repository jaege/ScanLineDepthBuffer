#include <cstdlib>
#include <Windows.h>
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

void MainWindow::Resize(UINT32 width, UINT32 height)
{
    // TODO(jaege): check which go first, free old memory or allocate new.
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
    //m_buffer.info.bmiHeader.biSizeImage = 0;
    //m_buffer.info.bmiHeader.biXPelsPerMeter = 0;
    //m_buffer.info.bmiHeader.biYPelsPerMeter = 0;
    //m_buffer.info.bmiHeader.biClrUsed = 0;
    //m_buffer.info.bmiHeader.biClrImportant = 0;

    int bitmapMemorySize =
        m_buffer.width * m_buffer.height * BYTES_PER_PIXEL;
    m_buffer.memory =
        VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    if (!m_buffer.memory)
    {
        OutputDebugString(L"VirtualAlloc Failed.");
        std::abort();
    }

    Render();
}

void MainWindow::Render()
{
    UINT8 *Row = (UINT8 *)m_buffer.memory;
    for (int y = 0; y < m_buffer.height; ++y)
    {
        UINT32 *Pixel = (UINT32 *)Row;
        for (int x = 0; x < m_buffer.width; ++x)
        {
            *Pixel++ = (((UINT8)(x)) << 16) |  // Red
                (((UINT8)(y)) << 8) |  // Green
                0;                                // Blue
        }
        Row += m_buffer.pitch;
    }
}