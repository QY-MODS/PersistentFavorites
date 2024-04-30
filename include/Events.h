#pragma once

#include "Manager.h"

class myEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                    public RE::BSTEventSink<RE::TESContainerChangedEvent>, 
                    public RE::BSTEventSink<RE::InputEvent*> {
    myEventSink() = default;
    myEventSink(const myEventSink&) = delete;
    myEventSink(myEventSink&&) = delete;
    myEventSink& operator=(const myEventSink&) = delete;
    myEventSink& operator=(myEventSink&&) = delete;

    Manager* M = Manager::GetSingleton();

public:

    static myEventSink* GetSingleton() {
        static myEventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*);

    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                          RE::BSTEventSource<RE::TESContainerChangedEvent>*);

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*);

    void SaveCallback(SKSE::SerializationInterface* serializationInterface);

    void LoadCallback(SKSE::SerializationInterface* serializationInterface);

};