#include <random>
#include "Color.h"

const Color Color::RED{255, 0, 0};
const Color Color::GREEN{0, 255, 0};
const Color Color::BLUE{0, 0, 255};
const Color Color::BLACK{0, 0, 0};
const Color Color::WHITE{255, 255, 255};

Color Color::RandomColor()
{
    static std::mt19937 eng{std::random_device()()};
    static std::uniform_int_distribution<unsigned int> u(0, 255);
    return{static_cast<UINT8>(u(eng)),
           static_cast<UINT8>(u(eng)),
           static_cast<UINT8>(u(eng))};
}