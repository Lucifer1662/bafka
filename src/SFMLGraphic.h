#pragma once
#include "graphics.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "eventCore.h"

struct SFMLRectangle : public IRectangle
{
    Vec2<int> pos;
    Vec2<int> size;
    sf::RectangleShape shape;
    sf::RenderWindow &window;

    SFMLRectangle(sf::RenderWindow &window) : window(window) {}

    void setPosition(const Vec2<int> &pos) override
    {
        shape.setPosition({(float)pos.x, (float)pos.y});
    }

    void setSize(const Vec2<int> &size)
    {
        shape.setSize({(float)size.x, (float)size.y});
    }

    void setColour(const Colour &colour) override
    {
        auto c = sf::Color(colour.r, colour.g, colour.b, colour.a);
        shape.setFillColor(c);
    }

    void setOutlineColour(const Colour &colour) override
    {
        auto c = sf::Color(colour.r, colour.g, colour.b, colour.a);
        shape.setOutlineColor(c);
    }

    void setOutlineThickness(const Vec4<int> &sizes) override
    {
        shape.setOutlineThickness((float)sizes.x);
        auto s = shape.getSize();
        s.x -=sizes.left-sizes.right;
        s.y -=sizes.top-sizes.bottom;
        shape.setSize(s);
    }

    void draw() override
    {
        window.draw(shape);
    }
};

struct FontLibrary {
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> fonts;
    std::shared_ptr<sf::Font> get_font(const std::string& font_name){
        auto it = fonts.find(font_name);

        if(it != fonts.end()){
            return  it->second;
        }

        auto font = std::make_shared<sf::Font>();
        if (!font->loadFromFile(font_name+".ttf"))
        {
            return {};
        }

        fonts.insert({font_name, font});

        return font;
    }
};

struct SFMLText : public IText {
    sf::Text text;
    sf::RenderWindow &window;
    FontLibrary& fontLibrary;

    SFMLText( sf::RenderWindow &window, FontLibrary& fontLibrary): window(window), fontLibrary(fontLibrary){}

    void setPosition(const Vec2<int> &pos) override {  
        text.setPosition({(float)pos.x, (float)pos.y});
    }

    void setColour(const Colour &colour) override
    {
        auto c = sf::Color(colour.r, colour.g, colour.b, colour.a);
        text.setFillColor(c);
    }

    void setFontSize(int font_size) override {
        text.setCharacterSize(font_size);
    
    }
    void setFont(const std::string& font) override {
        auto f = fontLibrary.get_font(font);
        text.setFont(*f);
    }

    void setText(const std::string& text_str) override {
        text.setString(text_str);
    }

    void draw() override
    {
        window.draw(text);
    }
};


struct SFMLFont : public IFont {
    std::shared_ptr<sf::Font> font;

    SFMLFont(std::shared_ptr<sf::Font> font):font(font){}

    int char_size(char c, int font_size) override {
        auto glyph = font->getGlyph(c, font_size,false);
        return (int)glyph.bounds.width;
    }

    int line_height(int font_size) override
    {
        return (int)font->getLineSpacing(font_size);
    }

};

struct SFMLEventCore : public EventCore{
    sf::Event event;
    sf::RenderWindow& window;
    sf::Clock keyClock; 
    std::optional<Key> lastKey;
    bool pressed = false;

    SFMLEventCore(sf::RenderWindow& window):window(window){}

    bool poll() override {
        // while there are pending events...
        window.pollEvent(event);
        // check the type of the event...
        switch (event.type)
        {
            // window closed
            case sf::Event::Closed:
                window.close();
                return false;

            case sf::Event::MouseButtonPressed:
            {

                if(event.mouseButton.button == sf::Mouse::Button::Left){
                    if(handler)handler->onClick(event.mouseButton.x, event.mouseButton.y);
                }
            }
            break;

            case sf::Event::KeyPressed:
            {
                if(event.key.code == sf::Keyboard::Key::Left)
                    handleKey(Key(0, Key::Other::Left));
                else if(event.key.code == sf::Keyboard::Key::Right)
                    handleKey(Key(0, Key::Other::Right));
                    
            }
            break;

            case sf::Event::KeyReleased:
            {
                lastKey = {};
            }
            break;

            case sf::Event::TextEntered:
            {
                Key k = event.text.unicode;
                handleKey(k);
                break;
            }

        }
        return true;
    }

    void handleKey(Key& k){
         if(!lastKey || lastKey->c != k.c){
            keyClock.restart();
            lastKey = k;
            if(handler)handler->onButtonPress(k);
        }else{
            auto elapsed = keyClock.getElapsedTime();

            if(elapsed.asMilliseconds() > 300){
                keyClock.restart();
                if(handler)handler->onButtonPress(k);
            }
        }
    }
};

struct SFMLGraphic : public IGraphics
{
    sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(200, 200), "SFML works!");
    FontLibrary fontLibrary;
    IRectangle *createRectangle() override
    {
        return new SFMLRectangle(window);
    };

    IText *createText() override {
        return new SFMLText(window, fontLibrary);
    }

    IFont* createFont(const std::string& font_name) override {
        return new SFMLFont(fontLibrary.get_font(font_name));
    }

    void clear()
    {
        window.clear();
    }

    void display()
    {
        window.display();
    }

    SFMLEventCore createEventCore(){
        return SFMLEventCore(window);
    }
};





