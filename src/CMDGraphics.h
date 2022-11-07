#pragma once
#include "graphics.h"
#include "vec.h"

struct CMDRectangle : public IRectangle
{
    Vec2<int> pos;
    Vec2<int> size;
    void setPosition(const Vec2<int> &pos) override
    {
        this->pos = pos;
    }

    void setSize(const Vec2<int> &size)
    {
        this->size = size;
    }

    void setColour(const Colour &colour) override
    {
    }

    void draw()
    {
        std::cout << pos.x << ", " << pos.y << ", " << size.x << ", " << size.y << std::endl;
    }
};

struct CMDGraphic : public IGraphics
{
    IRectangle *createRectangle() override
    {
        return new CMDRectangle();
    };

    IText *createText() override {
        return nullptr;
    }

    IFont* createFont(const std::string& font_name) override {
        return {};
    }

};
