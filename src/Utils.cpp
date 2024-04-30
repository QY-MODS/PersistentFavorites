
#pragma once

#include "Utils.h"

namespace Utils {

    std::string DecodeTypeCode(std::uint32_t typeCode) {
        char buf[4];
        buf[3] = char(typeCode);
        buf[2] = char(typeCode >> 8);
        buf[1] = char(typeCode >> 16);
        buf[0] = char(typeCode >> 24);
        return std::string(buf, buf + 4);
    };

    namespace MsgBoxesNotifs {
        namespace Windows {

            int Po3ErrMsg() {
                MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
                return 1;
            }
        };
    };

    namespace Functions {
        namespace String {
            std::vector<std::pair<int, bool>> encodeString(const std::string& inputString) {
                std::vector<std::pair<int, bool>> encodedValues;
                try {
                    for (int i = 0; i < 100 && inputString[i] != '\0'; i++) {
                        char ch = inputString[i];
                        if (std::isprint(ch) && (std::isalnum(ch) || std::isspace(ch) || std::ispunct(ch)) && ch >= 0 &&
                            ch <= 255) {
                            encodedValues.push_back(std::make_pair(static_cast<int>(ch), std::isupper(ch)));
                        }
                    }
                } catch (const std::exception& e) {
                    logger::error("Error encoding string: {}", e.what());
                    return encodeString("ERROR");
                }
                return encodedValues;
            }

            std::string decodeString(
                const std::vector<std::pair<int, bool>>& encodedValues) {
                std::string decodedString;
                for (const auto& pair : encodedValues) {
                    char ch = static_cast<char>(pair.first);
                    if (std::isalnum(ch) || std::isspace(ch) || std::ispunct(ch)) {
                        if (pair.second) {
                            decodedString += ch;
                        } else {
                            decodedString += static_cast<char>(std::tolower(ch));
                        }
                    }
                }
                return decodedString;
            }
        };
    }; 

    namespace FunctionsSkyrim {

        RE::TESForm* GetFormByID(const FormID id, const std::string& editor_id) {
            if (!editor_id.empty()) {
                auto* form = RE::TESForm::LookupByEditorID(editor_id);
                if (form) return form;
            }
            auto form = RE::TESForm::LookupByID(id);
            if (form) return form;
            return nullptr;
        };

        template <class T>
        static T* GetFormByID(const FormID id, const std::string& editor_id) {
            if (!editor_id.empty()) {
                auto* form = RE::TESForm::LookupByEditorID<T>(editor_id);
                if (form) return form;
            }
            T* form = RE::TESForm::LookupByID<T>(id);
            if (form) return form;
            return nullptr;
        };

        const std::string GetEditorID(const FormID a_formid) {
            if (const auto form = RE::TESForm::LookupByID(a_formid)) {
                return clib_util::editorID::get_editorID(form);
            } else {
                return "";
            }
        }

        namespace Inventory {

            const bool HasItemEntry(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner,
                                    bool nonzero_entry_check) {
                if (!item) {
                    logger::warn("Item is null");
                    return 0;
                }
                if (!inventory_owner) {
                    logger::warn("Inventory owner is null");
                    return 0;
                }
                auto inventory = inventory_owner->GetInventory();
                auto it = inventory.find(item);
                bool has_entry = it != inventory.end();
                if (nonzero_entry_check) return has_entry && it->second.first > 0;
                return has_entry;
            }

            const bool HasItem(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner) {
                if (HasItemEntry(item, item_owner, true)) return true;
                return false;
            }

            void FavoriteItem(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
                if (!item) return;
                if (!inventory_owner) return;
                auto inventory_changes = inventory_owner->GetInventoryChanges();
                auto entries = inventory_changes->entryList;
                for (auto it = entries->begin(); it != entries->end(); ++it) {
                    if (!(*it)) {
                        logger::error("Item entry is null");
                        continue;
                    }
                    const auto object = (*it)->object;
                    if (!object) {
                        logger::error("Object is null");
                        continue;
                    }
                    auto formid = object->GetFormID();
                    if (!formid) logger::critical("Formid is null");
                    if (formid == item->GetFormID()) {
                        logger::trace("Favoriting item: {}", item->GetName());
                        const auto xLists = (*it)->extraLists;
                        bool no_extra_ = false;
                        if (!xLists || xLists->empty()) {
                            logger::trace("No extraLists");
                            no_extra_ = true;
                        }
                        logger::trace("asdasd");
                        if (no_extra_) {
                            logger::trace("No extraLists");
                            // inventory_changes->SetFavorite((*it), nullptr);
                        } else if (xLists->front()) {
                            logger::trace("ExtraLists found");
                            inventory_changes->SetFavorite((*it), xLists->front());
                        }
                        return;
                    }
                }
                logger::error("Item not found in inventory");
            }

            void FavoriteItem(const FormID formid, const FormID refid) {
                FavoriteItem(GetFormByID<RE::TESBoundObject>(formid), GetFormByID<RE::TESObjectREFR>(refid));
            }

            [[nodiscard]] const bool HasItemPlusCleanUp(RE::TESBoundObject* item, RE::TESObjectREFR* item_owner,
                                                        RE::ExtraDataList* xList) {
                logger::trace("HasItemPlusCleanUp");

                if (HasItem(item, item_owner)) return true;
                if (HasItemEntry(item, item_owner)) {
                    item_owner->RemoveItem(item, 1, RE::ITEM_REMOVE_REASON::kRemove, xList, nullptr);
                    logger::trace("Item with zero count removed from player.");
                }
                return false;
            }

            [[nodiscard]] const bool IsFavorited(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
                if (!item) {
                    logger::warn("Item is null");
                    return false;
                }
                if (!inventory_owner) {
                    logger::warn("Inventory owner is null");
                    return false;
                }
                auto inventory = inventory_owner->GetInventory();
                auto it = inventory.find(item);
                if (it != inventory.end()) {
                    if (it->second.first <= 0) logger::warn("Item count is 0");
                    return it->second.second->IsFavorited();
                }
                return false;
            }

            [[nodiscard]] const bool IsFavorited(RE::FormID formid, RE::FormID refid) {
                return IsFavorited(GetFormByID<RE::TESBoundObject>(formid), GetFormByID<RE::TESObjectREFR>(refid));
            }

            [[nodiscard]] const bool IsPlayerFavorited(RE::TESBoundObject* item) {
                return IsFavorited(item, RE::PlayerCharacter::GetSingleton()->AsReference());
            }
        };

    };

    bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str) {
        std::vector<std::pair<int, bool>> encodedStr;
        std::size_t size;
        if (!a_intfc->ReadRecordData(size)) {
            return false;
        }
        for (std::size_t i = 0; i < size; i++) {
            std::pair<int, bool> temp_pair;
            if (!a_intfc->ReadRecordData(temp_pair)) {
                return false;
            }
            encodedStr.push_back(temp_pair);
        }
        a_str = Utils::Functions::String::decodeString(encodedStr);
        return true;
    }

    bool write_string(SKSE::SerializationInterface* a_intfc, const std::string& a_str) {
        const auto encodedStr = Utils::Functions::String::encodeString(a_str);
        // i first need the size to know no of iterations
        const auto size = encodedStr.size();
        if (!a_intfc->WriteRecordData(size)) {
            return false;
        }
        for (const auto& temp_pair : encodedStr) {
            if (!a_intfc->WriteRecordData(temp_pair)) {
                return false;
            }
        }
        return true;
    }

    
};

