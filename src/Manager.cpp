#include "Utils.h"
#include "Manager.h"
#include "Serialization.h"


void Manager::UpdateFavorites() {
    ENABLE_IF_NOT_UNINSTALLED
    const auto player_inventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    for (auto& item : player_inventory) {
        if (!item.first) continue;
        if (std::strlen(item.first->GetName()) == 0) continue;
        if (!item.second.second) continue;
        if (item.second.second->IsFavorited()) {
            if (favorites.insert(item.first->GetFormID()).second) {
                logger::trace("Item favorited. FormID: {:x}, EditorID: {}", item.first->GetFormID(), clib_util::editorID::get_editorID(item.first));
            }
        }
        else if (favorites.contains(item.first->GetFormID())) {
            if (favorites.erase(item.first->GetFormID())) {
                logger::trace("Item erased. FormID: {:x}, EditorID: {}", item.first->GetFormID(),
                              clib_util::editorID::get_editorID(item.first));
            }
        }
    }
}

void Manager::FavoriteCheck(const FormID formid){
    ENABLE_IF_NOT_UNINSTALLED
    if (!favorites.contains(formid)) return;
    const auto bound = Utils::FunctionsSkyrim::GetFormByID<RE::TESBoundObject>(formid);
    if (!bound) {
		logger::warn("FavoriteCheck: Form not found. FormID: {}", formid);
		favorites.erase(formid);
		return;
	}
    Utils::FunctionsSkyrim::Inventory::FavoriteItem(bound, RE::PlayerCharacter::GetSingleton());

};

void Manager::Reset() {
    ENABLE_IF_NOT_UNINSTALLED
    logger::info("Resetting manager...");
    favorites.clear();
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
        if (temp_editorid.empty()) continue;
        SaveDataLHS lhs({fav_id, temp_editorid});
        SetData(lhs, true);
        n_instances++;
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
        if (!rhs) continue;
        auto source_formid = lhs.first;
        const auto& source_editorid = lhs.second;

        if (!source_formid) {
            logger::error("ReceiveData: Formid is null.");
            continue;
        }
        if (source_editorid.empty()) {
            logger::error("ReceiveData: Editorid is empty.");
            continue;
        }
        const auto source_form = Utils::FunctionsSkyrim::GetFormByID(0, source_editorid);
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

        if (favorites.contains(source_formid)) {
			logger::warn("ReceiveData: Form already favorited. FormID: {}, EditorID: {}", source_formid, source_editorid);
			continue;
        }
        else favorites.insert(source_formid);

		logger::info("FormID: {}, EditorID: {}", source_formid, source_editorid);
		n_instances++;
    }
};