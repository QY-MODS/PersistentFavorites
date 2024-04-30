#include "Utils.h"


using SaveDataLHS = std::pair<RE::FormID, std::string>;
using SaveDataRHS = bool;


// github.com/ozooma10/OSLAroused/blob/29ac62f220fadc63c829f6933e04be429d4f96b0/src/PersistedData.cpp
template <typename T, typename U>
// BaseData is based off how powerof3's did it in Afterlife
class BaseData {
public:
    float GetData(T formId, T missing);

    void SetData(T formId, U value);

    virtual const char* GetType() = 0;

    virtual bool Save(SKSE::SerializationInterface*, std::uint32_t, std::uint32_t) { return false; };
    virtual bool Save(SKSE::SerializationInterface*) { return false; };
    virtual bool Load(SKSE::SerializationInterface*) { return false; };

    void Clear();

    virtual void DumpToLog() = 0;

protected:
    std::map<T, U> m_Data;

    using Lock = std::recursive_mutex;
    using Locker = std::lock_guard<Lock>;
    mutable Lock m_Lock;
};

class SaveLoadData : public BaseData<SaveDataLHS, SaveDataRHS> {
public:
    void DumpToLog() override {
        // nothing for now
    }

    [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface) override;

    [[nodiscard]] bool Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
                            std::uint32_t version) override;

    [[nodiscard]] bool Load(SKSE::SerializationInterface* serializationInterface) override;
};