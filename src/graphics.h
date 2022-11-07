#pragma once
#include "vec.h"
#include <string>

struct IRectangle
{
    virtual void setPosition(const Vec2<int> &pos) = 0;
    virtual void setSize(const Vec2<int> &size) = 0;
    virtual void setColour(const Colour &colour) = 0;
    virtual void setOutlineColour(const Colour &colour) {}
    virtual void setOutlineThickness(const Vec4<int> &sizes) {}

    virtual void draw() = 0;

    virtual ~IRectangle() {}
};


struct IText {
    virtual void setPosition(const Vec2<int> &pos) = 0;
    virtual void setColour(const Colour &colour) = 0;
    virtual void setFontSize(int font_size) = 0;
    virtual void setFont(const std::string& font) = 0;
    virtual void setText(const std::string& text) = 0;
    virtual void draw() = 0;
};


struct IFont
{
    virtual int char_size(char c, int font_size)
    {
        return font_size;
    }

    virtual int line_height(int font_size)
    {
        return font_size;
    }
};


struct IGraphics
{
    virtual IRectangle *createRectangle() = 0;
    virtual IText *createText() = 0;
    virtual IFont* createFont(const std::string& font_name) = 0;
    virtual ~IGraphics() {}
};

