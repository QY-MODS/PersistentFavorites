#pragma once

class myEventSink : public RE::BSTEventSink<RE::InputEvent*> {
    myEventSink() = default;
    myEventSink(const myEventSink&) = delete;
    myEventSink(myEventSink&&) = delete;
    myEventSink& operator=(const myEventSink&) = delete;
    myEventSink& operator=(myEventSink&&) = delete;

public:

    static myEventSink* GetSingleton() {
        static myEventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*);

};