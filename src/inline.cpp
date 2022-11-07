#include "inline.h"
#include "View.h"



void inlineDirty(View *p, View &parent, std::vector<View_Ptr> &children, IGraphics& graphics)
{

    parent.calcHorizontal({}, p);

    int height = 0;
    if (children.size() > 0)
    {
        height = children.front()->style.margin.top.toPixels(parent.marginBox.width);
    }

    for (auto c : children)
    {
        c->marginBox.y = height + parent.contentBox.y;
        c->marginBox.x = parent.contentBox.x;
        c->dirty(&parent, graphics);
        if(!c->style.pos.x && !c->style.pos.y)
        height += c->style.margin.bottom.toPixels(parent.marginBox.width) + c->marginBox.height;
    }


    parent.calcVertical(height, p);
}


void blockDirty(View *p, View &parent, std::vector<View_Ptr> &children, IGraphics& graphics)
{
    int height = 0;
    if (children.size() > 0)
    {
        height = children.front()->style.margin.top.toPixels(parent.marginBox.width);
    }

    parent.calcHorizontal({}, p);

    int maxWidth = 0;
    for (auto c : children)
    {
        c->marginBox.y = height;
        c->marginBox.x = parent.contentBox.x;
        c->dirty(&parent, graphics);
        height += c->style.margin.bottom.toPixels(parent.marginBox.width) + c->marginBox.height;
        maxWidth = std::max(c->marginBox.width, maxWidth);
    }

    parent.calcHorizontal(maxWidth, p);
    parent.calcVertical(height, p);
}

