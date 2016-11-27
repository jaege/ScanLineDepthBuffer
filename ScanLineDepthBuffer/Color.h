#pragma once

#include <Windows.h>

struct Color
{
    UINT8 red;
    UINT8 green;
    UINT8 blue;

    UINT32 GetColorCode() const
    {
        return (red << 16) | (green << 8) | blue;
    }

    static Color RandomColor();

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color BLACK;
    static const Color WHITE;
};