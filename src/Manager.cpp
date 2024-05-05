#include "Manager.h"


const bool Manager::RemoveFavorite(const FormID formid) {
	const auto removed = favorites.erase(formid);
	hotkey_map.erase(formid);
    return removed;
};
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

void Manager::UpdateHotkeyMap(const FormID spell_formid, const int a_hotkey) {
    if (IsHotkeyValid(a_hotkey)) {
		logger::trace("Hotkey found. FormID: {:x}, Hotkey: {}", spell_formid, a_hotkey);
		hotkey_map[spell_formid] = a_hotkey;
	}
}

const std::map<unsigned int, FormID> Manager::GetInventoryHotkeys() const { 
    std::map<unsigned int,FormID> hotkeys_in_use;
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (item.second.first <= 0) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (!item.second.second->extraLists || item.second.second->extraLists->empty()) continue;
        if (!item.second.second->IsFavorited()) continue;
        const auto hotkey_temp = GetHotkey(item.second.second.get());
        if (IsHotkeyValid(hotkey_temp)) {
            hotkeys_in_use[hotkey_temp] = item.first->GetFormID();
            logger::trace("GetInventoryHotkeys: FormID: {:x}, Hotkey: {}", item.first->GetFormID(), hotkey_temp);
        }
    }
    return hotkeys_in_use;
}

const std::map<FormID,unsigned int> Manager::GetMagicHotkeys() const { 
    std::map<FormID,unsigned int> hotkeys_in_use;
    const auto& mg_hotkeys = RE::MagicFavorites::GetSingleton()->hotkeys;
    unsigned int index = 0;
    for (auto& hotkeyed_spell : mg_hotkeys) {
        if (!hotkeyed_spell) {
			index++;
			continue;
		}
        if (IsHotkeyValid(index)) {
            hotkeys_in_use[hotkeyed_spell->GetFormID()] = index;
            logger::trace("GetMagicHotkeys: FormID: {:x}, Hotkey: {}", hotkeyed_spell->GetFormID(), index);
        }
		index++;
	}
	return hotkeys_in_use;
};

const std::map<unsigned int,FormID> Manager::GetHotkeysInUse() const {
    const auto& inventory_hotkeys = GetInventoryHotkeys();
    const auto& magic_hotkeys = GetMagicHotkeys();
    std::map<unsigned int, FormID> hotkeys_in_use;
    for (const auto& [hotkey, form_id] : inventory_hotkeys) {
        if (IsHotkeyValid(hotkey)) hotkeys_in_use[hotkey]=form_id;
    }
    for (const auto& [form_id,hotkey] : magic_hotkeys) {
		if (IsHotkeyValid(hotkey)) hotkeys_in_use[hotkey]=form_id;
	}
    return hotkeys_in_use;
};

const FormID Manager::HotkeyIsInUse(const FormID formid, const int a_hotkey) const {
    if (!IsHotkeyValid(a_hotkey)) {
        logger::error("Hotkey invalid. Hotkey: {}", a_hotkey);
        return formid;
    }
    const auto hotkeys_temp = GetHotkeysInUse();
    if (const auto u_hotkey = static_cast<unsigned int>(a_hotkey); hotkeys_temp.contains(u_hotkey)) {
        const auto used_by = hotkeys_temp.at(u_hotkey);
        logger::trace("Hotkey {} in use by {}", a_hotkey, used_by);
        return used_by;
	}
    return formid;
}

void Manager::HotkeySpell(RE::TESForm* form, const unsigned int hotkey) {
    const auto magic_favs = RE::MagicFavorites::GetSingleton();
    if (!magic_favs) {
		logger::error("HotkeySpell: MagicFavorites is null.");
		return;
	}
    magic_favs->SetFavorite(form);
    auto& hotkeys = magic_favs->hotkeys;
    unsigned int index = 0;
    for (auto& hotkeyed_spell : hotkeys) {
        if (!hotkeyed_spell && index == hotkey) {
            hotkeyed_spell = form;
            logger::trace("HotkeySpell: Hotkey set. FormID: {:x}, Hotkey: {}", form->GetFormID(), hotkey);
            return;
        }
        else if (hotkeyed_spell && hotkeyed_spell->GetFormID() == form->GetFormID()) {
			logger::trace("HotkeySpell: Hotkey already set. FormID: {:x}, Hotkey: {}", form->GetFormID(), hotkey);
			return;
		}
        index++;
    }
    logger::error("HotkeySpell: Failed to set hotkey. FormID: {:x}, Hotkey: {}", form->GetFormID(), hotkey);

}

void Manager::ApplyHotkey(const FormID formid) {
    if (!formid) return;
    if (!favorites.contains(formid)) {
        logger::trace("ApplyHotkey: Form not favorited. FormID: {:x}", formid);
        return;
    }
    if (!hotkey_map.contains(formid)) {
        logger::trace("ApplyHotkey: Hotkey not found. FormID: {:x}", formid);
        return;
    }
    const auto hotkey = hotkey_map.at(formid);
    if (!IsHotkeyValid(hotkey)) {
        logger::error("Hotkey invalid. FormID: {:x}, Hotkey: {}", formid, hotkey);
        hotkey_map.erase(formid);
		return;
    }
    if (const auto used_by = HotkeyIsInUse(formid, hotkey); used_by != formid) {
        logger::trace("Hotkey in use. FormID: {:x}, Hotkey: {}, used_by {:x}", formid, hotkey, used_by);
		hotkey_map.erase(formid);
        hotkey_map[used_by] = hotkey;
        return;
    }
    const auto spell = Utils::FunctionsSkyrim::GetFormByID<RE::SpellItem>(formid);
    if (spell && RE::PlayerCharacter::GetSingleton()->HasSpell(spell)) {
        HotkeySpell(spell, hotkey);
		return;
    }
    else if (spell) {
        logger::trace("Spell not found in player's spell list. FormID: {:x}", formid);
    }

    // Spell ended

    // Now items
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

void Manager::SyncHotkeys_Item() {
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
}

void Manager::SyncHotkeys_Spell() {
    ENABLE_IF_NOT_UNINSTALLED
    const auto& mg_favorites = RE::MagicFavorites::GetSingleton()->spells;
    const auto mg_hotkeys = GetMagicHotkeys();
    for (auto& spell : mg_favorites) {
        if (!spell) continue;
        if (std::strlen(spell->GetName()) == 0) continue;
        if (!mg_hotkeys.contains(spell->GetFormID())) continue;
        const auto spell_formid = spell->GetFormID();
        UpdateHotkeyMap(spell_formid, mg_hotkeys.at(spell_formid));
    }
}

void Manager::SyncHotkeys() {
    ENABLE_IF_NOT_UNINSTALLED
	SyncHotkeys_Item();
	SyncHotkeys_Spell();
}

const bool Manager::IsSpellFavorited(const FormID a_spell, const RE::BSTArray<RE::TESForm*>& favs) const {
    for (auto& fav : favs) {
		if (!fav) continue;
		if (fav->GetFormID() == a_spell) return true;
	}
	return false;
}

RE::BSContainer::ForEachResult Manager::Visit(RE::SpellItem* a_spell) {
    if (!a_spell) return RE::BSContainer::ForEachResult::kContinue;
    if (std::strlen(a_spell->GetName()) == 0) return RE::BSContainer::ForEachResult::kContinue;
    temp_all_spells.insert(a_spell->GetFormID());
    return RE::BSContainer::ForEachResult::kContinue;
}

void Manager::CollectPlayerSpells() {
    temp_all_spells.clear();
    const auto player = RE::PlayerCharacter::GetSingleton(); 
    player->VisitSpells(*this);
}

void Manager::AddFavorites_Item() {
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
        } else if (favorites.contains(item.first->GetFormID())) {
            Utils::FunctionsSkyrim::Inventory::FavoriteItem(item.first, player);
            ApplyHotkey(item.first->GetFormID());
        }
    }
}

void Manager::AddFavorites_Spell() {
    ENABLE_IF_NOT_UNINSTALLED
    
    const auto mg_favorites = RE::MagicFavorites::GetSingleton();
    const auto& favorited_spells = mg_favorites->spells;
    const auto hotkeyed_spells = GetMagicHotkeys();
    CollectPlayerSpells();
    if (temp_all_spells.empty()) {
        logger::warn("AddFavorites: No spells found.");
        return;
    }
    for (auto& spell_formid : temp_all_spells) {
        const auto spell = Utils::FunctionsSkyrim::GetFormByID(spell_formid);
        if (!spell) continue;
        //logger::trace("Player has spell: {}", spell->GetName());
        if (IsSpellFavorited(spell_formid, favorited_spells)) {
            if (favorites.insert(spell_formid).second) {
                logger::trace("Spell favorited. FormID: {:x}, EditorID: {}", spell_formid,
                              clib_util::editorID::get_editorID(spell));
            }
            if (hotkeyed_spells.contains(spell_formid)) UpdateHotkeyMap(spell_formid, hotkeyed_spells.at(spell_formid));
        } else if (favorites.contains(spell_formid)) {
            mg_favorites->SetFavorite(spell);
            ApplyHotkey(spell_formid);
        }
    }
    temp_all_spells.clear();
}

void Manager::AddFavorites() {
    ENABLE_IF_NOT_UNINSTALLED
    AddFavorites_Item();
    AddFavorites_Spell();
}

void Manager::SyncFavorites_Item(){
    ENABLE_IF_NOT_UNINSTALLED
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
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
        } else if (RemoveFavorite(item.first->GetFormID())) {
            logger::trace("Item erased. FormID: {:x}, EditorID: {}", item.first->GetFormID(),
                          clib_util::editorID::get_editorID(item.first));
        }
    }
}

void Manager::SyncFavorites_Spell(){
    ENABLE_IF_NOT_UNINSTALLED
    const auto& favorited_spells = RE::MagicFavorites::GetSingleton()->spells;
    const auto hotkeyed_spells = GetMagicHotkeys();
    CollectPlayerSpells();
    if (temp_all_spells.empty()) {
        logger::warn("SyncFavorites: No spells found.");
        return;
    }
    for (auto& spell_formid : temp_all_spells) {
        const auto spell = Utils::FunctionsSkyrim::GetFormByID(spell_formid);
        if (!spell) continue;
        logger::trace("Player has spell: {}", spell->GetName());
        if (IsSpellFavorited(spell_formid, favorited_spells)) {
            if (favorites.insert(spell_formid).second) {
                logger::trace("Spell favorited. FormID: {:x}, EditorID: {}", spell_formid,
                              clib_util::editorID::get_editorID(spell));
            }
            if (hotkeyed_spells.contains(spell_formid)) UpdateHotkeyMap(spell_formid, hotkeyed_spells.at(spell_formid));
        } else if (RemoveFavorite(spell_formid)) {
            logger::trace("Spell erased. FormID: {:x}, EditorID: {}", spell_formid,
                          clib_util::editorID::get_editorID(spell));
        }
    }
    temp_all_spells.clear();
};

void Manager::SyncFavorites() {
    ENABLE_IF_NOT_UNINSTALLED
    SyncFavorites_Item();
    SyncFavorites_Spell();
}

void Manager::FavoriteCheck_Item(const FormID formid) {
    ENABLE_IF_NOT_UNINSTALLED
    if (!favorites.contains(formid)) return;
    const auto bound = Utils::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid);
    if (!bound) {
        logger::warn("FavoriteCheck_Item: Form not found. FormID: {}", formid);
        RemoveFavorite(formid);
        return;
    }
    Utils::FunctionsSkyrim::Inventory::FavoriteItem(bound, RE::PlayerCharacter::GetSingleton());
    ApplyHotkey(bound->GetFormID());
}

void Manager::FavoriteCheck_Spell(const FormID formid){
    if (!favorites.contains(formid)) {
        logger::trace("FavoriteCheck_Spell: Form not favorited. FormID: {:x}", formid);
        return;
    }
    const auto spell = Utils::FunctionsSkyrim::GetFormByID(formid);
    if (!spell) {
		logger::warn("FavoriteCheck_Spell: Form not found. FormID: {}", formid);
		RemoveFavorite(formid);
		return;
	}
    logger::trace("FavoriteCheck_Spell: Favoriting spell. FormID: {:x}, EditorID: {}", formid, clib_util::editorID::get_editorID(spell));
    RE::MagicFavorites::GetSingleton()->SetFavorite(spell);
    logger::trace("FavoriteCheck_Spell: Applying hotkey. FormID: {:x}", formid);
    logger::info("spell name {}", spell->GetName());
	ApplyHotkey(formid);
};

void Manager::FavoriteCheck_Spell(){
    CollectPlayerSpells();
    if (temp_all_spells.empty()) {
		logger::warn("FavoriteCheck_Spell: No spells found.");
		return;
	}
    for (auto& spell_formid : temp_all_spells) {
        FavoriteCheck_Spell(spell_formid);
    }
    temp_all_spells.clear();
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
