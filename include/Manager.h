
#pragma once


#include "Serialization.h"

#define ENABLE_IF_NOT_UNINSTALLED if (isUninstalled) return;

class Manager : public SaveLoadData {

    std::set<FormID> favorites;

    bool isUninstalled = false;

public:
    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }
    
    const char* GetType() override { return "Manager"; }

    void UpdateFavorites();

    void FavoriteCheck(const FormID formid);

    inline void Uninstall() { isUninstalled = true; };

    void Reset();

    void SendData();

    void ReceiveData();

};
