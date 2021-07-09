#pragma once


class MathHelper
{
public:
    static const float Infinity;
    static const float Pi;

    template<typename T>
    static T Clamp(const T& x, const T& low, const T& high)
    {
        return x < low ? low : (x > high ? high : x);
    }
};

