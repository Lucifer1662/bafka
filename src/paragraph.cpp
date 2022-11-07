#include "paragraph.h"


void Paragraph::draw(IGraphics &graphics)
{
    View::draw(graphics);


    if (style.font_colour)
    {
        if (style.whiteSpaceWrap == WhiteSpaceWrap::NoWrap)
        {
            auto textGraphic = std::unique_ptr<IText>(graphics.createText());
            textGraphic->setPosition({paddingBox.x, paddingBox.y});
            textGraphic->setFontSize(style.font_size);
            textGraphic->setFont(style.font);
            textGraphic->setText(text);
            textGraphic->setColour(*style.font_colour);
            textGraphic->draw();
        }
        else
        {
            int y = paddingBox.y;
            auto font = std::unique_ptr<IFont>(graphics.createFont(style.font));
            int line_height = font->line_height(style.font_size);
            for (auto &t : formatted_text)
            {
                auto textGraphic = std::unique_ptr<IText>(graphics.createText());
                textGraphic->setPosition({paddingBox.x, y});
                textGraphic->setFontSize(style.font_size);
                textGraphic->setFont(style.font);
                textGraphic->setText(t);
                textGraphic->setColour(*style.font_colour);
                textGraphic->draw();
                y += style.font_size;
            }
        }
    }
}



void Paragraph::dirty(View *parent, IGraphics &graphics)
{
    positions = {};
    positions.reserve(text.size()+1);
    if(style.whiteSpaceWrap == WhiteSpaceWrap::NoWrap){
        calcHorizontal({}, parent);
        int current_width = 0;
        auto font = std::unique_ptr<IFont>(graphics.createFont(style.font));
        positions.emplace_back(current_width + contentBox.x, contentBox.y);
        for (size_t i = 0; i < text.size(); i++)
        {
            char c_size = font->char_size(text[i], style.font_size);
            current_width += c_size;
            positions.emplace_back(current_width + contentBox.x, contentBox.y);
        }

        calcVertical(style.font_size, parent);

    }else if(style.whiteSpaceWrap == WhiteSpaceWrap::Wrap){
        auto font = std::unique_ptr<IFont>(graphics.createFont(style.font));
        calcHorizontal({}, parent);
        int width = contentBox.width;
        
        int new_height = 0;
        if(text.size() == 0){
            new_height = style.font_size;
        }

        int startIndex = 0;
        formatted_text = {};
        while(startIndex < text.size()){
            int current_width = 0;
            int last_break = -1;
            size_t i = startIndex;
            for (; i < text.size(); i++)
            {
                char c_size = font->char_size(text[i], style.font_size);
                if(current_width + c_size > width && last_break != -1){
                    formatted_text.push_back(text.substr(startIndex, last_break-startIndex));
                    break;
                }

                if(text[i] == '\n'){
                    formatted_text.push_back(text.substr(startIndex, i-startIndex));
                    last_break = i;
                    break;
                }

                if(std::isspace(text[i])){
                    last_break = i;
                }

                current_width += c_size;
            }

            new_height += font->line_height(style.font_size);

            if(i==text.size()){
                formatted_text.push_back(text.substr(startIndex, text.size()-startIndex));
                startIndex = i;
            }else{
                startIndex = last_break+1;
            }

        }

        calcVertical(new_height, parent);
    }


}



Vec2<int> Paragraph::characterPosition(int offset){
   return positions[offset];
}



int Paragraph::characterPositionFromPos(Vec2<int> pos){
    auto it = std::lower_bound(positions.begin(), positions.end(), pos, [](auto& a, auto& b){return a.x < b.x;});
    return std::max(0, std::distance(positions.begin(), it)-1);
}

