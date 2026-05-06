#pragma once
#include "Player.h"
#include "ItemSystem.h"
#include <string>
#include <set>

// 游戏全局会话（跨状态共享的数据）
class GameSession {
public:
    static GameSession& Instance();

    Player& GetPlayer() { return m_player; }
    InventorySystem& GetInventory() { return m_inventory; }

    // 初始化新游戏（不加载存档）
    void NewGame();

    // 存档/读档（save/load.json 保存在 exe 同目录）
    bool SaveGame(const std::string& path = "save.json") const;
    bool LoadGame(const std::string& path = "save.json");
    bool HasSaveFile(const std::string& path = "save.json") const;

    // 宝箱状态（跨地图持久化）
    std::set<std::string>& GetOpenedChests() { return m_openedChests; }
    const std::set<std::string>& GetOpenedChests() const { return m_openedChests; }

    // 当前位置（用于读档后恢复）
    std::string m_currentMapId = "qingniu_town";
    int m_playerX = 12;
    int m_playerY = 6;

private:
    GameSession();
    Player m_player;
    InventorySystem m_inventory;
    std::set<std::string> m_openedChests;  // 已开启宝箱集合 "mapId@x,y"
};
