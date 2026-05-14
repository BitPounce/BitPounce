#pragma once

namespace BitPounce
{
    template<typename T>
    struct Rect
    {
        T x = 0;
        T y = 0;
        T w = 0;
        T h = 0;

        Rect(T x = 0, T y = 0, T w = 0, T h = 0)
        {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
        }
    };

    template<typename T>
    bool RectCheckCollision(const Rect<T>& a, const Rect<T>& b)
    {
        return (
            a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y
        );
    }
}