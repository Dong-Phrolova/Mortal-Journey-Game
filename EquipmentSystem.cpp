#pragma execution_character_set("utf-8")
#include "EquipmentSystem.h"
#include <algorithm>

// ============================================================
//  单例
// ============================================================
EquipmentSystem& EquipmentSystem::Instance() {
    static EquipmentSystem inst;
    return inst;
}

// ============================================================
//  初始化
// ============================================================
void EquipmentSystem::Initialize() {
    if (!m_equipments.empty()) return;  // 已初始化
    DefineBuiltInEquipments();

    // 初始化装备槽为空
    for (int i = 0; i < 3; ++i) {
        m_slots[i].equippedId = "";
    }
}

void EquipmentSystem::DefineBuiltInEquipments() {
    // ===== 武器类 =====

    // 铁木剑 — 新手武器
    {
        EquipmentData eq;
        eq.id = "iron_wood_sword";
        eq.name = "铁木剑";
        eq.description = "凡铁打造的长剑，虽不锋利但足以自保。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Common;
        eq.atkBonus = 5;
        eq.price = 25;
        m_equipments.push_back(eq);
    }

    // 寒铁剑
    {
        EquipmentData eq;
        eq.id = "cold_iron_sword";
        eq.name = "寒铁剑";
        eq.description = "掺入寒铁矿锻造的剑，挥舞时带有寒气。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Uncommon;
        eq.atkBonus = 12;
        eq.spdBonus = 2;
        eq.price = 80;
        m_equipments.push_back(eq);
    }

    // 青铜古剑
    {
        EquipmentData eq;
        eq.id = "bronze_ancient_sword";
        eq.name = "青铜古剑";
        eq.description = "上古遗留的青铜剑，似乎蕴含微弱的灵力。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Rare;
        eq.atkBonus = 22;
        eq.critBonus = true;
        eq.critRate = 0.08f;
        eq.critDmg = 1.5f;
        eq.price = 250;
        m_equipments.push_back(eq);
    }

    // 炎阳刃
    {
        EquipmentData eq;
        eq.id = "flame_blade";
        eq.name = "炎阳刃";
        eq.description = "以地心火炼制的利刃，剑身常年散发着灼热之气。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Epic;
        eq.atkBonus = 35;
        eq.hpBonus = 30;
        eq.vampiric = true;
        eq.vampiricRate = 0.1f;
        eq.price = 600;
        m_equipments.push_back(eq);
    }

    // 七星龙渊剑
    {
        EquipmentData eq;
        eq.id = "dragon_blade";
        eq.name = "七星龙渊剑";
        eq.description = "传说中的神兵，剑身上镶嵌七颗星辰碎片。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Legendary;
        eq.atkBonus = 60;
        eq.spdBonus = 8;
        eq.spiritBonus = 10;
        eq.critBonus = true;
        eq.critRate = 0.15f;
        eq.critDmg = 2.0f;
        eq.vampiric = true;
        eq.vampiricRate = 0.15f;
        eq.price = 2000;
        m_equipments.push_back(eq);
    }

    // 碧影针 — 刺客武器（高速度+暴击）
    {
        EquipmentData eq;
        eq.id = "shadow_needle";
        eq.name = "碧影针";
        eq.description = "细如发丝的暗器，专攻要害。";
        eq.slot = EquipSlot::Weapon;
        eq.rarity = EquipRarity::Rare;
        eq.atkBonus = 18;
        eq.spdBonus = 10;
        eq.critBonus = true;
        eq.critRate = 0.2f;
        eq.critDmg = 1.8f;
        eq.dodgeBonus = true;
        eq.dodgeRate = 0.08f;
        eq.price = 350;
        m_equipments.push_back(eq);
    }

    // ===== 防具类 =====

    // 粗布衣 — 新手防具
    {
        EquipmentData eq;
        eq.id = "cloth_armor";
        eq.name = "粗布衣";
        eq.description = "普通的粗布衣服，聊胜于无。";
        eq.slot = EquipSlot::Armor;
        eq.rarity = EquipRarity::Common;
        eq.defBonus = 3;
        eq.hpBonus = 10;
        eq.price = 15;
        m_equipments.push_back(eq);
    }

    // 皮甲
    {
        EquipmentData eq;
        eq.id = "leather_armor";
        eq.name = "猎户皮甲";
        eq.description = "兽皮缝制的护甲，轻便且有一定防护力。";
        eq.slot = EquipSlot::Armor;
        eq.rarity = EquipRarity::Uncommon;
        eq.defBonus = 8;
        eq.hpBonus = 30;
        eq.spdBonus = 2;
        eq.price = 60;
        m_equipments.push_back(eq);
    }

    // 青缎衣 — 七玄门弟子标准装束（原著描述）
    {
        EquipmentData eq;
        eq.id = "green_gown";
        eq.name = "青缎衣";
        eq.description = "七玄门弟子标准装束，青缎制成，衣袂飘飘。";
        eq.slot = EquipSlot::Armor;
        eq.rarity = EquipRarity::Common;
        eq.defBonus = 5;
        eq.hpBonus = 15;
        eq.spdBonus = 1;
        eq.price = 40;
        m_equipments.push_back(eq);
    }

    // 精钢铠
    {
        EquipmentData eq;
        eq.id = "steel_armor";
        eq.name = "精钢锁子甲";
        eq.description = "精钢丝编织的锁子甲，防护出色但略显笨重。";
        eq.slot = EquipSlot::Armor;
        eq.rarity = EquipRarity::Rare;
        eq.defBonus = 18;
        eq.hpBonus = 60;
        eq.counterAtk = true;
        eq.counterRate = 0.1f;
        eq.price = 280;
        m_equipments.push_back(eq);
    }

    // 玄龟甲
    {
        EquipmentData eq;
        eq.id = "turtle_shell_armor";
        eq.name = "玄龟甲";
        eq.description = "用千年玄龟壳打磨而成，坚不可摧。";
        eq.slot = EquipSlot::Armor;
        eq.rarity = EquipRarity::Epic;
        eq.defBonus = 30;
        eq.hpBonus = 100;
        eq.mpBonus = 40;
        eq.dodgeBonus = true;
        eq.dodgeRate = 0.08f;
        eq.counterAtk = true;
        eq.counterRate = 0.12f;
        eq.price = 700;
        m_equipments.push_back(eq);
    }

    // ===== 饰品类 =====

    // 铁项链
    {
        EquipmentData eq;
        eq.id = "iron_necklace";
        eq.name = "铁项链";
        eq.description = "粗制的铁环项链，没什么特别之处。";
        eq.slot = EquipSlot::Accessory;
        eq.rarity = EquipRarity::Common;
        eq.hpBonus = 15;
        eq.mpBonus = 10;
        eq.price = 20;
        m_equipments.push_back(eq);
    }

    // 灵石戒指
    {
        EquipmentData eq;
        eq.id = "spirit_ring";
        eq.name = "灵石戒指";
        eq.description = "镶嵌低阶灵石的银戒，微微提升法力上限。";
        eq.slot = EquipSlot::Accessory;
        eq.rarity = EquipRarity::Uncommon;
        eq.mpBonus = 25;
        eq.spiritBonus = 5;
        eq.price = 100;
        m_equipments.push_back(eq);
    }

    // 金缕玉带
    {
        EquipmentData eq;
        eq.id = "gold_thread_belt";
        eq.name = "金缕玉带";
        eq.description = "金线编织的玉带，佩之可固本培元。";
        eq.slot = EquipSlot::Accessory;
        eq.rarity = EquipRarity::Rare;
        eq.defBonus = 5;
        eq.hpBonus = 50;
        eq.mpBonus = 30;
        eq.price = 200;
        m_equipments.push_back(eq);
    }

    // 速行靴（特殊饰品）
    {
        EquipmentData eq;
        eq.id = "speed_boots";
        eq.name = "风行靴";
        eq.description = "轻如羽毛的靴子，穿戴者行动如风。";
        eq.slot = EquipSlot::Accessory;
        eq.rarity = EquipRarity::Rare;
        eq.spdBonus = 8;
        eq.dodgeBonus = true;
        eq.dodgeRate = 0.1f;
        eq.price = 300;
        m_equipments.push_back(eq);
    }
}

// ============================================================
//  查询
// ============================================================
const EquipmentData* EquipmentSystem::GetEquipment(const std::string& id) const {
    for (const auto& eq : m_equipments) {
        if (eq.id == id) return &eq;
    }
    return nullptr;
}

const EquipmentSystem::EquipSlotData& EquipmentSystem::GetEquipSlot(EquipSlot slot) const {
    return m_slots[static_cast<int>(slot)];
}

const EquipmentData* EquipmentSystem::GetEquipped(EquipSlot slot) const {
    const auto& sd = m_slots[static_cast<int>(slot)];
    if (sd.equippedId.empty()) return nullptr;
    return GetEquipment(sd.equippedId);
}

// ============================================================
//  装备/卸装
// ============================================================
std::string EquipmentSystem::Equip(const std::string& equipId) {
    const auto* eq = GetEquipment(equipId);
    if (!eq) return "";

    int slotIdx = static_cast<int>(eq->slot);
    std::string oldId = m_slots[slotIdx].equippedId;
    m_slots[slotIdx].equippedId = equipId;
    return oldId;
}

std::string EquipmentSystem::Unequip(EquipSlot slot) {
    int slotIdx = static_cast<int>(slot);
    std::string oldId = m_slots[slotIdx].equippedId;
    m_slots[slotIdx].equippedId = "";
    return oldId;
}

// ============================================================
//  总加成计算
// ============================================================
int EquipmentSystem::TotalAtk() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->atkBonus;
    }
    return total;
}

int EquipmentSystem::TotalDef() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->defBonus;
    }
    return total;
}

int EquipmentSystem::TotalHp() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->hpBonus;
    }
    return total;
}

int EquipmentSystem::TotalMp() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->mpBonus;
    }
    return total;
}

int EquipmentSystem::TotalSpd() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->spdBonus;
    }
    return total;
}

int EquipmentSystem::TotalSpirit() const {
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq) total += eq->spiritBonus;
    }
    return total;
}

// ============================================================
//  特殊效果合并
// ============================================================
bool EquipmentSystem::HasVampiric() const {
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->vampiric) return true;
    }
    return false;
}
float EquipmentSystem::VampiricRate() const {
    float r = 0.f;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->vampiric) r += eq->vampiricRate;
    }
    return r;
}
bool EquipmentSystem::HasCounterAtk() const {
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->counterAtk) return true;
    }
    return false;
}
float EquipmentSystem::CounterRate() const {
    float r = 0.f;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->counterAtk) r += eq->counterRate;
    }
    return r;
}
bool EquipmentSystem::HasDodgeBonus() const {
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->dodgeBonus) return true;
    }
    return false;
}
float EquipmentSystem::DodgeRate() const {
    float r = 0.f;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->dodgeBonus) r += eq->dodgeRate;
    }
    return r;
}
bool EquipmentSystem::HasCritBonus() const {
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->critBonus) return true;
    }
    return false;
}
float EquipmentSystem::CritRate() const {
    float r = 0.f;
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->critBonus) r += eq->critRate;
    }
    return r;
}
float EquipmentSystem::CritDmgMult() const {
    float d = 1.5f;  // 默认暴击伤害
    for (int i = 0; i < 3; ++i) {
        const auto* eq = GetEquipped(static_cast<EquipSlot>(i));
        if (eq && eq->critBonus && eq->critDmg > d) d = eq->critDmg;
    }
    return d;
}
