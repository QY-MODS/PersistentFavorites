
#pragma once


#include "Serialization.h"

#define ENABLE_IF_NOT_UNINSTALLED if (isUninstalled) return;

class Manager : public SaveLoadData {

    std::set<FormID> favorites;

    bool isUninstalled = false;

    std::map<FormID, unsigned int> hotkey_map;

    std::set<unsigned int> allowed_hotkeys = {0,1,2,3,4,5,6,7};

    const int GetHotkey(const RE::InventoryEntryData* a_entry) const ;

    const bool IsHotkeyValid(const int hotkey) const;

    void UpdateHotkeyMap(const FormID item_formid, const RE::InventoryEntryData* a_entry);
    
    const bool HotkeyIsInUse(const int hotkey);

    void ApplyHotkey(const FormID formid);

    void SyncHotkeys();

public:
    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }
    
    const char* GetType() override { return "Manager"; }

    void AddFavorites();

    void SyncFavorites();

    void FavoriteCheck(const FormID formid);

    const bool RemoveFavorite(const FormID formid);

    inline void Uninstall() { isUninstalled = true; };

    void Reset();

    void SendData();

    void ReceiveData();

};
