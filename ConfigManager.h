#pragma once
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "ItemSystem.h"

using json = nlohmann::json;

// 功法/技能数据（从 JSON 加载）
struct TechniqueData {
    std::string id;
    std::string name;
    std::string type;       // "功法" / "剑技" / "身法" / "法术"
    std::string element;     // "金" "木" "水" "火" "土" "无"
    std::string cultivationReq; // 所需境界 id
    int maxLevel;
    std::string description;
    // 每级加成
    int hpPerLevel    = 0;
    int mpPerLevel    = 0;
    int atkPerLevel   = 0;
    int defPerLevel   = 0;
    int speedPerLevel = 0;
    int spiritPerLevel= 0;
    // 修炼加成偏向（修炼时额外奖励的属性）
    std::string cultBonusPrimary = "hp";     // 主属性: hp/mp/atk/def/spd/spirit
    std::string cultBonusSecondary = "mp";   // 副属性
    // 等级解锁的战斗技能
    struct SkillUnlock {
        int level = 0;
        std::string skillName;
        std::string skillDesc;
    };
    std::vector<SkillUnlock> unlocksSkill;
};

// NPC 数据
struct NPCData {
    std::string id;
    std::string name;
    std::string title;
    std::string location;
    std::string cultivation;
    std::string description;
    int defaultRelation = 0;
};

// 修炼境界数据（从 JSON 同步）
struct RealmData {
    std::string id;
    std::string name;
    int tier;
    int subStage;
    int hpBonus;
    int mpBonus;
    int atkBonus;
    int defBonus;
    int speedBonus;
    int spiritBonus;
    int lifespan;
};

// ============ 配置管理器（单例）============
class ConfigManager {
public:
    static ConfigManager& Instance();

    bool LoadAll(const std::string& configDir);
    bool Reload(const std::string& configDir);

    // 查询
    const TechniqueData* GetTechnique(const std::string& id) const;
    const std::vector<TechniqueData>& GetAllTechniques() const { return m_techniques; }

    const NPCData* GetNPC(const std::string& id) const;
    const std::vector<NPCData>& GetAllNPCs() const { return m_npcs; }

    const RealmData* GetRealm(const std::string& id) const;
    const std::vector<RealmData>& GetAllRealms() const { return m_realms; }

    const ItemData* GetItem(const std::string& id) const;
    const std::vector<ItemData>& GetAllItems() const { return m_items; }

private:
    ConfigManager() = default;
    bool LoadFile(const std::string& path, json& out);
    void ParseTechniques(const json& j);
    void ParseNPCs(const json& j);
    void ParseRealms(const json& j);
    void ParseItems(const json& j);

    std::vector<TechniqueData> m_techniques;
    std::vector<NPCData>       m_npcs;
    std::vector<RealmData>     m_realms;
    std::vector<ItemData>      m_items;
};
