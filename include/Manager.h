
#pragma once
#include "Serialization.h"

#define ENABLE_IF_NOT_UNINSTALLED if (isUninstalled) return;

class Manager : public SaveLoadData, public RE::Actor::ForEachSpellVisitor {

    std::set<FormID> favorites;
    std::map<FormID, unsigned int> hotkey_map;
    std::set<FormID> temp_all_spells;

    bool isUninstalled = false;

    const std::set<unsigned int> allowed_hotkeys = {0,1,2,3,4,5,6,7};
    
    const bool RemoveFavorite(const FormID formid);

    const int GetHotkey(const RE::InventoryEntryData* a_entry) const ;

    const bool IsHotkeyValid(const int hotkey) const;

    void UpdateHotkeyMap(const FormID item_formid, const RE::InventoryEntryData* a_entry);

    void UpdateHotkeyMap(const FormID spell_formid, const int a_hotkey);

    const std::set<unsigned int> GetInventoryHotkeys() const;

    const std::map<FormID, unsigned int> GetMagicHotkeys() const;
    
    const std::set<unsigned int> GetHotkeysInUse() const;

    const bool HotkeyIsInUse(const int hotkey) const;

    void HotkeySpell(RE::TESForm* form, const unsigned int hotkey);

    void ApplyHotkey(const FormID formid);

    void SyncHotkeys_Bound();

    void SyncHotkeys_Spell();

    void SyncHotkeys();

    const bool IsSpellFavorited(const FormID a_spell,const RE::BSTArray<RE::TESForm*>& favs) const;

    RE::BSContainer::ForEachResult Visit(RE::SpellItem* a_spell);

    void CollectPlayerSpells();

public:
    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }
    
    const char* GetType() override { return "Manager"; }

    void AddFavorites_Bound();
    
    void AddFavorites_Spell();

    void AddFavorites();

    void SyncFavorites_Bound();

    void SyncFavorites_Spell();

    void SyncFavorites();

    void FavoriteCheck_Bound(const FormID formid);

    void FavoriteCheck_Spell(const FormID formid);

    void FavoriteCheck_Spell();
    
    inline void Uninstall() { isUninstalled = true; };

    void Reset();

    void SendData();

    void ReceiveData();

};