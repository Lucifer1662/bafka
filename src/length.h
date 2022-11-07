#pragma once

struct Length
{
    union
    {
        float percent;
        int pixels;
    };

    enum class Type
    {
        Percent,
        Pixels,
        Undefined
    } type;

    // template<Type t>
    // Length (int pixels){

    // }

    Length()
    {
        type = Type::Undefined;
    }

    Length(int pixels) : pixels(pixels)
    {
        type = Type::Pixels;
    };
    Length(const Length &) = default;
    Length(Length &&) = default;
    Length &operator=(const Length &) = default;

    int toPixels(int parent_size)
    {
        if (type == Type::Percent)
        {
            return (int)(percent * pixels);
        }
        else
        {
            return pixels;
        }
    }

    int toPixels()
    {
        return pixels;
    }

    operator bool()
    {
        return type != Type::Undefined;
    }
};