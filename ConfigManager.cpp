#pragma execution_character_set("utf-8")
#include "ConfigManager.h"
#include <fstream>
#include <iostream>

// ============ ConfigManager ============
ConfigManager& ConfigManager::Instance() {
    static ConfigManager inst;
    return inst;
}

bool ConfigManager::LoadAll(const std::string& configDir) {
    json jTech, jNpc, jRealm, jItem;
    bool ok = true;
    ok = ok && LoadFile(configDir + "/techniques.json", jTech);
    ok = ok && LoadFile(configDir + "/npcs.json",      jNpc);
    ok = ok && LoadFile(configDir + "/cultivation.json", jRealm);
    // items.json 可选
    LoadFile(configDir + "/items.json", jItem);
    if (ok) {
        ParseTechniques(jTech);
        ParseNPCs(jNpc);
        if (jRealm.contains("realms")) ParseRealms(jRealm["realms"]);
        if (!jItem.empty() && jItem.contains("items")) ParseItems(jItem["items"]);
    }
    return ok;
}

bool ConfigManager::LoadFile(const std::string& path, json& out) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "[Config] 无法打开: " << path << std::endl;
        return false;
    }
    try {
        ifs >> out;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Config] JSON解析失败: " << path << " -> " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::ParseTechniques(const json& j) {
    m_techniques.clear();
    if (!j.contains("techniques")) return;
    for (const auto& t : j["techniques"]) {
        TechniqueData d;
        d.id            = t.value("id", "");
        d.name          = t.value("name", "");
        d.type          = t.value("type", "");
        d.element       = t.value("element", "无");
        d.cultivationReq= t.value("cultivationReq", "");
        d.maxLevel      = t.value("maxLevel", 1);
        d.description   = t.value("description", "");
        auto e = t.value("effects", json::object());
        d.hpPerLevel    = e.value("hpPerLevel",    0);
        d.mpPerLevel    = e.value("mpPerLevel",    0);
        d.atkPerLevel   = e.value("atkPerLevel",   0);
        d.defPerLevel   = e.value("defPerLevel",   0);
        d.speedPerLevel = e.value("speedPerLevel", 0);
        d.spiritPerLevel= e.value("spiritPerLevel",0);
        // 修炼加成偏向
        auto cb = t.value("cultivationBonus", json::object());
        d.cultBonusPrimary   = cb.value("primary", "hp");
        d.cultBonusSecondary = cb.value("secondary", "mp");
        // 等级解锁技能
        if (t.contains("unlocksSkill")) {
            for (const auto& us : t["unlocksSkill"]) {
                TechniqueData::SkillUnlock su;
                su.level     = us.value("level", 1);
                su.skillName = us.value("skillName", "");
                su.skillDesc = us.value("skillDesc", "");
                d.unlocksSkill.push_back(su);
            }
        }
        m_techniques.push_back(d);
    }
}

void ConfigManager::ParseNPCs(const json& j) {
    m_npcs.clear();
    if (!j.contains("npcs")) return;
    for (const auto& n : j["npcs"]) {
        NPCData d;
        d.id             = n.value("id", "");
        d.name           = n.value("name", "");
        d.title          = n.value("title", "");
        d.location       = n.value("location", "");
        d.cultivation    = n.value("cultivation", "");
        d.description    = n.value("description", "");
        d.defaultRelation= n.value("relation", 0);
        m_npcs.push_back(d);
    }
}

void ConfigManager::ParseRealms(const json& j) {
    m_realms.clear();
    for (const auto& r : j) {
        RealmData d;
        d.id        = r.value("id", "");
        d.name      = r.value("name", "");
        d.tier      = r.value("tier", 0);
        d.subStage  = r.value("subStage", 0);
        d.hpBonus   = r.value("hpBonus", 0);
        d.mpBonus   = r.value("mpBonus", 0);
        d.atkBonus  = r.value("atkBonus", 0);
        d.defBonus  = r.value("defBonus", 0);
        d.speedBonus= r.value("speedBonus", 0);
        d.spiritBonus = r.value("spiritBonus", 0);
        d.lifespan  = r.value("lifespan", 0);
        m_realms.push_back(d);
    }
}

const TechniqueData* ConfigManager::GetTechnique(const std::string& id) const {
    for (const auto& t : m_techniques) if (t.id == id) return &t;
    return nullptr;
}

const NPCData* ConfigManager::GetNPC(const std::string& id) const {
    for (const auto& n : m_npcs) if (n.id == id) return &n;
    return nullptr;
}

const RealmData* ConfigManager::GetRealm(const std::string& id) const {
    for (const auto& r : m_realms) if (r.id == id) return &r;
    return nullptr;
}

void ConfigManager::ParseItems(const json& j) {
    m_items.clear();
    for (const auto& item : j) {
        ItemData d;
        d.id             = item.value("id", "");
        d.name           = item.value("name", "");
        
        // 字符串转 ItemType 枚举
        std::string typeStr = item.value("type", "Misc");
        if (typeStr == "Material")       d.type = ItemType::Material;
        else if (typeStr == "Medicine") d.type = ItemType::Medicine;
        else if (typeStr == "Treasure") d.type = ItemType::Treasure;
        else if (typeStr == "Weapon")   d.type = ItemType::Weapon;
        else if (typeStr == "Armor")    d.type = ItemType::Armor;
        else if (typeStr == "Accessory")d.type = ItemType::Accessory;
        else if (typeStr == "TechniqueBook") d.type = ItemType::TechniqueBook;
        else                            d.type = ItemType::Misc;
        
        d.description    = item.value("description", "");
        d.stackable      = item.value("stackable", 1);
        d.value          = item.value("value", 0);
        
        // 字符串转 EffectType 枚举
        std::string effectStr = item.value("effectType", "None");
        if (effectStr == "RestoreHp")     d.effectType = EffectType::RestoreHp;
        else if (effectStr == "RestoreMp")    d.effectType = EffectType::RestoreMp;
        else if (effectStr == "BuffAtk")      d.effectType = EffectType::BuffAtk;
        else if (effectStr == "BuffDef")      d.effectType = EffectType::BuffDef;
        else if (effectStr == "ExpBoost")     d.effectType = EffectType::ExpBoost;
        else if (effectStr == "BreakthroughAid") d.effectType = EffectType::BreakthroughAid;
        else if (effectStr == "PermanentHp")  d.effectType = EffectType::PermanentHp;
        else if (effectStr == "PermanentMp")  d.effectType = EffectType::PermanentMp;
        else                                  d.effectType = EffectType::None;
        
        d.effectValue    = item.value("effectValue", 0);
        d.cultivationReq = item.value("cultivationReq", "");
        m_items.push_back(d);
    }
}

const ItemData* ConfigManager::GetItem(const std::string& id) const {
    for (const auto& item : m_items) if (item.id == id) return &item;
    return nullptr;
}
