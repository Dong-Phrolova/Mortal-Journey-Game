#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

// ============================================================
//  装备部位枚举
// ============================================================
enum class EquipSlot {
    Weapon,     // 武器（影响攻击力、特殊技能效果）
    Armor,      // 防具/护甲（影响防御力）
    Accessory   // 饰品（影响速度、特殊属性）
};

// 装备 rarity 品质
enum class EquipRarity {
    Common,     // 普通（白）— 基础属性
    Uncommon,   // 优秀（绿）— 略强于普通
    Rare,       // 稀有（蓝）— 明显提升
    Epic,       // 史诗（紫）— 强力装备
    Legendary   // 传说（金）— 极品
};

// ============================================================
//  单个装备数据
// ============================================================
struct EquipmentData {
    std::string id;                 // 唯一ID，如 "iron_wood_sword"
    std::string name;               // 装备名称
    std::string description;        // 描述文字

    EquipSlot slot = EquipSlot::Weapon;  // 装备部位
    EquipRarity rarity = EquipRarity::Common; // 品质

    // 属性加成（基础值 + 品质倍率）
    int atkBonus = 0;      // 攻击力加成
    int defBonus = 0;      // 防御力加成
    int hpBonus = 0;       // 生命上限加成
    int mpBonus = 0;       // 法力上限加成
    int spdBonus = 0;      // 速度加成
    int spiritBonus = 0;   // 神识加成

    // 特殊效果标记
    bool vampiric = false;      // 吸血：攻击回复生命
    float vampiricRate = 0.f;   // 吸血比例 (0.0~1.0)
    bool counterAtk = false;    // 反击：被攻击时反弹伤害
    float counterRate = 0.f;    // 反弹比例
    bool dodgeBonus = false;    // 闪避加成
    float dodgeRate = 0.f;      // 闪避率加成
    bool critBonus = false;     // 暴击加成
    float critRate = 0.f;       // 暴击率
    float critDmg = 0.f;        // 暴击伤害倍率

    // 价格
    int price = 0;

    // 品质颜色（用于UI渲染）
    sf::Color GetRarityColor() const {
        switch (rarity) {
            case EquipRarity::Common:    return sf::Color(200, 200, 200);
            case EquipRarity::Uncommon:  return sf::Color(100, 200, 100);
            case EquipRarity::Rare:      return sf::Color(100, 140, 255);
            case EquipRarity::Epic:      return sf::Color(180, 100, 255);
            case EquipRarity::Legendary: return sf::Color(255, 200, 60);
            default: return sf::Color(200, 200, 200);
        }
    }

    std::wstring GetSlotName() const {
        switch (slot) {
            case EquipSlot::Weapon:   return L"武器";
            case EquipSlot::Armor:    return L"防具";
            case EquipSlot::Accessory: return L"饰品";
            default: return L"未知";
        }
    }

    std::wstring GetRarityName() const {
        switch (rarity) {
            case EquipRarity::Common:    return L"普通";
            case EquipRarity::Uncommon:  return L"优秀";
            case EquipRarity::Rare:      return L"稀有";
            case EquipRarity::Epic:      return L"史诗";
            case EquipRarity::Legendary: return L"传说";
            default: return L"未知";
        }
    }
};

// ============================================================
//  装备系统管理器（单例）
// ============================================================
class EquipmentSystem {
public:
    static EquipmentSystem& Instance();

    // 初始化内置装备数据
    void Initialize();

    // 获取所有可用装备定义（商店浏览用）
    const std::vector<EquipmentData>& GetAllEquipments() const { m_equipments; }

    // 根据ID获取装备定义
    const EquipmentData* GetEquipment(const std::string& id) const;

    // 当前装备状态（玩家身上穿的）
    struct EquipSlotData {
        std::string equippedId;   // 空字符串表示未装备
    };

    // 获取当前装备
    const EquipSlotData& GetEquipSlot(EquipSlot slot) const;
    const EquipmentData* GetEquipped(EquipSlot slot) const;

    // 穿戴装备（返回替换下来的旧装备ID，空表示该位置为空）
    std::string Equip(const std::string& equipId);

    // 卸下装备
    std::string Unequip(EquipSlot slot);

    // 计算装备总加成
    int TotalAtk() const;
    int TotalDef() const;
    int TotalHp() const;
    int TotalMp() const;
    int TotalSpd() const;
    int TotalSpirit() const;

    // 合并特殊效果
    bool HasVampiric() const;
    float VampiricRate() const;
    bool HasCounterAtk() const;
    float CounterRate() const;
    bool HasDodgeBonus() const;
    float DodgeRate() const;
    bool HasCritBonus() const;
    float CritRate() const;
    float CritDmgMult() const;

private:
    EquipmentSystem() = default;
    void DefineBuiltInEquipments();

    std::vector<EquipmentData> m_equipments;   // 所有装备定义
    EquipSlotData m_slots[3];                  // Weapon/Armor/Accessory
};
