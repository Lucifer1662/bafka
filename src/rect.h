#pragma once
#include "vec.h"

struct Rect
{
    int x = 0, y = 0, width = 0, height = 0;

    bool contains(const Vec2<int> &pos)
    {
        return x <= pos.x && y <= pos.y && pos.x <= x + width && pos.y <= y + height;
    }

    Vec4<int> dif(const Rect r)
    {
        return {
            r.x - x,
            r.y - y,
            y + height - (r.y + r.height),
            x + width - (r.x + r.width),
        };
    }
};