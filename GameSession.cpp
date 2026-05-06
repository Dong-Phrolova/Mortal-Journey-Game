#pragma execution_character_set("utf-8")
#include "GameSession.h"
#include "CultivationSystem.h"
#include "QuestSystem.h"
#include "EquipmentSystem.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

GameSession::GameSession() : 
    m_player(L"韩立", 100, 50, 25, 10) {
}

GameSession& GameSession::Instance() {
    static GameSession inst;
    return inst;
}

void GameSession::NewGame() {
    // 重置玩家属性
    m_player = Player(L"韩立", 100, 50, 25, 10);

    PlayerCulti culti;
    culti.major = MajorRealm::Qi;
    culti.qiLayer = 1;
    culti.subStage = SubStage::None;
    culti.spiritRoot = SpiritRoot::ThreeElement;
    culti.cultivationExp = 0;
    culti.breakthroughReq = 100;
    m_player.SetCulti(culti);
    m_player.SetCurrentHp(m_player.GetMaxHp());
    m_player.SetCurrentMp(m_player.GetMaxMp());

    // 重置背包
    m_inventory = InventorySystem();
    m_inventory.AddItem("hp_potion_small", 3);
    m_inventory.AddItem("mp_potion_small", 2);

    // 重置任务系统（强制重新初始化，清除之前状态）
    QuestSystem::Instance().Reset();
    QuestSystem::Instance().Initialize();

    // 初始化装备系统 + 给新手装备
    EquipmentSystem::Instance().Initialize();
    EquipmentSystem::Instance().Equip("iron_wood_sword");   // 初始武器：铁木剑
    EquipmentSystem::Instance().Equip("cloth_armor");       // 初始防具：粗布衣

    // 重置宝箱和位置
    m_openedChests.clear();
    m_currentMapId = "qingniu_town";
    m_playerX = 12;
    m_playerY = 6;
}

// ============ 存档 ============
bool GameSession::SaveGame(const std::string& path) const {
    try {
        const auto& culti = m_player.GetCulti();
        json j;

        // 玩家基础信息
        j["player"]["hp"]    = m_player.GetCurrentHp();
        j["player"]["mp"]    = m_player.GetCurrentMp();
        j["player"]["gold"]  = m_player.GetGold();

        // 修炼信息
        j["culti"]["major"]           = static_cast<int>(culti.major);
        j["culti"]["qiLayer"]         = culti.qiLayer;
        j["culti"]["subStage"]        = static_cast<int>(culti.subStage);
        j["culti"]["spiritRoot"]      = static_cast<int>(culti.spiritRoot);
        j["culti"]["cultivationExp"]  = culti.cultivationExp;
        j["culti"]["breakthroughReq"] = culti.breakthroughReq;

        // 已学功法
        json techs = json::array();
        for (const auto& lt : m_player.GetLearned()) {
            techs.push_back({{"id", lt.techniqueId}, {"level", lt.level}});
        }
        j["techniques"] = techs;

        // 功法点数
        json tpObj = json::object();
        for (const auto& [realm, pts] : m_player.GetAllTechPoints()) {
            tpObj[realm] = pts;
        }
        j["techPoints"] = tpObj;

        // 背包
        json inv = json::array();
        for (const auto& item : m_inventory.GetItems()) {
            inv.push_back({{"id", item.itemId}, {"count", item.count}});
        }
        j["inventory"] = inv;

        // 任务进度
        {
            json questsArray = json::array();
            for (const auto& q : QuestSystem::Instance().GetAllQuests()) {
                json qj;
                qj["id"] = q.id;
                qj["status"] = static_cast<int>(q.status);
                json objsArray = json::array();
                for (const auto& obj : q.objectives) {
                    json oj;
                    oj["currentCount"] = obj.currentCount;
                    oj["completed"] = obj.completed;
                    objsArray.push_back(oj);
                }
                qj["objectives"] = objsArray;
                questsArray.push_back(qj);
            }
            j["quests"] = questsArray;
        }

        // 装备状态（按槽位保存装备ID）
        json equipObj = json::object();
        const auto& weaponSlot = EquipmentSystem::Instance().GetEquipSlot(EquipSlot::Weapon);
        const auto& armorSlot = EquipmentSystem::Instance().GetEquipSlot(EquipSlot::Armor);
        const auto& accSlot = EquipmentSystem::Instance().GetEquipSlot(EquipSlot::Accessory);
        equipObj["weapon"] = weaponSlot.equippedId;
        equipObj["armor"] = armorSlot.equippedId;
        equipObj["accessory"] = accSlot.equippedId;
        j["equipment"] = equipObj;

        // 宝箱状态
        json chestsArray = json::array();
        for (const auto& key : m_openedChests) {
            chestsArray.push_back(key);
        }
        j["openedChests"] = chestsArray;

        // 当前位置
        j["currentMap"] = m_currentMapId;
        j["playerX"] = m_playerX;
        j["playerY"] = m_playerY;

        std::ofstream ofs(path);
        if (!ofs.is_open()) return false;
        ofs << j.dump(2);
        return true;
    } catch (...) {
        return false;
    }
}

// ============ 读档 ============
bool GameSession::LoadGame(const std::string& path) {
    try {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return false;

        json j;
        ifs >> j;

        // 重建玩家（用默认值重新构造）
        m_player = Player(L"韩立", 100, 50, 25, 10);

        // 修炼信息（先设置，这样后面 SetCurrentHp/Mp 能正确算上限）
        PlayerCulti culti;
        culti.major          = static_cast<MajorRealm>(j["culti"]["major"].get<int>());
        culti.qiLayer        = j["culti"]["qiLayer"].get<int>();
        culti.subStage       = static_cast<SubStage>(j["culti"]["subStage"].get<int>());
        culti.spiritRoot     = static_cast<SpiritRoot>(j["culti"]["spiritRoot"].get<int>());
        culti.cultivationExp = j["culti"]["cultivationExp"].get<int>();
        culti.breakthroughReq= j["culti"]["breakthroughReq"].get<int>();
        m_player.SetCulti(culti);

        // 当前 HP/MP/Gold
        m_player.SetCurrentHp(j["player"]["hp"].get<int>());
        m_player.SetCurrentMp(j["player"]["mp"].get<int>());
        // gold 通过 AddGold 设置（需要先清零）
        // 因为 Player 没有 SetGold，用 SpendGold 扣到0再加
        {
            int cur = m_player.GetGold();
            if (cur > 0) m_player.SpendGold(cur);
            m_player.AddGold(j["player"]["gold"].get<int>());
        }

        // 功法点数（必须在加载功法之前，因为升级功法会消耗点数）
        if (j.contains("techPoints")) {
            std::map<std::string, int> tp;
            for (auto& [key, val] : j["techPoints"].items()) {
                tp[key] = val.get<int>();
            }
            m_player.SetTechPoints(tp);
        }

        // 已学功法
        if (j.contains("techniques")) {
            for (const auto& t : j["techniques"]) {
                std::string id = t["id"].get<std::string>();
                int lv = t["level"].get<int>();
                m_player.LearnTechnique(id);
                // 如果等级>1，继续升级
                for (int i = 1; i < lv; ++i)
                    m_player.UpgradeTechnique(id);
            }
        }

        // 背包
        m_inventory = InventorySystem();
        if (j.contains("inventory")) {
            for (const auto& item : j["inventory"]) {
                m_inventory.AddItem(item["id"].get<std::string>(), item["count"].get<int>());
            }
        }

        // 任务进度恢复
        QuestSystem::Instance().Initialize();  // 先初始化任务定义
        if (j.contains("quests")) {
            for (const auto& qj : j["quests"]) {
                auto* q = QuestSystem::Instance().GetQuest(qj["id"].get<std::string>());
                if (!q) continue;
                q->status = static_cast<QuestStatus>(qj["status"].get<int>());
                if (qj.contains("objectives")) {
                    const auto& objs = qj["objectives"];
                    for (size_t i = 0; i < objs.size() && i < q->objectives.size(); ++i) {
                        q->objectives[i].currentCount = objs[i]["currentCount"].get<int>();
                        q->objectives[i].completed = objs[i]["completed"].get<bool>();
                    }
                }
            }
        }
        // 恢复后自动接取满足前置条件的后续任务
        QuestSystem::Instance().ResumeAfterLoad();

        // 装备恢复
        EquipmentSystem::Instance().Initialize();
        if (j.contains("equipment")) {
            const auto& eq = j["equipment"];
            if (eq.contains("weapon") && !eq["weapon"].get<std::string>().empty())
                EquipmentSystem::Instance().Equip(eq["weapon"].get<std::string>());
            if (eq.contains("armor") && !eq["armor"].get<std::string>().empty())
                EquipmentSystem::Instance().Equip(eq["armor"].get<std::string>());
            if (eq.contains("accessory") && !eq["accessory"].get<std::string>().empty())
                EquipmentSystem::Instance().Equip(eq["accessory"].get<std::string>());
        }

        // 宝箱状态恢复
        m_openedChests.clear();
        if (j.contains("openedChests")) {
            for (const auto& key : j["openedChests"]) {
                m_openedChests.insert(key.get<std::string>());
            }
        }

        // 当前位置恢复
        if (j.contains("currentMap")) m_currentMapId = j["currentMap"].get<std::string>();
        if (j.contains("playerX")) m_playerX = j["playerX"].get<int>();
        if (j.contains("playerY")) m_playerY = j["playerY"].get<int>();

        return true;
    } catch (...) {
        return false;
    }
}

bool GameSession::HasSaveFile(const std::string& path) const {
    std::ifstream ifs(path);
    return ifs.good();
}
