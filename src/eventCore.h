#pragma once



struct IEventCoreHandler {
    virtual void onClick(int x, int y){}
    virtual void onButtonPress(Key k){}
};

struct EventCoreHandler : public IEventCoreHandler {
    View_Ptr parent_view;
    View_Ptr in_focus;
    EventCoreHandler(View_Ptr parent_view):parent_view(parent_view){}
    virtual void onClick(int x, int y){
        parent_view->onClick({x,y});
        
        auto new_in_focus = parent_view->requestFocus({x,y});
        if(new_in_focus != in_focus){
            if(in_focus)
                in_focus->lostFocus();
            in_focus = new_in_focus;
        }
    }

    virtual void onButtonPress(Key k){
        if(in_focus){
            in_focus->OnKeyPress(k);
        }
    }

};

struct EventCore{
    IEventCoreHandler* handler = nullptr;

    virtual bool poll() = 0;

};