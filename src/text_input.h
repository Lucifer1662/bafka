#pragma once
#include "View.h"
#include "paragraph.h"


struct IGraphics;

struct TextInput : public View
{
    std::shared_ptr<Paragraph> text;
    std::shared_ptr<View> cursor;
    int cursor_position = 0;

    void dirty(View *parent, IGraphics &graphics) override {
        View::dirty(parent, graphics);

        if(cursor){
            auto pos = text->characterPosition(cursor_position);
            cursor->style.pos = Vec2<Length>(pos.x, pos.y); 
        }
    }

    TextInput()
    {
        text = make<Paragraph>();
        text->style.whiteSpaceWrap = WhiteSpaceWrap::NoWrap;
        Add(text);
    };  


    void onClick(const Vec2<int>& pos) override {
        cursor_position = text->characterPositionFromPos(pos);
        if(!cursor){
            cursor = make<View>();
            cursor->style.colour = Colour(0,0,0,255);
            cursor->style.size.y = Length(text->style.font_size);
            cursor->style.size.x = Length(3);
            Add(cursor);
        }
        auto p = text->characterPosition(cursor_position);
        cursor->style.pos = Vec2<Length>(p.x, p.y); 
        
    }

    View_Ptr requestFocus(const Vec2<int>& pos) override
    {
        return self.lock();
    }

    void lostFocus() override {
        Remove(cursor);
        cursor={};
    }


    void OnKeyPress(const Key &key) override {
        if(key.other){
            if(*key.other == Key::Other::Left){
                cursor_position = std::max(cursor_position-1, 0);
            }else if(*key.other == Key::Other::Right){
                cursor_position = std::min(cursor_position+1, (int)text->text.size());
            }

        }else if(key.isbackspace()){
            if(cursor_position > 0){
                text->text.erase(cursor_position-1, 1);
                cursor_position--;
            }
        }else{
            text->text.insert(cursor_position, 1, key.c);
            cursor_position++;
        }


    }



};
