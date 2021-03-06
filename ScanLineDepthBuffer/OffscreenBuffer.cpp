﻿#include <cstdlib>  // std::abort
#include <cassert>
#include "OffscreenBuffer.h"
#include "DebugPrint.h"

void OffscreenBuffer::Resize(INT32 width, INT32 height)
{
    if (m_memory)
    {
        VirtualFree(m_memory, 0, MEM_RELEASE);
        m_memory = nullptr;
    }

    m_width = width;
    m_height = height;
    m_pitch = width * BYTES_PER_PIXEL;

    m_info.bmiHeader.biSize = sizeof(m_info.bmiHeader);
    m_info.bmiHeader.biWidth = m_width;
    m_info.bmiHeader.biHeight = -m_height;  // from top to bottom
    m_info.bmiHeader.biPlanes = 1;
    m_info.bmiHeader.biBitCount = 32;
    m_info.bmiHeader.biCompression = BI_RGB;

    SIZE_T bitmapMemorySize = m_width * m_height * BYTES_PER_PIXEL;
    m_memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    if (!m_memory && width != 0 && height != 0)
    {
        DebugPrint(L"[WRN] VirtualAlloc Failed.");
        std::abort();
    }
}

void OffscreenBuffer::SetPixel(INT32 x, INT32 y, const Color & color)
{
    assert(x >= 0 && x < m_width && y >= 0 && y < m_height);
    UINT32 *pixel = reinterpret_cast<UINT32 *>(
        static_cast<UINT8 *>(m_memory) + x * BYTES_PER_PIXEL + y * m_pitch);
    //UINT32 *pixel2 = static_cast<UINT32 *>(m_memory) + x + y * m_width;
    *pixel = color.GetColorCode();
}

void OffscreenBuffer::SetRow(INT32 y, const std::vector<Color> & row)
{
    assert(y >= 0 && y < m_height && row.size() == m_width);
    UINT32 *pixel = reinterpret_cast<UINT32 *>(
        static_cast<UINT8 *>(m_memory) + y * m_pitch);
    for (const Color &c : row)
        *pixel++ = c.GetColorCode();
}

void OffscreenBuffer::OnPaint(HDC hdc, INT32 width, INT32 height)
{
    StretchDIBits(hdc, 0, 0, width, height, 0, 0, m_width, m_height,
                  m_memory, &m_info, DIB_RGB_COLORS, SRCCOPY);
}

void OffscreenBuffer::DebugDrawPoint(INT32 x, INT32 y, const Color & color)
{
#ifndef NDEBUG
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
        SetPixel(x, y, color);
#endif  // NDEBUG
}
void OffscreenBuffer::DebugDrawRectangle(RECT rect, const Color & color)
{
#ifndef NDEBUG
    // Ignore rectangle that out of screen.
    if (rect.left >= 0 && rect.left < m_width)
        for (INT32 y = rect.top; y < rect.bottom; ++y)
            if (y >= 0 && y < m_height)
                SetPixel(rect.left, y, color);
    if (rect.right >= 0 && rect.right < m_width)
        for (INT32 y = rect.top; y < rect.bottom; ++y)
            if (y >= 0 && y < m_height)
                SetPixel(rect.right, y, color);
    if (rect.top >= 0 && rect.top < m_height)
        for (INT32 x = rect.left; x < rect.right; ++x)
            if (x >= 0 && x < m_width)
                SetPixel(x, rect.top, color);
    if (rect.bottom >= 0 && rect.bottom < m_height)
        for (INT32 x = rect.left; x < rect.right; ++x)
            if (x >= 0 && x < m_width)
                SetPixel(x, rect.bottom, color);
#endif  // NDEBUG
}

void OffscreenBuffer::DebugDarwRandomPicture()
{
    // NOTE(jaege): Below code is just for fun.
    UINT8 *row = static_cast<UINT8 *>(m_memory);
    for (int y = 0; y < m_height; ++y)
    {
        UINT32 *pixel = reinterpret_cast<UINT32 *>(row);
        for (int x = 0; x < m_width; ++x)
        {
            *pixel++ = Color::RandomColor().GetColorCode();
        }
        row += m_pitch;
    }
}