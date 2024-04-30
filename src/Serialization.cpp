
#include "Serialization.h"
#include "Utils.h"

template <typename T, typename U>
float BaseData<T, U>::GetData(T formId, T missing) {
    Locker locker(m_Lock);
    if (auto idx = m_Data.find(formId) != m_Data.end()) {
        return m_Data[formId];
    }
    return missing;
}

template <typename T, typename U>
void BaseData<T, U>::SetData(T formId, U value) {
    Locker locker(m_Lock);
    m_Data[formId] = value;
}
template <typename T, typename U>
void BaseData<T, U>::Clear() {
    Locker locker(m_Lock);
    m_Data.clear();
};


[[nodiscard]] bool SaveLoadData::Save(SKSE::SerializationInterface* serializationInterface) {
    assert(serializationInterface);
    Locker locker(m_Lock);

    const auto numRecords = m_Data.size();
    if (!serializationInterface->WriteRecordData(numRecords)) {
        logger::error("Failed to save {} data records", numRecords);
        return false;
    }

    for (const auto& [lhs, rhs] : m_Data) {
        std::uint32_t formid = lhs.first;
        logger::trace("Formid:{}", formid);
        if (!serializationInterface->WriteRecordData(formid)) {
            logger::error("Failed to save formid");
            return false;
        }

        const std::string editorid = lhs.second;
        logger::trace("Editorid:{}", editorid);
        Utils::write_string(serializationInterface, editorid);
    }
    return true;
}

[[nodiscard]] bool SaveLoadData::Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
                        std::uint32_t version) {
    if (!serializationInterface->OpenRecord(type, version)) {
        logger::error("Failed to open record for Data Serialization!");
        return false;
    }

    return Save(serializationInterface);
}

[[nodiscard]] bool SaveLoadData::Load(SKSE::SerializationInterface* serializationInterface) {
    assert(serializationInterface);

    std::size_t recordDataSize;
    serializationInterface->ReadRecordData(recordDataSize);
    logger::info("Loading data from serialization interface with size: {}", recordDataSize);

    Locker locker(m_Lock);
    m_Data.clear();

    logger::trace("Loading data from serialization interface.");
    for (auto i = 0; i < recordDataSize; i++) {
        std::uint32_t formid = 0;
        logger::trace("ReadRecordData:{}", serializationInterface->ReadRecordData(formid));
        if (!serializationInterface->ResolveFormID(formid, formid)) {
            logger::error("Failed to resolve form ID, 0x{:X}.", formid);
            continue;
        }

        std::string editorid;
        if (!Utils::read_string(serializationInterface, editorid)) {
            logger::error("Failed to read editorid");
            return false;
        }

        SaveDataLHS lhs({formid, editorid});
        logger::trace("Reading value...");

        m_Data[lhs] = true;
        logger::info("Loaded data for formid {}, editorid {}", formid, editorid);
    }

    return true;
}
