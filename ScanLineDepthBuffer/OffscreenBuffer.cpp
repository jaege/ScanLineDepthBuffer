#include <cstdlib>  // std::abort
#include "OffscreenBuffer.h"

void OffscreenBuffer::Resize(INT32 width, INT32 height)
{
    if (m_memory)
    {
        VirtualFree(m_memory, 0, MEM_RELEASE);
        m_memory = NULL;
    }

    m_width = width;
    m_height = height;
    m_pitch = width * BYTES_PER_PIXEL;

    m_info.bmiHeader.biSize = sizeof(m_info.bmiHeader);
    m_info.bmiHeader.biWidth = m_width;
    m_info.bmiHeader.biHeight = -m_height;
    m_info.bmiHeader.biPlanes = 1;
    m_info.bmiHeader.biBitCount = 32;
    m_info.bmiHeader.biCompression = BI_RGB;

    SIZE_T bitmapMemorySize = m_width * m_height * BYTES_PER_PIXEL;
    m_memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    if (m_memory)
    {
        Render();
    }
    else if (width != 0 && height != 0)
    {
        OutputDebugString(L"VirtualAlloc Failed.\n");
        std::abort();
    }
}

void OffscreenBuffer::Render()
{
    // TODO(jaege): Below code is just for fun, I will implement scan-line
    //              z-buffer algorithm here in future.

    UINT8 *row = (UINT8 *)m_memory;
    for (int y = 0; y < m_height; ++y)
    {
        UINT32 *pixel = (UINT32 *)row;
        for (int x = 0; x < m_width; ++x)
        {
            *pixel++ = (((UINT8)(x)) << 16) |  // Red
                       (((UINT8)(y)) << 8) |  // Green
                       (UINT8)(x + y);  // Blue
        }
        row += m_pitch;
    }
}

void OffscreenBuffer::OnPaint(HDC hdc, LONG width, LONG height)
{ 
    StretchDIBits(hdc, 0, 0, width, height, 0, 0, m_width, m_height,
                  m_memory, &m_info, DIB_RGB_COLORS, SRCCOPY);
}
