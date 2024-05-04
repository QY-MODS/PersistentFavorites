
#pragma once
#include "Manager.h"

class myEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                    public RE::BSTEventSink<RE::TESContainerChangedEvent>, 
                    public RE::BSTEventSink<RE::InputEvent*>,
                    public RE::BSTEventSink<RE::SpellsLearned::Event> {
    myEventSink() = default;
    myEventSink(const myEventSink&) = delete;
    myEventSink(myEventSink&&) = delete;
    myEventSink& operator=(const myEventSink&) = delete;
    myEventSink& operator=(myEventSink&&) = delete;


    Manager* M = Manager::GetSingleton();

    virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) override;
    virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                          RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;;

    virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;;

    virtual RE::BSEventNotifyControl ProcessEvent(const RE::SpellsLearned::Event* a_event,
                                     RE::BSTEventSource<RE::SpellsLearned::Event>*) override;;

    inline bool IsHotkeyEvent(const RE::BSFixedString& event_name) const;

public:
    using EventResult = RE::BSEventNotifyControl;
    static myEventSink* GetSingleton() {
        static myEventSink singleton;
        return &singleton;
    }


    void SaveCallback(SKSE::SerializationInterface* serializationInterface);

    void LoadCallback(SKSE::SerializationInterface* serializationInterface);

};