#pragma once


template<typename T>
T default_value(){
    return T();
}


template<>
int default_value<int>();




template <typename T>
struct Vec2
{
    T x = default_value<T>();
    T y = default_value<T>();
    Vec2(T x, T y) : x(x), y(y) {}
    Vec2(T x) : x(x), y(x) {}
    Vec2() = default;
    Vec2(const Vec2<T> &) = default;
    Vec2(Vec2<T> &&) = default;
    Vec2<T> &operator=(const Vec2<T> &) = default;

    Vec2<T> &operator=(const T& t){
        x=y=t;
        return *this;
    }
};

template <typename T = int>
struct Vec4
{
    union
    {
        T x = default_value<T>();
        T r;
        T left;
    };
    union
    {
        T y = default_value<T>();
        T g;
        T top;
    };
    union
    {
        T z = default_value<T>();
        T b;
        T bottom;
    };
    union
    {
        T w = default_value<T>();
        T a;
        T right;
    };

    Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    Vec4(T x) : x(x), y(x), z(x), w(x) {}
    Vec4(){};
    Vec4(const Vec4<T> &) = default;
    Vec4(Vec4<T> &&) = default;
    Vec4<T> &operator=(const Vec4<T> &) = default;

    Vec4<T> &operator=(const T& t){
        x=y=z=w=t;
        return *this;
    }
};

using Colour = Vec4<int>;
