#include "Events.h"
#include "Settings.h"


RE::BSEventNotifyControl myEventSink::ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!*evns) return RE::BSEventNotifyControl::kContinue;
    for (RE::InputEvent* e = *evns; e; e = e->next) {
        if (e->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) continue;
        const RE::ButtonEvent* a_event = e->AsButtonEvent();
        if (a_event->IsPressed() || a_event->IsHeld()) continue;
        const RE::IDEvent* id_event = e->AsIDEvent();
        const auto& userEvent = id_event->userEvent;
        const auto userevents = RE::UserEvents::GetSingleton();
        if (userEvent != userevents->toggleFavorite && userEvent != userevents->yButton) continue;
        logger::trace("User event: {}", userEvent.c_str());
        M->UpdateFavorites();
        return RE::BSEventNotifyControl::kContinue;
    }
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl myEventSink::ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    if (event->newContainer!=player_refid) return RE::BSEventNotifyControl::kContinue;
    M->FavoriteCheck(event->baseObj);
    return RE::BSEventNotifyControl::kContinue;
}

void myEventSink::SaveCallback(SKSE::SerializationInterface* serializationInterface) {
    M->SendData();
    if (!M->Save(serializationInterface, Settings::kDataKey, Settings::kSerializationVersion)) {
        logger::critical("Failed to save Data");
    }
}

void myEventSink::LoadCallback(SKSE::SerializationInterface* serializationInterface){

    logger::info("Loading Data from skse co-save.");

    M->Reset();

    std::uint32_t type;
    std::uint32_t version;
    std::uint32_t length;

    bool cosave_found = false;
    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
        auto temp = Utils::DecodeTypeCode(type);

        if (version != Settings::kSerializationVersion) {
            logger::critical("Loaded data has incorrect version. Recieved ({}) - Expected ({}) for Data Key ({})",
                             version, Settings::kSerializationVersion, temp);
            continue;
        }
        switch (type) {
            case Settings::kDataKey: {
                logger::trace("Loading Record: {} - Version: {} - Length: {}", temp, version, length);
                if (!M->Load(serializationInterface))
                    logger::critical("Failed to Load Data for Manager");
                else cosave_found = true;
            } break;
            default:
                logger::critical("Unrecognized Record Type: {}", temp);
                break;
        }
    }

    if (cosave_found) {
        logger::info("Receiving Data.");
        M->ReceiveData();
        logger::info("Data loaded from skse co-save.");
    } else logger::info("No cosave data found.");

};



