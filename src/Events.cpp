#include "Events.h"


RE::BSEventNotifyControl myEventSink::ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!*evns) return RE::BSEventNotifyControl::kContinue;
    for (RE::InputEvent* e = *evns; e; e = e->next) {
        if (e->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) continue;
        const RE::ButtonEvent* a_event = e->AsButtonEvent();
        if (a_event->IsPressed() || a_event->IsHeld()) continue;
        const RE::IDEvent* id_event = e->AsIDEvent();
        const auto& userEvent = id_event->userEvent;
        const auto userevents = RE::UserEvents::GetSingleton();
        if (!IsHotkeyEvent(userEvent) && userEvent != userevents->toggleFavorite && userEvent != userevents->yButton) continue;
        logger::trace("User event: {}", userEvent.c_str());
        M->SyncFavorites();
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

RE::BSEventNotifyControl myEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                   RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    if (event->menuName != RE::FavoritesMenu::MENU_NAME &&
        event->menuName != RE::InventoryMenu::MENU_NAME &&
        event->menuName != RE::ContainerMenu::MENU_NAME)
        return RE::BSEventNotifyControl::kContinue;
    M->AddFavorites();
    return RE::BSEventNotifyControl::kContinue;
}

bool myEventSink::IsHotkeyEvent(const RE::BSFixedString& event_name){
    return Utils::Functions::String::includesString(std::string(event_name.c_str()), {"Hotkey"});
};

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
    unsigned int received_version = 0;
    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
        
        auto temp = Utils::DecodeTypeCode(type);
        if (!Settings::version_map.contains(version)) {
            logger::critical("Loaded data has incorrect version. Recieved ({}) - Expected ({}) for Data Key ({})",
                             version, Settings::kSerializationVersion, temp);
            continue;
        }
        

        switch (type) {
            case Settings::kDataKey: {
                received_version = Settings::version_map.at(version);
                logger::trace("Loading Record: {} - Version: {} - Length: {}", temp, version, length);
                if (!M->Load(serializationInterface, received_version)) logger::critical("Failed to Load Data for Manager");
                else cosave_found = true;
            } break;
            default:
                logger::critical("Unrecognized Record Type: {}", temp);
                break;
        }
    }

    if (cosave_found) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << received_version / 10.f;
        logger::info("Receiving Data from cosave with plugin version: {}.", oss.str());
        M->ReceiveData();
        logger::info("Data loaded from skse co-save.");
    } else logger::info("No cosave data found.");

};



