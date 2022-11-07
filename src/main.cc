
#include <memory>
#include <iostream>
#include <unordered_set>
#include <functional>
#include <list>
#include <queue>
#include <string>
#include <optional>
#include "context.h"
#include "div.h"
#include "paragraph.h"
#include "button.h"
#include "text_input.h"
#include "SFMLGraphic.h"

struct TaxCalculator
{
    State<int> income = State<int>(10);

    int calc() const
    {
        auto res = income.get() * 5;
        return res;
    }
};




struct View;



View_Ptr App()
{
    auto status = State<std::string>("good");

    auto div = make<Div>();
    div->style.size = Vec2<Length>(150, 150);
    div->style.colour = Colour(255, 0, 0, 255);

    auto textBox1 = make<TextInput>();
    textBox1->style.colour = Colour(0, 255, 0, 255);
    textBox1->text->text = "Way";
    textBox1->style.margin = Length(3);

    auto textBox2 = make<TextInput>();
    textBox2->style.colour = Colour(255, 255, 30, 255);
    textBox2->text->text = "Help";
    textBox2->style.margin = Length(3);
    textBox2->style.padding = Length(2);


    auto button = make<Button>();
    button->style.margin.bottom = Length(5);
    button->style.padding.left = Length(5);
    // button->style.size = Vec2<Length>(50,50);
    button->style.colour = Colour(0, 255, 0, 255);

    auto button2 = make<Button>();
    button2->style.margin.bottom = Length(30);
    // button2->style.margin.left = Length(30);
    // button2->style.padding.top = Length(5);
    button2->style.padding = Length(5);
    button2->style.margin = Length(30);

    // button2->style.padding.left = Length(5);
    // button->style.size = Vec2<Length>(50,50);
    button2->style.colour = Colour(0, 255, 255, 255);
    button2->style.border_colour = Colour(255, 0, 255, 255);
    button2->style.border = Length(2);
    button2->getText().text = "click me";

    // button->RegisterOnClick([=]()
    //                      { status.set("bad"); });

    auto p = make<Paragraph>();
    p->style.font_colour = Colour(0,0,255,255);
    p->style.colour = Colour(255,0,255,255);
    p->text = "Hello gggWorld";

    auto p1 = make<Paragraph>();
    p1->style.font_colour = Colour(0,0,255,255);
    p1->style.colour = Colour(255,0,255,255);
    p1->text = "Hello World World World World World";

    div->Add(textBox1);
    div->Add(textBox2);
    // div->Add(textBox2);
    div->Add(button);
    // div->Add(button2);
    // div->Add(p);
    // div->Add(p1);

    return div;
};

#include "junk.h"


int main(int argn, char **argv)
{
    Foo f;
    f.foo();
    std::cout << "Update" << std::endl;
    f.foo();
    return 0;

    SFMLGraphic window;
    auto eventCore = window.createEventCore();

    View_Ptr app = App();

    EventCoreHandler handler(app);
    eventCore.handler = &handler;

    while (eventCore.poll())
    {
        app->dirty(nullptr, window);
        window.clear();
        app->draw(window);
        window.display();
    }

    return 0;

    // auto c3 = context([&]()
    //                   {
    //     app = App();

    //     app->draw(window);
    // });

    return 0;

    // auto c4 = context([=]()
    //                   { click.set(Vec2<int>(5, 5)); });

    // auto [get, set] = event_emmiter1<int>(0);

    // TaxCalculator tax;

    // TaxCalculator t1 = tax;

    // auto c = context([=]()
    //                  {
    //     std::cout << get() + 1 << std::endl;
    //     std::cout << "Tax After:" << tax.calc() << std::endl; });

    // // input from command line -> state -> calculation -> output

    // bool run = true;
    // while (run)
    // {
    //     auto c2 = context([&]()
    //                       {

    //         int x;
    //         std::cin >> x;

    //         std::cout << "Tax Before:" << tax.calc() << std::endl;

    //         if(x==0)
    //             run=false;

    //         set(x);
    //         tax.income.set(x); });

    //     std::cout << "Loop" << std::endl;
    // }

    return 0;
}