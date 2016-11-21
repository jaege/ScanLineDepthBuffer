#pragma once

#include <Windows.h>

struct Color
{
    UINT8 red;
    UINT8 green;
    UINT8 blue;

    UINT32 GetColor() const
    {
        return (red << 16) | (green << 8) | blue;
    }

    static Color RandomColor();
};