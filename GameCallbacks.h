#pragma once
#include <string>

// 全局回调：由 main.cpp 和各 State 文件共享
// 这些函数在 GameCallbacks.cpp 中实现

// 新游戏选择后 → 进入世界地图（不加载存档）
void OnNewGameSelected();

// 继续游戏 → 读取存档并进入世界地图
void OnContinueGameSelected();

// 打开设置界面
void OnOpenSettings();

// 打开修炼界面（从地图按 C）
void OnOpenCultivationUI();

// 打开背包界面（从地图按 I）
void OnOpenInventory();

// 开始战斗（传入敌人模板 ID）
void OnStartCombat(const std::string& enemyId);

// 对话中学习功法
void OnLearnTechnique(const std::string& techId);

// 对话中获得物品
void OnGiveItem(const std::string& itemId, int count);

// NPC 交互（打开对话）
void OnTalkToNPC(const std::string& npcId);

// 打开商店界面
void OnOpenShop(const std::wstring& shopName, const std::string& shopType);

// 传送到青牛镇（SimpleMapState）
void OnGoToQingniuTown();

// 打开任务界面
void OnOpenQuestUI();

// 到达新位置时调用（通知任务系统）
void OnReachLocation(const std::string& mapId, int tileX, int tileY);

// 玩家升级时调用（通知任务系统）
void OnLevelUp(const std::string& levelStr);

// 获得物品时调用（通知任务系统）
void OnGetItem(const std::string& itemId, int count);
