#pragma execution_character_set("utf-8")
#include "GameCallbacks.h"
#include "GameState.h"
#include "CombatState.h"
#include "CultivationUIState.h"
#include "InventoryState.h"
#include "DialogueState.h"
#include "ShopState.h"           // 商店界面
#include "QuestSystem.h"         // 任务系统
#include "WorldMapState.h"
#include "SettingsState.h"
#include "SimpleMapState.h"      // 青牛镇地图（已废弃，保留兼容）
#include "QuestUIState.h"         // 任务界面
#include "StoryIntroState.h"
#include "GameSession.h"

void OnNewGameSelected() {
    // 先显示开场剧情，剧情结束后自动初始化并进入游戏
    GameStateManager::Instance().SwitchState(std::make_unique<StoryIntroState>());
}

void OnContinueGameSelected() {
    // 直接加载最高槽位存档，进入游戏
    for (int slot = 2; slot >= 0; --slot) {
        std::string path = "save_slot" + std::to_string(slot) + ".json";
        if (GameSession::Instance().LoadGame(path)) {
            GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
            return;
        }
    }
    // 没有存档，尝试新游戏
    OnNewGameSelected();
}

void OnOpenSettings() {
    GameStateManager::Instance().PushState(std::make_unique<SettingsState>());
}

void OnOpenCultivationUI() {
    GameStateManager::Instance().PushState(std::make_unique<CultivationUIState>());
}

void OnOpenInventory() {
    GameStateManager::Instance().PushState(std::make_unique<InventoryState>());
}

void OnStartCombat(const std::string& enemyId) {
    if (enemyId == "mortal_thug") {
        auto cs = std::make_unique<CombatState>(L"七玄门恶徒", 60, 10, 15, 5, EnemyType::Mortal, 6, 3);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "street_rogue") {
        auto cs = std::make_unique<CombatState>(L"街头流氓", 45, 5, 12, 3, EnemyType::Mortal, 8, 5);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "qi_disciple") {
        auto cs = std::make_unique<CombatState>(L"黄枫谷弟子", 120, 40, 28, 15, EnemyType::Qi, 12, 8);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "spirit_beast") {
        auto cs = std::make_unique<CombatState>(L"低阶灵兽", 90, 20, 22, 12, EnemyType::YaoBeast, 18, 12);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "blood_spider") {
        auto cs = std::make_unique<CombatState>(L"血色蜘蛛", 100, 25, 26, 10, EnemyType::YaoBeast, 20, 15);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "yao_beast_qi") {
        auto cs = std::make_unique<CombatState>(L"妖兽·练气期", 150, 35, 32, 18, EnemyType::YaoBeast, 30, 20);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "mo_dafu_boss") {
        // 墨大夫Boss战 — 高难度特殊Boss
        auto cs = std::make_unique<CombatState>(L"墨大夫·夺舍", 250, 80, 35, 20, EnemyType::MoXiu, 100, 250);
        cs->SetBossFlag(true);  // 标记为Boss战
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "yelang_thug") {
        auto cs = std::make_unique<CombatState>(L"野狼帮匪徒", 70, 15, 18, 8, EnemyType::Mortal, 5, 2);
        GameStateManager::Instance().PushState(std::move(cs));
    } else if (enemyId == "yelang_captain") {
        auto cs = std::make_unique<CombatState>(L"野狼帮头目", 120, 30, 25, 14, EnemyType::Mortal, 10, 6);
        GameStateManager::Instance().PushState(std::move(cs));
    } else {
        auto cs = std::make_unique<CombatState>(L"未知敌人", 80, 20, 18, 8, EnemyType::Mortal, 10, 5);
        GameStateManager::Instance().PushState(std::move(cs));
    }
}

void OnLearnTechnique(const std::string& techId) {
    auto& player = GameSession::Instance().GetPlayer();
    player.LearnTechnique(techId);
    // 通知任务系统
    QuestSystem::Instance().UpdateProgress(QuestTargetType::LearnTechnique, techId);
}

void OnGiveItem(const std::string& itemId, int count) {
    auto& inv = GameSession::Instance().GetInventory();
    inv.AddItem(itemId, count);
}

void OnTalkToNPC(const std::string& npcId) {
    GameStateManager::Instance().PushState(std::make_unique<DialogueState>(npcId));
    // 通知任务系统：与 NPC 对话
    QuestSystem::Instance().UpdateProgress(QuestTargetType::TalkToNPC, npcId);
}

void OnOpenShop(const std::wstring& shopName, const std::string& shopType) {
    GameStateManager::Instance().PushState(std::make_unique<ShopState>(shopName, shopType));
}

void OnGoToQingniuTown() {
    // 使用新的WorldMapState统一青牛镇
    auto& gs = GameSession::Instance();
    gs.m_currentMapId = "qingniu_town";
    gs.m_playerX = 0;
    gs.m_playerY = 12;
    GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
}

void OnOpenQuestUI() {
    GameStateManager::Instance().PushState(std::make_unique<QuestUIState>());
}

void OnReachLocation(const std::string& mapId, int tileX, int tileY) {
    QuestSystem::Instance().CheckLocation(mapId, tileX, tileY);
}

void OnLevelUp(const std::string& levelStr) {
    QuestSystem::Instance().CheckLevel(levelStr);
}

void OnGetItem(const std::string& itemId, int count) {
    auto& inv = GameSession::Instance().GetInventory();
    int totalCount = inv.GetItemCount(itemId);
    QuestSystem::Instance().CheckItemCollection(itemId, totalCount);
}
