#pragma once
#include <string>
#include <vector>
#include <functional>

// 物品类型
enum class ItemType {
    Material,   // 材料
    Medicine,   // 丹药（消耗品）
    Treasure,   // 宝物/珍稀材料
    Weapon,     // 武器
    Armor,      // 防具
    Accessory,  // 饰品
    TechniqueBook, // 功法秘籍
    Misc        // 杂物
};

// 物品效果类型
enum class EffectType {
    None,
    RestoreHp,     // 恢复生命
    RestoreMp,     // 恢复法力
    BuffAtk,       // 临时攻击加成
    BuffDef,       // 临时防御加成
    ExpBoost,      // 增加修炼经验
    BreakthroughAid, // 辅助突破
    PermanentHp,   // 永久增加生命上限
    PermanentMp    // 永久增加法力上限
};

// 单个物品模板数据
struct ItemData {
    std::string id;
    std::string name;
    ItemType type = ItemType::Misc;
    std::string description;
    int stackable = 1;          // 最大堆叠数量，1=不可堆叠
    int value = 0;              // 售价（灵石）
    EffectType effectType = EffectType::None;
    int effectValue = 0;        // 效果数值
    std::string cultivationReq; // 使用所需境界（空=无限制）
};

// 背包中的一个物品实例
struct InventoryItem {
    std::string itemId;
    int count = 1;

    bool operator==(const InventoryItem& other) const {
        return itemId == other.itemId;
    }
};

// 背包系统
class InventorySystem {
public:
    static const int MAX_SLOTS = 40; // 最大背包格数

    // 添加物品（返回实际添加数量）
    int AddItem(const std::string& itemId, int count = 1);
    bool RemoveItem(const std::string& itemId, int count = 1);
    int GetItemCount(const std::string& itemId) const;
    bool HasItem(const std::string& itemId, int count = 1) const;

    // 使用物品（返回是否成功使用）
    bool UseItem(const std::string& itemId, class Player& player);

    const std::vector<InventoryItem>& GetItems() const { return m_items; }
    int GetSize() const { return static_cast<int>(m_items.size()); }

private:
    std::vector<InventoryItem> m_items;
};
