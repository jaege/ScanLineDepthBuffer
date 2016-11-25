#include <random>
#include "Color.h"

Color Color::RandomColor()
{
    std::mt19937 eng;
    eng.seed(std::random_device()());
    std::uniform_int_distribution<unsigned int> u(0, 255);
    return Color{ static_cast<UINT8>(u(eng)),
                  static_cast<UINT8>(u(eng)),
                  static_cast<UINT8>(u(eng)) };
}
