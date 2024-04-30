
#include "Events.h"
#include "Settings.h"

auto* eventSink = myEventSink::GetSingleton();

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Start
        if (!Utils::IsPo3Installed()) {
            logger::error("Po3 is not installed.");
            Utils::MsgBoxesNotifs::Windows::Po3ErrMsg();
            return;
        }
    }
    if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
        // Post-load
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(eventSink);
        auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);

        logger::info("Event sink added.");
    }
}

static void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
#endif
    logger::info("Name of the plugin is {}.", pluginName);
    logger::info("Version of the plugin is {}.", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
}


void SaveCallback(SKSE::SerializationInterface* serializationInterface) {
    eventSink->SaveCallback(serializationInterface);
};

void LoadCallback(SKSE::SerializationInterface* serializationInterface) {
	eventSink->LoadCallback(serializationInterface);
};

void InitializeSerialization() {
    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(Settings::kDataKey);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    SKSE::log::trace("Cosave serialization initialized.");
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}