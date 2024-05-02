
#pragma once
#include <windows.h>
#include <ClibUtil/editorID.hpp>

namespace Utils {
    const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
    constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
    inline bool IsPo3Installed() { return std::filesystem::exists(po3path); };
    const auto po3_err_msgbox = std::format(
        "{}: You must have powerofthree's Tweaks "
        "installed. See mod page for further instructions.",
        mod_name);

    std::string DecodeTypeCode(std::uint32_t typeCode);

    std::string GetPluginVersion(const unsigned int n_stellen);


    namespace MsgBoxesNotifs {
        namespace Windows {
            int Po3ErrMsg();
        };
    };

    namespace Functions {

        namespace String {

            std::vector<std::pair<int, bool>> encodeString(const std::string& inputString);

            std::string decodeString(const std::vector<std::pair<int, bool>>& encodedValues);

            std::string toLowercase(const std::string& str);

            bool includesString(const std::string& input, const std::vector<std::string>& strings);

        };
    };

    namespace FunctionsSkyrim {
        
        RE::TESForm* GetFormByID(const FormID id, const std::string& editor_id = "");

        template <class T>
        static T* GetFormByID(const FormID id, const std::string& editor_id = "") {
            if (!editor_id.empty()) {
                auto* form = RE::TESForm::LookupByEditorID<T>(editor_id);
                if (form) return form;
            }
            T* form = RE::TESForm::LookupByID<T>(id);
            if (form) return form;
            return nullptr;
        };

        const std::string GetEditorID(const FormID a_formid);

        namespace Inventory {

            const bool HasItemEntry(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner,
                                    bool nonzero_entry_check = false);

            const bool HasItem(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner);

            void FavoriteItem(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner);

            void FavoriteItem(const FormID formid, const FormID refid);

            [[nodiscard]] const bool HasItemPlusCleanUp(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner,
                                                        RE::ExtraDataList* xList = nullptr);

            [[nodiscard]] const bool IsFavorited(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner);

            [[nodiscard]] const bool IsFavorited(RE::FormID formid, RE::FormID refid);

            [[nodiscard]] const bool IsPlayerFavorited(RE::TESBoundObject* item);
        };
    
    };
    
    bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str);

    bool write_string(SKSE::SerializationInterface* a_intfc, const std::string& a_str);

};