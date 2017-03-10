#pragma once

#include <vector>
#include <Windows.h>
#include "Color.h"

class OffscreenBuffer
{
    static constexpr INT32 BYTES_PER_PIXEL = 4;
public:
    void Resize(INT32 width, INT32 height);

    void SetPixel(INT32 x, INT32 y, const Color & color);
    void SetRow(INT32 y, const std::vector<Color> & row);

    void OnPaint(HDC hdc, INT32 width, INT32 height);

    void DebugDrawPoint(INT32 x, INT32 y, const Color & color);
    void DebugDrawRectangle(RECT rect, const Color & color);
    void DebugDarwRandomPicture();

    INT32 GetWidth() const { return m_width; }
    INT32 GetHeight() const { return m_height; }

private:
    // Memory Layout:
    //     From top to bottom, from left to right.
    //
    //     Position (x, y):
    //
    //      4 bytes   0       1       2 .. x .. width-1
    //      1  byte   0 1 2 3 4 5 6 7 8 ..   .. pitch-4 pitch-3 pitch-2 pitch-1
    //     ----------------------------------------------------------------
    //            0   b g r x b g r x b ..   .. b       g       r       x
    //            1   b g r x b g r x b ..   .. b       g       r       x
    //            2   b g r x b g r x b ..   .. b       g       r       x
    //            3   b g r x b g r x b ..   .. b       g       r       x
    //            .   .
    //            .   .
    //            y                        o
    //            .   .
    //            .   .
    //     height-1   b g r x b g r x b ..   .. b       g       r       x
    void * m_memory{nullptr};
    INT32 m_width{0};
    INT32 m_height{0};
    INT32 m_pitch{0};
    BITMAPINFO m_info{ };
};
