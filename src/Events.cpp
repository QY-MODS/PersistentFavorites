#include "Events.h"


RE::BSEventNotifyControl myEventSink::ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!*evns) return RE::BSEventNotifyControl::kContinue;

    for (RE::InputEvent* e = *evns; e; e = e->next) {
        if (e->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) continue;
        const RE::ButtonEvent* a_event = e->AsButtonEvent();
        if (!a_event->IsPressed()) continue;
        const RE::IDEvent* id_event = e->AsIDEvent();
        const auto& userEvent = id_event->userEvent;
        const auto userevents = RE::UserEvents::GetSingleton();
        if (userEvent != userevents->toggleFavorite && userEvent != userevents->yButton) continue;
        logger::trace("User event: {}", userEvent.c_str());
    }
    return RE::BSEventNotifyControl::kContinue;
};