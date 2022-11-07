#pragma once
#include <string>
#include <list>
#include "View.h"
#include "graphics.h"


struct IGraphics;

struct Paragraph : public View
{
    std::string text;
    std::list<std::string> formatted_text;
    std::vector<Vec2<int>> positions;
    void draw(IGraphics &graphics) override;
    void dirty(View *parent, IGraphics &graphics) override;

    Vec2<int> characterPosition(int offset);
    int characterPositionFromPos(Vec2<int> pos);
};
