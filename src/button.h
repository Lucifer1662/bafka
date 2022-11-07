#pragma once

struct Button : public View
{
    std::function<void()> onClickHandler = []() {};
    Context_Ptr clickContext;
    bool initialized = false;
    std::shared_ptr<Paragraph> text;


    Button()
    {
        text = make<Paragraph>();
        style.display = DisplayStyle::Block;
        Add(text);
    };

    Button(const Button &) = default;
    Button(Button &&) = default;
    Button &operator=(const Button &) = default;

    void RegisterOnClick(std::function<void()> handler)
    {
        onClickHandler = handler;
    }

    void onClick(const Vec2<int> &pos) override
    {
        onClickHandler();
    }



    Paragraph& getText(){
        return *text;
    }
};
