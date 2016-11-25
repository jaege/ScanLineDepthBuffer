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

};

const Color RED{255, 0, 0};
const Color GREEN{0, 255, 0};
const Color BLUE{0, 0, 255};
const Color BLACK{0, 0, 0};
const Color WHITE{255, 255, 255};