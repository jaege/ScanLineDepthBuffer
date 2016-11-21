#pragma once

#include <Windows.h>

#define BYTES_PER_PIXEL 4

class OffscreenBuffer
{
public:
    void Resize(INT32 width, INT32 height);
    void Render();
    void OnPaint(HDC hdc, LONG width, LONG height);

private:
    BITMAPINFO m_info;
    LPVOID m_memory;
    INT32 m_width;
    INT32 m_height;
    INT32 m_pitch;

};
