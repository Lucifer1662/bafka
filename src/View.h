#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "rect.h"
#include "style.h"
#include "graphics.h"
#include "inline.h"
#include "vec.h"


struct View;

using View_Ptr = std::shared_ptr<View>;
using newV = std::shared_ptr<View>;


struct Key
{
    char c;

    enum Other {
        Left,
        Right,
        Down,
        Up
    };

    std::optional<Other> other;

    Key(char c, std::optional<Other>other={}):c(c),other(other){}


    bool isbackspace() const {
        return c == 8;
    }

};

struct View
{
    std::vector<View_Ptr> children;
    std::weak_ptr<View> self;

    Rect marginBox;
    Rect contentBox;
    Rect borderBox;
    Rect paddingBox;
    Style style;
    std::unique_ptr<IRectangle> rectangleGraphic;

    View() = default;
    View(const View &v) = delete;
    View(View &&v) : children(children)
    {
        v.children = {};
    }

    virtual void draw(IGraphics &graphics);

    virtual void dirty(View *parent, IGraphics &graphics);

    std::optional<View_Ptr> ViewIn(const Vec2<int> &pos)
    {
        for (auto &child : children)
        {
            if (child->marginBox.contains(pos))
            {
                return child;
            }
        }
        return {};
    }

    virtual void onClick(const Vec2<int> &pos)
    {
        auto child = ViewIn(pos);
        if (child)
        {
            (*child)->onClick(pos);
        }
    }

    virtual View_Ptr requestFocus(const Vec2<int>& pos)
    {
        auto child = ViewIn(pos);
        if (child)
        {
            return (*child)->requestFocus(pos);
        }
        return {};
    }

    virtual void lostFocus()
    {

    }

    virtual void OnKeyPress(const Key &key) {}

    void Add(View_Ptr view)
    {
        children.push_back(view);
    }

    void Remove(View_Ptr view){
        auto it = std::find(children.begin(), children.end(), view);
        if(it != children.end()){
            children.erase(it);
        }
    }

    void calcHorizontal(std::optional<int> override_width, View* p);

    void calcVertical(int height, View* p);

    virtual ~View() {}
};

template <typename V>
inline std::shared_ptr<V> make()
{
    auto v = std::make_shared<V>();
    v->self = v;
    return v;
}


