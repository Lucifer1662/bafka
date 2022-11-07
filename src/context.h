#pragma once
#include <memory>
#include <iostream>
#include <unordered_set>
#include <functional>
#include <list>
#include <queue>
#include <string>


class Context;

thread_local std::queue<Context *> current_context;

class Context
{
    std::function<void()> handler;
    std::unordered_set<void *> dependents;
    std::unordered_set<void *> invokers;
    std::unordered_set<Context *> dirty;
    bool being_invoked = false;

public:
    template <typename Func>
    Context(Func &&handler) : handler(std::forward<Func>(handler)) {}

    void invoke()
    {
        if (!being_invoked)
        {
            current_context.push(this);
            being_invoked = true;
            dependents = {};
            invokers = {};
            handler();
            being_invoked = false;
            current_context.pop();

            update_dirty();
        }
    }

    void called(void *id)
    {
        dependents.insert(id);
    }

    void updated(void *id)
    {
        invokers.insert(id);
    }

    void dirtied(const std::unordered_set<Context *> cs)
    {
        dirty.insert(cs.begin(), cs.end());
    }

    void update_dirty()
    {
        bool self_dirtied = false;
        for (auto &d : dirty)
        {
            if (d != this)
            {
                d->invoke();
            }
            else
            {
                self_dirtied = true;
            }
        }
        dirty = {};

        // if(self_dirtied){
        //     invoke();
        // }
    }
};

using Context_Ptr = std::unique_ptr<Context>;

template <typename Func>
Context_Ptr context(Func &&func)
{
    auto c = std::make_unique<Context>(func);
    c->invoke();
    return c;
}

template <typename Event>
struct Emitter
{
    Event event;
    std::unordered_set<Context *> used_in_contexts;
    Emitter(Event &&event) : event(std::forward<Event>(event)) {}

    void operator()(Event &&new_event)
    {
        if (!current_context.empty())
        {
            current_context.back()->updated(this);
            current_context.back()->dirtied(used_in_contexts);
            used_in_contexts = {};
        }
        event = std::forward<Event>(new_event);
        // for(auto used_in : used_in_contexts){
        //     used_in->invoke();
        // }
    }

    const Event &operator()()
    {
        if (!current_context.empty())
        {
            current_context.back()->called(this);
            used_in_contexts.insert(current_context.back());
        }
        return event;
    }
};

template <typename Event>
auto event_emmiter1(Event &&default_event)
{
    auto emitter = std::make_shared<Emitter<Event>>(std::forward<Event>(default_event));
    return std::make_tuple(
        [=]()
        {
            return (*emitter)();
        },
        [=](auto new_e)
        {
            (*emitter)(std::forward<Event>(new_e));
        });
}

template <typename Event>
struct State
{
    std::shared_ptr<Emitter<Event>> emitter;

    State(Event &&default_event) : emitter(std::make_shared<Emitter<Event>>(std::forward<Event>(default_event))) {}
    State(const State &s) : emitter(s.emitter) {}

    const Event &get() const
    {
        return (*emitter)();
    }

    template <typename E>
    void set(E &&e) const
    {
        (*emitter)(std::forward<Event>(e));
    }

    auto to_tup()
    {
        return std::make_tuple(
            [=]()
            {
                return (*emitter)();
            },
            [=](auto new_e)
            {
                (*emitter)(std::forward<Event>(new_e));
            });
    }
};

template <typename Event>
auto event_emmiter(Event &&default_event)
{
    auto e = std::make_shared<Event>(std::forward<Event>(default_event));
    return std::make_tuple(
        [=]()
        {
            if (!current_context.empty())
            {
                current_context.back()->called(&*e);
            }
            return *e;
        },
        [=](Event &&new_e)
        {
            *e = std::forward<Event>(new_e);
        });
}