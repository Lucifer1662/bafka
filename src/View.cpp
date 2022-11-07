#include "View.h"
#include "inline.h"



void View::dirty(View *parent, IGraphics& graphics)
{
    if(style.pos.x){
        marginBox.x = style.pos.x.toPixels(parent->contentBox.x);
    }

    if(style.pos.y){
        marginBox.y = style.pos.y.toPixels(parent->contentBox.y);
    }

    if(style.display == DisplayStyle::Inline){
        inlineDirty(parent, *this, children, graphics);
    }else if(style.display == DisplayStyle::Block){
        blockDirty(parent, *this, children, graphics);
    }
}



void View::draw(IGraphics &graphics)
{
    if (style.colour || (style.border_colour && style.border.x))
    {
        rectangleGraphic = std::unique_ptr<IRectangle>(graphics.createRectangle());
        rectangleGraphic->setPosition({paddingBox.x, paddingBox.y});
        rectangleGraphic->setSize({paddingBox.width, paddingBox.height});
        if (style.colour){
            rectangleGraphic->setColour(*style.colour);
        }else{
            rectangleGraphic->setColour(Colour(0,0,0,0));
        }

        if(style.border_colour && style.border.x){
            rectangleGraphic->setOutlineColour(*style.border_colour);
            rectangleGraphic->setOutlineThickness(borderBox.dif(paddingBox));
        }
    }

    if (rectangleGraphic)
    {
        rectangleGraphic->draw();
    }

    for (auto c : children)
    {
        c->draw(graphics);
    }
}



void View::calcHorizontal(std::optional<int> override_width, View* p){
        
    int margin_width = 0;
    int border_width = 0;
    int padding_width = 0;
    int margin_width_left = 0;
    int border_width_left = 0;
    int padding_width_left = 0;

    if (p != nullptr)
    {
        int border_left = style.border.left.toPixels(p->marginBox.width);
        int border_right = style.border.right.toPixels(p->marginBox.width);

        int margin_left = style.margin.left.toPixels(p->marginBox.width);
        int margin_right = style.margin.right.toPixels(p->marginBox.width);

        int padding_left = style.padding.left.toPixels(p->marginBox.width);
        int padding_right = style.padding.right.toPixels(p->marginBox.width);

        border_width_left = margin_left;
        padding_width_left = border_width_left + border_left;
        margin_width_left = padding_width_left + padding_left;

        margin_width = margin_left + border_left + padding_left + padding_right + border_right + margin_right;
        border_width = border_left + padding_left + padding_right + border_right;
        padding_width = padding_left + padding_right;
    }

    int width = 0;
    int x = marginBox.x;

    if(override_width){
        width = *override_width;
    }else{
        if (style.size.x)
        {
            width = style.size.x.toPixels();
        }
        else
        {
            if (p != nullptr)
            {
                width = p->marginBox.width - margin_width;
            }
        }
    }

    marginBox.width = width + margin_width;
    contentBox.width = width;
    borderBox.width = width + border_width;
    paddingBox.width = width + padding_width;

    marginBox.x = x;
    borderBox.x = x + border_width_left;
    paddingBox.x = x + padding_width_left;
    contentBox.x = x + margin_width_left;
}



void View::calcVertical(int height, View* p){
    int additional_height = 0;
    int margin_height = 0;
    int border_height = 0;
    int padding_height = 0;
    int margin_height_top = 0;
    int border_height_top = 0;
    int padding_height_top = 0;
    int y = marginBox.y;

    if (p != nullptr)
    {
        
        int margin_top = style.margin.top.toPixels(p->marginBox.width);
        int margin_bottom = style.margin.bottom.toPixels(p->marginBox.width);
        
        int border_top = style.border.top.toPixels(p->marginBox.width);
        int border_bottom = style.border.bottom.toPixels(p->marginBox.width);

        int padding_top = style.padding.top.toPixels(p->marginBox.width);
        int padding_bottom = style.padding.bottom.toPixels(p->marginBox.width);

        border_height_top = margin_top;
        padding_height_top = border_height_top + border_top;
        margin_height_top = padding_height_top + padding_top;

        margin_height = margin_top + border_top + padding_top + padding_bottom + border_bottom + margin_bottom;
        border_height = border_top + padding_top + padding_bottom + border_bottom;
        padding_height = padding_top + padding_bottom;
    }

    if (style.size.y)
    {
        height = style.size.y.toPixels();
    }

    marginBox.height = height + margin_height;
    contentBox.height = height;
    borderBox.height = height + border_height;
    paddingBox.height = height + padding_height;

    marginBox.y = y;
    contentBox.y = y + margin_height_top;
    borderBox.y = y + border_height_top;
    paddingBox.y = y + padding_height_top;
}

