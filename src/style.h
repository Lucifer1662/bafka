#pragma once
#include "length.h"
#include "vec.h"
#include <optional>
#include <string>

enum class DisplayStyle
{
    Flex,
    Inline,
    Block,
};

enum class WhiteSpaceWrap
{
    Wrap,
    NoWrap
};




struct Style
{
    DisplayStyle display = DisplayStyle::Inline;
    Length l;
    Vec2<Length> pos;
    Vec2<Length> size;
    Vec4<Length> margin = Length(0);
    Vec4<Length> padding = Length(0);
    Vec4<Length> border = Length(0);
    std::optional<Colour> border_colour;
    std::optional<Colour> colour;
    std::optional<Colour> font_colour =  Colour(0,0,0,255);
    std::string font = "Roboto-Regular";
    WhiteSpaceWrap whiteSpaceWrap = WhiteSpaceWrap::Wrap;
    int font_size = 16;

    Style(){};
    Style(const Style &) = default;
    Style(Style &&) = default;
    Style &operator=(const Style &) = default;
};

