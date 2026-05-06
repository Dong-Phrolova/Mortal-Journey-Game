#pragma execution_character_set("utf-8")
#include "ItemSystem.h"
#include "Player.h"
#include "ConfigManager.h"
#include "GameCallbacks.h"

int InventorySystem::AddItem(const std::string& itemId, int count) {
    if (count <= 0) return 0;
    const auto* data = ConfigManager::Instance().GetItem(itemId);
    if (!data) return 0; // 未知物品

    // 可堆叠：寻找已有堆
    if (data->stackable > 1) {
        for (auto& item : m_items) {
            if (item.itemId == itemId) {
                int space = data->stackable - item.count;
                int add = std::min(count, space);
                item.count += add;
                if (add < count) {
                    int remaining = count - add;
                    int added = AddItem(itemId, remaining);
                    OnGetItem(itemId, GetItemCount(itemId));
                    return add + added;
                }
                OnGetItem(itemId, GetItemCount(itemId));
                return count;
            }
        }
    }

    // 新格子
    if ((int)m_items.size() >= MAX_SLOTS) return 0;
    int toAdd = std::min(count, data->stackable);
    m_items.push_back({itemId, toAdd});
    OnGetItem(itemId, GetItemCount(itemId));
    return m_items.back().count;
}

bool InventorySystem::RemoveItem(const std::string& itemId, int count) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->itemId == itemId) {
            it->count -= count;
            if (it->count <= 0) m_items.erase(it);
            return true;
        }
    }
    return false;
}

int InventorySystem::GetItemCount(const std::string& itemId) const {
    for (const auto& item : m_items)
        if (item.itemId == itemId) return item.count;
    return 0;
}

bool InventorySystem::HasItem(const std::string& itemId, int count) const {
    return GetItemCount(itemId) >= count;
}

bool InventorySystem::UseItem(const std::string& itemId, Player& player) {
    const auto* data = ConfigManager::Instance().GetItem(itemId);
    if (!data || !RemoveItem(itemId, 1)) return false;

    switch (data->effectType) {
        case EffectType::RestoreHp:
            player.SetCurrentHp(player.GetCurrentHp() + data->effectValue);
            break;
        case EffectType::RestoreMp:
            player.SetCurrentMp(player.GetCurrentMp() + data->effectValue);
            break;
        case EffectType::ExpBoost: {
            // 增加修炼经验
            player.GetCultiMutable().cultivationExp += data->effectValue;
            break;
        }
        case EffectType::PermanentHp:
        case EffectType::PermanentMp:
            // 永久加成需要特殊处理，暂时跳过
            break;
        default:
            break;
    }
    return true;
}
