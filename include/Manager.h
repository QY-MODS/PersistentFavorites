
#pragma once

#include "Serialization.h"


class Manager : public SaveLoadData {
    static Manager* GetSingleton() {
        static Manager singleton;
        return &singleton;
    }
    
    const char* GetType() override { return "Manager"; }
};
