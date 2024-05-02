#include "Manager.h"


const int Manager::GetHotkey(const RE::InventoryEntryData* a_entry) const { 
    if (!a_entry) {
        logger::warn("GetHotkey: Entry is null.");
        return -1;
    }
    if (!a_entry->IsFavorited()) {
        logger::warn("GetHotkey: Entry is not favorited.");
        return -1;
    }
    if (!a_entry->extraLists || a_entry->extraLists->empty()) {
        logger::warn("GetHotkey: Entry has no extraLists.");
        return -1;
    }
    for (auto& extraList : *a_entry->extraLists) {
		if (!extraList) continue;
        if (extraList->HasType(RE::ExtraDataType::kHotkey)) {
            const auto hotkey = extraList->GetByType<RE::ExtraHotkey>()->hotkey.underlying();
            if (!IsHotkeyValid(hotkey)) continue;
            logger::trace("GetHotkey: Hotkey found: {}", hotkey);
            return hotkey;
        }
    }
    return -1;
}

const inline bool Manager::IsHotkeyValid(const int hotkey) const { 
    return allowed_hotkeys.contains(hotkey);
}

void Manager::UpdateHotkeyMap(const FormID item_formid, const RE::InventoryEntryData* a_entry) {
    const auto hotkey = GetHotkey(a_entry);
    if (IsHotkeyValid(hotkey)) {
        logger::trace("Hotkey found. FormID: {:x}, Hotkey: {}", item_formid, hotkey);
        hotkey_map[item_formid] = hotkey;
    }
}

const bool Manager::HotkeyIsInUse(const int a_hotkey) const {
    if (!IsHotkeyValid(a_hotkey)) {
        logger::error("Hotkey invalid. Hotkey: {}", a_hotkey);
        return false;
    }
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (!item.second.second->extraLists || item.second.second->extraLists->empty()) continue;
        if (!item.second.second->IsFavorited()) continue;
        const auto hotkey_in_use = GetHotkey(item.second.second.get());
        if (IsHotkeyValid(hotkey_in_use) && hotkey_in_use == a_hotkey) return true;
    }
    return false;
}

void Manager::ApplyHotkey(const FormID formid) {
    if (!formid) return;
    if (!favorites.contains(formid)) return;
    if (!hotkey_map.contains(formid)) return;
    const auto hotkey = hotkey_map.at(formid);
    if (!IsHotkeyValid(hotkey)) {
        logger::error("Hotkey invalid. FormID: {:x}, Hotkey: {}", formid, hotkey);
        hotkey_map.erase(formid);
		return;
    }
    if (HotkeyIsInUse(hotkey)) {
		logger::trace("Hotkey in use. FormID: {:x}, Hotkey: {}", formid, hotkey);
		hotkey_map.erase(formid);
        return;
    }
    const auto bound = Utils::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid);
    if (!bound) {
        logger::error("ApplyHotkey: Form not found. FormID: {:x}", formid);
        return;
    }
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    const auto item = player_inventory.find(bound);
    if (item == player_inventory.end()) {
		logger::trace("ApplyHotkey: Item not found in inventory. FormID: {:x}", formid);
		return;
	}
    auto* xList = item->second.second->extraLists->front();
    if (!xList) {
		logger::error("ApplyHotkey: ExtraList is null. FormID: {:x}", formid);
		return;
	}
    if (xList->HasType(RE::ExtraDataType::kHotkey)) {
        auto* old_xHotkey = xList->GetByType<RE::ExtraHotkey>();
        const auto old_hotkey = static_cast<int>(old_xHotkey->hotkey.get());
        logger::trace("ApplyHotkey: Hotkey already exists. FormID: {:x}, old Hotkey: {}", formid, old_hotkey);
        //old_xHotkey->hotkey.reset(static_cast<RE::ExtraHotkey::Hotkey>(hotkey));
        old_xHotkey->hotkey = static_cast<RE::ExtraHotkey::Hotkey>(hotkey);
    } else {
        logger::trace("ApplyHotkey: Creating hotkey. FormID: {:x}, Hotkey: {}", formid, hotkey);
        RE::ExtraHotkey* xHotkey = RE::ExtraHotkey::Create<RE::ExtraHotkey>();
        if (!xHotkey) {
            logger::error("ApplyHotkey: Failed to create hotkey. FormID: {:x}", formid);
            return;
        }
        xHotkey->hotkey = static_cast<RE::ExtraHotkey::Hotkey>(hotkey);
        if (static_cast<uint8_t>(xHotkey->hotkey.get()) != hotkey) {
            logger::error("ApplyHotkey: Failed to set hotkey. FormID: {:x}, Hotkey: {}", formid, hotkey);
            delete xHotkey;
            return;
        }
        logger::trace("ApplyHotkey: Adding hotkey. FormID: {:x}, Hotkey: {}", formid, hotkey);
        xList->Add(xHotkey);
    }
    const int added_hotkey = GetHotkey(item->second.second.get());
    if (added_hotkey != static_cast<int>(hotkey)) {
        logger::error("ApplyHotkey: Failed to add hotkey. FormID: {:x}, Hotkey: {}, added:{}", formid, hotkey,
                      added_hotkey);
		return;
	}
    logger::trace("Hotkey applied. FormID: {:x}, Hotkey: {}", formid, hotkey);
    
}

void Manager::SyncHotkeys() {
    ENABLE_IF_NOT_UNINSTALLED
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (!item.second.second->extraLists || item.second.second->extraLists->empty()) continue;
        if (!item.second.second->IsFavorited()) continue;
        UpdateHotkeyMap(item.first->GetFormID(), item.second.second.get());
    }
};

void Manager::AddFavorites() {
    ENABLE_IF_NOT_UNINSTALLED
    const auto player = RE::PlayerCharacter::GetSingleton();
    const auto player_inventory = player->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (item.second.second->IsFavorited()) {
            if (favorites.insert(item.first->GetFormID()).second) {
                logger::trace("Item favorited. FormID: {:x}, EditorID: {}", item.first->GetFormID(),
                              clib_util::editorID::get_editorID(item.first));
            }
            UpdateHotkeyMap(item.first->GetFormID(), item.second.second.get());
        }
        else if (favorites.contains(item.first->GetFormID())) {
            Utils::FunctionsSkyrim::Inventory::FavoriteItem(item.first, player);
            ApplyHotkey(item.first->GetFormID());
		}
    }
};

void Manager::SyncFavorites() {
    ENABLE_IF_NOT_UNINSTALLED
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (item.second.second->IsFavorited()) {
            if (favorites.insert(item.first->GetFormID()).second) {
                logger::trace("Item favorited. FormID: {:x}, EditorID: {}", item.first->GetFormID(), clib_util::editorID::get_editorID(item.first));
            }
            UpdateHotkeyMap(item.first->GetFormID(), item.second.second.get());
        } 
        else if (RemoveFavorite(item.first->GetFormID())) {
            logger::trace("Item erased. FormID: {:x}, EditorID: {}", item.first->GetFormID(),
                            clib_util::editorID::get_editorID(item.first));
        }
    }
};

void Manager::FavoriteCheck(const FormID formid){
    ENABLE_IF_NOT_UNINSTALLED
    if (!favorites.contains(formid)) return;
    const auto bound = Utils::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid);
    if (!bound) {
		logger::warn("FavoriteCheck: Form not found. FormID: {}", formid);
        RemoveFavorite(formid);
		return;
	}
    Utils::FunctionsSkyrim::Inventory::FavoriteItem(bound, RE::PlayerCharacter::GetSingleton());
    ApplyHotkey(bound->GetFormID());

}

const bool Manager::RemoveFavorite(const FormID formid) {
	const auto removed = favorites.erase(formid);
	hotkey_map.erase(formid);
    return removed;
};

void Manager::Reset() {
    ENABLE_IF_NOT_UNINSTALLED
    logger::info("Resetting manager...");
    favorites.clear();
    hotkey_map.clear();
    Clear();
    logger::info("Manager reset.");
};

void Manager::SendData() {
    ENABLE_IF_NOT_UNINSTALLED
    // std::lock_guard<std::mutex> lock(mutex);
    logger::info("--------Sending data---------");
    Clear();

    int n_instances = 0;
    for (auto& fav_id : favorites) {
        const auto temp_form = Utils::FunctionsSkyrim::GetFormByID(fav_id);
        if (!temp_form) continue;
        const auto temp_editorid = clib_util::editorID::get_editorID(temp_form);
        //if (temp_editorid.empty()) continue;
        SaveDataLHS lhs({fav_id, temp_editorid});
        const int rhs = hotkey_map.contains(fav_id) && IsHotkeyValid(hotkey_map.at(fav_id)) ? hotkey_map.at(fav_id) : -1;
        SetData(lhs, rhs);
        n_instances++;
        if (n_instances >= Settings::instance_limit) {
			logger::warn("SendData: Instance limit reached. Number of instances: {}", n_instances);
			break;
		}
    }
    logger::info("Data sent. Number of instances: {}", n_instances);
}

void Manager::ReceiveData() {
    ENABLE_IF_NOT_UNINSTALLED
    logger::info("--------Receiving data---------");
    if (m_Data.empty()) {
        logger::warn("ReceiveData: No data to receive.");
        return;
    }

    int n_instances = 0;
    for (const auto& [lhs, rhs] : m_Data) {
        
        auto source_formid = lhs.first;
        auto source_editorid = lhs.second;

        if (!source_formid) {
            logger::error("ReceiveData: Formid is null.");
            continue;
        }
        //if (source_editorid.empty()) {
        //    logger::error("ReceiveData: Editorid is empty.");
        //    continue;
        //}
        const auto source_form = Utils::FunctionsSkyrim::GetFormByID(source_formid, source_editorid);
        if (!source_form) {
            logger::critical("ReceiveData: Source form not found. Saved formid: {}, editorid: {}", source_formid,
                             source_editorid);
            continue;
        }
        if (source_form->GetFormID() != source_formid) {
            logger::warn("ReceiveData: Source formid does not match. Saved formid: {}, editorid: {}", source_formid,
                         source_editorid);
            source_formid = source_form->GetFormID();
        }
        if (source_editorid.empty()) {
			source_editorid = clib_util::editorID::get_editorID(source_form);
		}

        if (favorites.contains(source_formid)) {
			logger::warn("ReceiveData: Form already favorited. FormID: {}, EditorID: {}", source_formid, source_editorid);
			continue;
        }
        else favorites.insert(source_formid);

        if (IsHotkeyValid(rhs)) hotkey_map[source_formid] = rhs;

		logger::info("FormID: {}, EditorID: {}", source_formid, source_editorid);
		n_instances++;
    }

    SyncHotkeys();

    logger::info("Data received. Number of instances: {}", n_instances);
};