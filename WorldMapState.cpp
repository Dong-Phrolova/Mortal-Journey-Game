#pragma execution_character_set("utf-8")
#include "WorldMapState.h"
#include "GameState.h"
#include "GameCallbacks.h"
#include "Player.h"
#include "GameSession.h"
#include "ConfigManager.h"
#include "SettingsState.h"
#include "MainMenuState.h"
#include "CultivationSystem.h"
#include "QuestSystem.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#define NOMINMAX
#include <Windows.h>

// ============================================================
//  UTF-8 to Wide 转换
// ============================================================
static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// ============================================================
//  WorldMapState 构造
// ============================================================
WorldMapState::WorldMapState()
    : m_camera(25, 20, 800, 600) {
    m_type = GameStateType::WorldMap;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    InitAllMaps();

    // 从存档读取位置（如果有）
    auto& gs = GameSession::Instance();
    SwitchToMap(gs.m_currentMapId, gs.m_playerX, gs.m_playerY);

    m_floatText.setFont(m_font);
    m_floatText.setFont(m_font);
    m_floatText.setCharacterSize(16);
}

// ============================================================
//  初始化所有地图定义
// ============================================================
void WorldMapState::InitAllMaps() {
    BuildQixuanmen();
    BuildJiazhou();
    BuildHuangfengu();
    BuildXuese();
    BuildQixuanmenBack();
    BuildJiazhouMarket();
    BuildLuanshenghai();
    BuildCaixiaMountain();  // 彩霞山（七玄门主峰）
    BuildLianguCliff();     // 炼骨崖（采药场景）
    BuildShenshuValley();   // 神手谷（墨大夫隐居地）
    BuildQingniuTown();     // 青牛镇
}

// ============================================================
//  七玄门·外门 地图构建 (25x20 格)
// ============================================================
void WorldMapState::BuildQixuanmen() {
    MapData map;
    map.id = "qixuanmen";
    map.displayNameW = L"七玄门·外门";
    map.width = 25;
    map.height = 20;
    map.hasRandomEncounter = false;
    map.encounterRate = 0;
    map.defaultSpawnX = 12;
    map.defaultSpawnY = 18;

    // 初始化所有格子为草地
    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // === 布局设计 ===
    // 外围：上下左右四面围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;       // 上墙
        map.tiles[map.height-1][x].type = TileType::Wall; // 下墙
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;        // 左墙
        map.tiles[y][map.width-1].type = TileType::Wall; // 右墙
    }

    // 南门（下方中央）→ 青牛镇
    map.tiles[map.height-1][12].type = TileType::Door;
    map.tiles[map.height-1][12].warpTargetMap = "qingniu_town";
    map.tiles[map.height-1][12].warpTargetX = 12;
    map.tiles[map.height-1][12].warpTargetY = 1;  // 落点在北门内侧Path格

    // 东门（右侧中央） → 通往黄枫谷
    map.tiles[10][map.width-1].type = TileType::Door;
    map.tiles[10][map.width-1].warpTargetMap = "huangfengu";
    map.tiles[10][map.width-1].warpTargetX = 2;
    map.tiles[10][map.width-1].warpTargetY = 10;  // 落点在门前一格

    // 西门（左侧中央）→ 通往神手谷
    map.tiles[10][0].type = TileType::Door;
    map.tiles[10][0].warpTargetMap = "shenshu_valley";
    map.tiles[10][0].warpTargetX = 8;
    map.tiles[10][0].warpTargetY = 18;

    // === 内部建筑 ===
    // 演武场（中央）— 用 Fence 围起来
    for (int y = 6; y <= 10; ++y)
        for (int x = 6; x <= 18; ++x) {
            map.tiles[y][x].type = TileType::Grass; // 练武场是草地
            map.tiles[y][x].encounterRate = 5; // 低概率遇敌
        }
    // 演武场围栏（四周）
    for (int x = 6; x <= 18; ++x) {
        map.tiles[6][x].type = TileType::Fence;
        map.tiles[10][x].type = TileType::Fence;
    }
    for (int y = 6; y <= 10; ++y) {
        map.tiles[y][6].type = TileType::Fence;
        map.tiles[y][18].type = TileType::Fence;
    }
    // 演武场入口（南边）
    map.tiles[10][12].type = TileType::Path;
    map.tiles[11][12].type = TileType::Path;

    // 药房（左上角：3x3 房间）
    for (int y = 2; y <= 4; ++y)
        for (int x = 2; x <= 4; ++x)
            map.tiles[y][x].type = TileType::Floor;
    // 药房外墙
    for (int x = 2; x <= 4; ++x) {
        map.tiles[2][x].type = TileType::Wall;
        map.tiles[4][x].type = TileType::Wall;
    }
    for (int y = 2; y <= 4; ++y) {
        map.tiles[y][2].type = TileType::Wall;
        map.tiles[y][4].type = TileType::Wall;
    }
    // 药房门（改为普通门洞）
    map.tiles[3][4].type = TileType::Floor;
    // 药房内部一格
    map.tiles[3][5].type = TileType::Floor;
    map.tiles[3][5].npcId = 1; // 药房伙计

    // 住处（右上角：3x3 房间）— 改为普通房间
    for (int y = 2; y <= 4; ++y)
        for (int x = 20; x <= 22; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 20; x <= 22; ++x) {
        map.tiles[2][x].type = TileType::Wall;
        map.tiles[4][x].type = TileType::Wall;
    }
    // 住处门（改为普通门口）
    map.tiles[3][20].type = TileType::Path;
    map.tiles[3][21].type = TileType::Path;

    // 秘籍商人（演武场西侧小屋）
    for (int y = 2; y <= 4; ++y)
        for (int x = 14; x <= 16; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 14; x <= 16; ++x) {
        map.tiles[2][x].type = TileType::Wall;
        map.tiles[4][x].type = TileType::Wall;
    }
    for (int y = 2; y <= 4; ++y) {
        map.tiles[y][14].type = TileType::Wall;
        map.tiles[y][16].type = TileType::Wall;
    }
    map.tiles[3][16].type = TileType::Door;  // 门朝东
    map.tiles[3][15].npcId = 12;  // 秘籍商人 inside

    // 通神手谷入口（左下角小院 — 原传送阵，改为普通院子）
    for (int y = 14; y <= 16; ++y)
        for (int x = 2; x <= 4; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 2; x <= 4; ++x) {
        map.tiles[14][x].type = TileType::Wall;
        map.tiles[16][x].type = TileType::Wall;
    }
    for (int y = 14; y <= 16; ++y) {
        map.tiles[y][2].type = TileType::Wall;
        map.tiles[y][4].type = TileType::Wall;
    }
    map.tiles[15][4].type = TileType::Floor;
    // 提示文字用石头装饰代替
    map.tiles[15][3].type = TileType::Floor;

    // 王护法（南门入口附近）
    map.tiles[18][12].type = TileType::Floor;
    map.tiles[18][12].npcId = 14; // 王护法

    // 岳堂主（演武场东侧）
    map.tiles[8][20].type = TileType::Floor;
    map.tiles[8][20].npcId = 15; // 岳堂主

    // 院子里的树装饰
    map.tiles[15][6].type = TileType::Tree;
    map.tiles[13][8].type = TileType::Tree;
    map.tiles[17][10].type = TileType::Tree;

    // 北门（上方中央）→ 通往七玄门后山
    map.tiles[0][12].type = TileType::Door;
    map.tiles[0][12].warpTargetMap = "qixuanmen_back";
    map.tiles[0][12].warpTargetX = 10;
    map.tiles[0][12].warpTargetY = 14;

    // 宝箱 x2
    map.tiles[8][3].type = TileType::Chest;
    map.tiles[8][3].chestLootItemId = "hp_potion_small";
    map.tiles[8][3].chestLootCount = 3;
    map.tiles[8][22].type = TileType::Chest;
    map.tiles[8][22].chestLootItemId = "mp_potion_small";
    map.tiles[8][22].chestLootCount = 2;

    // 路径：从南门到演武场
    for (int y = 17; y >= 11; --y) {
        map.tiles[y][12].type = TileType::Path;
    }
    // 横向路径
    for (int x = 4; x <= 20; ++x) {
        map.tiles[12][x].type = TileType::Path;
    }
    // 到药房的路
    for (int y = 11; y >= 5; --y)
        map.tiles[y][3].type = TileType::Path;
    // 到处住的路
    for (int y = 11; y >= 5; --y)
        map.tiles[y][21].type = TileType::Path;
    // 到神手谷传送的路
    for (int x = 3; x <= 4; ++x)
        for (int y = 15; y <= 17; ++y)
            if (!(x == 4 && y == 15)) // 保留门的位置
                map.tiles[y][x].type = TileType::Path;

    // 树木装饰
    map.tiles[6][10].type = TileType::Tree;
    map.tiles[6][14].type = TileType::Tree;
    map.tiles[11][6].type = TileType::Tree;
    map.tiles[11][18].type = TileType::Tree;
    map.tiles[13][20].type = TileType::Tree;
    map.tiles[11][5].type = TileType::Tree;
    map.tiles[8][19].type = TileType::Tree;
    map.tiles[17][11].type = TileType::Tree;
    map.tiles[19][11].type = TileType::Tree;
    map.tiles[19][6].type = TileType::Tree;
    map.tiles[5][11].type = TileType::Tree;

    // 石头装饰
    map.tiles[5][6].type = TileType::Ground;
    map.tiles[11][10].type = TileType::Ground;
    map.tiles[18][11].type = TileType::Ground;

    // 草地上的随机高草（小范围）
    map.tiles[8][8].type = TileType::Grass; map.tiles[8][8].encounterRate = 10;
    map.tiles[8][9].type = TileType::Grass; map.tiles[8][9].encounterRate = 10;
    map.tiles[9][8].type = TileType::Grass; map.tiles[9][8].encounterRate = 10;
    map.tiles[9][9].type = TileType::Grass; map.tiles[9][9].encounterRate = 10;

    // 添加 NPC 到地图
    // ID 1: 药房伙计（在药房内）
    // ID 2: 墨大夫（在居所内）
    // ID 3: 厉飞雨（在演武场右上角）
    map.tiles[8][17].npcId = 3;
    // ID 4: 张铁（在演武场左下角）
    map.tiles[8][7].npcId = 4;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  嘉州城 地图构建
// ============================================================
void WorldMapState::BuildJiazhou() {
    MapData map;
    map.id = "jiazhou";
    map.displayNameW = L"嘉州城";
    map.width = 30;
    map.height = 22;
    map.hasRandomEncounter = true;
    map.encounterRate = 8;
    map.enemyPool = {"mortal_thug"};
    map.defaultSpawnX = 12;
    map.defaultSpawnY = 1;

    // 全部初始化为路径（城内街道）
    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Path, 0, -1}));

    // 城墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 北门1 -> 通往青牛镇（从青牛镇东门进入）
    map.tiles[0][10].type = TileType::Door;
    map.tiles[0][10].warpTargetMap = "qingniu_town";
    map.tiles[0][10].warpTargetX = 28;
    map.tiles[0][10].warpTargetY = 12;

    // 北门2 -> 通往嘉州城市场
    map.tiles[0][16].type = TileType::Door;
    map.tiles[0][16].warpTargetMap = "jiazhou_market";
    map.tiles[0][16].warpTargetX = 11;
    map.tiles[0][16].warpTargetY = 16;

    // 南门 -> 通往黄枫谷（从南面沿山路南下）
    map.tiles[map.height-1][15].type = TileType::Door;
    map.tiles[map.height-1][15].warpTargetMap = "huangfengu";
    map.tiles[map.height-1][15].warpTargetX = 1;
    map.tiles[map.height-1][15].warpTargetY = 15;  // 落点在门前一格

    // 城内建筑和树木（装饰）
    // 药铺（左上）
    for (int y = 3; y <= 6; ++y)
        for (int x = 3; x <= 7; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 3; x <= 7; ++x) {
        map.tiles[3][x].type = TileType::Wall;
        map.tiles[6][x].type = TileType::Wall;
    }
    for (int y = 3; y <= 6; ++y) {
        map.tiles[y][3].type = TileType::Wall;
        map.tiles[y][7].type = TileType::Wall;
    }
    map.tiles[4][7].type = TileType::Door;

    // 酒楼（右上）
    for (int y = 3; y <= 6; ++y)
        for (int x = 22; x <= 27; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 22; x <= 27; ++x) {
        map.tiles[3][x].type = TileType::Wall;
        map.tiles[6][x].type = TileType::Wall;
    }
    for (int y = 3; y <= 6; ++y) {
        map.tiles[y][22].type = TileType::Wall;
        map.tiles[y][27].type = TileType::Wall;
    }
    map.tiles[4][22].type = TileType::Door;

    // 坊市（中央大片区域）
    for (int y = 8; y <= 14; ++y)
        for (int x = 8; x <= 21; ++x)
            map.tiles[y][x].type = TileType::Grass; // 坊市是露天场地

    // 几个宝箱
    map.tiles[5][15].type = TileType::Chest;
    map.tiles[5][15].chestLootItemId = "spirit_gathering_pill";
    map.tiles[5][15].chestLootCount = 2;
    map.tiles[16][15].type = TileType::Chest;
    map.tiles[16][15].chestLootItemId = "hp_potion_small";
    map.tiles[16][15].chestLootCount = 5;

    // 树木装饰
    map.tiles[2][10].type = TileType::Tree;
    map.tiles[2][20].type = TileType::Tree;
    map.tiles[19][10].type = TileType::Tree;
    map.tiles[19][20].type = TileType::Tree;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  黄枫谷·山门 地图构建
// ============================================================
void WorldMapState::BuildHuangfengu() {
    MapData map;
    map.id = "huangfengu";
    map.displayNameW = L"黄枫谷·山门";
    map.width = 20;
    map.height = 18;
    map.hasRandomEncounter = true;
    map.encounterRate = 20;
    map.enemyPool = {"qi_disciple", "spirit_beast"};
    map.minRealmLevel = 3; // 筑基期
    map.defaultSpawnX = 1;
    map.defaultSpawnY = 10;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 西门 -> 回七玄门（对应七玄门东门）
    map.tiles[10][0].type = TileType::Door;
    map.tiles[10][0].warpTargetMap = "qixuanmen";
    map.tiles[10][0].warpTargetX = 23;
    map.tiles[10][0].warpTargetY = 10;

    // 从七玄门东门到达的落地格→就是回传门
    map.tiles[10][1].type = TileType::Door;
    map.tiles[10][1].warpTargetMap = "qixuanmen";
    map.tiles[10][1].warpTargetX = 23;
    map.tiles[10][1].warpTargetY = 10;  // 落点在门前一格

    // 南门 -> 回嘉州城（从谷口向北上山）
    map.tiles[16][1].type = TileType::Door;
    map.tiles[16][1].warpTargetMap = "jiazhou";
    map.tiles[16][1].warpTargetX = 15;
    map.tiles[16][1].warpTargetY = 20;  // 落点在门前一格

    // 山门（大殿）
    for (int y = 6; y <= 12; ++y)
        for (int x = 14; x <= 18; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 14; x <= 18; ++x) {
        map.tiles[6][x].type = TileType::Wall;
        map.tiles[12][x].type = TileType::Wall;
    }
    for (int y = 6; y <= 12; ++y) {
        map.tiles[y][14].type = TileType::Wall;
        map.tiles[y][18].type = TileType::Wall;
    }
    map.tiles[9][14].type = TileType::Door; // 山门入口
    // 万小山在殿内
    map.tiles[9][16].type = TileType::Floor;
    map.tiles[9][16].npcId = 5; // 万小山 ID=5

    // 入谷的小径
    for (int y = 10; y <= 16; ++y)
        map.tiles[y][1].type = TileType::Path;
    for (int x = 1; x <= 14; ++x)
        map.tiles[16][x].type = TileType::Path;

    // 林木装饰
    map.tiles[3][3].type = TileType::Tree;
    map.tiles[4][8].type = TileType::Tree;
    map.tiles[3][15].type = TileType::Tree;
    map.tiles[14][4].type = TileType::Tree;
    map.tiles[15][8].type = TileType::Tree;
    map.tiles[14][15].type = TileType::Tree;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  血色禁地·入口 地图构建
// ============================================================
void WorldMapState::BuildXuese() {
    MapData map;
    map.id = "xuese";
    map.displayNameW = L"血色禁地·入口";
    map.width = 18;
    map.height = 15;
    map.hasRandomEncounter = true;
    map.encounterRate = 40;
    map.enemyPool = {"blood_spider", "yao_beast_qi"};
    map.minRealmLevel = 3; // 筑基期
    map.defaultSpawnX = 16; // 靠近东门，方便回去
    map.defaultSpawnY = 10;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Grass, 0, -1}));

    // 围墙（血色禁地外层）
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 东门 -> 回七玄门
    map.tiles[10][map.width-1].type = TileType::Door;
    map.tiles[10][map.width-1].warpTargetMap = "qixuanmen";
    map.tiles[10][map.width-1].warpTargetX = 1;
    map.tiles[10][map.width-1].warpTargetY = 10;

    // 从七玄门西门到达的落地格→就是回传门
    map.tiles[10][16].type = TileType::Door;
    map.tiles[10][16].warpTargetMap = "qixuanmen";
    map.tiles[10][16].warpTargetX = 0;
    map.tiles[10][16].warpTargetY = 10;

    // 禁地内部道路（曲折小径）
    // 主路：入口 -> 禁地深处
    for (int y = 1; y <= 8; ++y)
        map.tiles[y][9].type = TileType::Path;
    for (int x = 9; x <= 15; ++x)
        map.tiles[8][x].type = TileType::Path;
    // 禁地深处（高草）
    for (int y = 9; y < map.height-1; ++y)
        for (int x = 10; x < map.width-1; ++x)
            map.tiles[y][x].type = TileType::Grass;

    // 危险标识树
    map.tiles[5][3].type = TileType::Tree;
    map.tiles[5][4].type = TileType::Tree;
    map.tiles[6][3].type = TileType::Tree;
    map.tiles[6][4].type = TileType::Tree;

    // 宝箱（危险但奖励丰厚）
    map.tiles[11][12].type = TileType::Chest;
    map.tiles[11][12].chestLootItemId = "hp_potion_small";
    map.tiles[11][12].chestLootCount = 5;
    map.tiles[12][13].type = TileType::Chest;
    map.tiles[12][13].chestLootItemId = "spirit_gathering_pill";
    map.tiles[12][13].chestLootCount = 3;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  切换地图
// ============================================================
void WorldMapState::SwitchToMap(const std::string& mapId, int spawnX, int spawnY) {
    // 重置交互状态（防止切换地图后卡在选项/对话界面）
    m_interactState = InteractState::None;
    m_nearbyNPCId = -1;
    m_selectedOption = 0;
    m_dialogueLines.clear();
    m_dialogueLineIndex = 0;

    auto it = m_maps.find(mapId);
    if (it == m_maps.end()) return;

    MapData* newMap = &it->second;

    // 境界检查
    auto* player = &GameSession::Instance().GetPlayer();
    if (newMap->minRealmLevel > 0) {
        int curLevel = static_cast<int>(player->GetMajorRealm());
        if (curLevel < newMap->minRealmLevel) {
            // 境界不足，拒绝进入（暂用简单提示）
            return;
        }
    }

    m_currentMap = newMap;
    m_tileMap.LoadMap(newMap);

    // 设置玩家出生点
    if (spawnX >= 0) {
        m_playerTileX = spawnX;
        m_playerTileY = spawnY;
    } else {
        m_playerTileX = newMap->defaultSpawnX;
        m_playerTileY = newMap->defaultSpawnY;
    }
    m_player.SetTilePosition(m_playerTileX, m_playerTileY);

    // 通知任务系统：玩家到达了新地图
    OnReachLocation(m_currentMap->id, m_playerTileX, m_playerTileY);

    // 保存当前位置到存档系统
    GameSession::Instance().m_currentMapId = m_currentMap->id;
    GameSession::Instance().m_playerX = m_playerTileX;
    GameSession::Instance().m_playerY = m_playerTileY;

    // 恢复已开启的宝箱状态（防止退出重开后重复开启，格式 "mapId@x,y"）
    for (const auto& key : GameSession::Instance().GetOpenedChests()) {
        // key = "mapId@x,y"
        size_t atPos = key.find('@');
        if (atPos == std::string::npos) continue;
        std::string kMapId = key.substr(0, atPos);
        if (kMapId != mapId) continue;
        size_t commaPos = key.find(',', atPos + 1);
        if (commaPos == std::string::npos) continue;
        int cx = atoi(key.substr(atPos + 1, commaPos - atPos - 1).c_str());
        int cy = atoi(key.substr(commaPos + 1).c_str());
        if (cx >= 0 && cy >= 0 && cx < newMap->width && cy < newMap->height) {
            newMap->tiles[cy][cx].chestOpened = true;
            newMap->tiles[cy][cx].variant = 2; // 已开渲染
        }
    }

    // 设置摄像机
    m_camera.SetMapSize(newMap->width, newMap->height);
    m_camera.SetTarget(m_playerTileX * TileSet::TILE_SIZE,
                       m_playerTileY * TileSet::TILE_SIZE);

    // 加载 NPC
    m_tileMap.ClearNPCs();
    // 根据当前地图添加对应 NPC
    // 七玄门 NPC
    if (mapId == "qixuanmen") {
        MapNPC npc1; npc1.id = 1; npc1.name = "药房伙计";
        npc1.nameW = L"药房伙计"; npc1.color = sf::Color(100, 150, 200);
        npc1.mapX = 3; npc1.mapY = 5; npc1.faceDir = 3; npc1.isMoving = false;
        m_tileMap.AddNPC(npc1);

        // 王护法 — 南门处守护
        MapNPC npc14; npc14.id = 14; npc14.name = "王护法";
        npc14.nameW = L"王护法"; npc14.color = sf::Color(150, 80, 60);
        npc14.mapX = 12; npc14.mapY = 18; npc14.faceDir = 0; npc14.isMoving = false;
        m_tileMap.AddNPC(npc14);

        // 岳堂主 — 演武场东侧
        MapNPC npc15; npc15.id = 15; npc15.name = "岳堂主";
        npc15.nameW = L"岳堂主"; npc15.color = sf::Color(180, 120, 60);
        npc15.mapX = 20; npc15.mapY = 8; npc15.faceDir = 3; npc15.isMoving = false;
        m_tileMap.AddNPC(npc15);

        MapNPC npc3; npc3.id = 3; npc3.name = "厉飞雨";
        npc3.nameW = L"厉飞雨"; npc3.color = sf::Color(80, 120, 180);
        npc3.mapX = 17; npc3.mapY = 8; npc3.faceDir = 2; npc3.isMoving = false;
        m_tileMap.AddNPC(npc3);

        // 张铁 — 任务7未激活时在七玄门外门；激活后移到神手谷
        auto* quest7Check = QuestSystem::Instance().GetQuest("quest_007_mo_scheme");
        if (!quest7Check || quest7Check->status == QuestStatus::Locked) {
            MapNPC npc4; npc4.id = 4; npc4.name = "张铁";
            npc4.nameW = L"张铁"; npc4.color = sf::Color(180, 140, 100);
            npc4.mapX = 7; npc4.mapY = 8; npc4.faceDir = 0; npc4.isMoving = false;
            m_tileMap.AddNPC(npc4);
        }

        MapNPC npc12; npc12.id = 12; npc12.name = "秘籍商人";
        npc12.nameW = L"秘籍商人"; npc12.color = sf::Color(140, 100, 160);
        npc12.mapX = 15; npc12.mapY = 3; npc12.faceDir = 3; npc12.isMoving = false;
        m_tileMap.AddNPC(npc12);
    }
    // 黄枫谷 NPC
    else if (mapId == "huangfengu") {
        MapNPC npc5; npc5.id = 5; npc5.name = "万小山";
        npc5.nameW = L"万小山"; npc5.color = sf::Color(60, 140, 90);
        npc5.mapX = 16; npc5.mapY = 9; npc5.faceDir = 2; npc5.isMoving = false;
        m_tileMap.AddNPC(npc5);
    }
    // 七玄门后山 NPC
    else if (mapId == "qixuanmen_back") {
        MapNPC npc6; npc6.id = 6; npc6.name = "山民";
        npc6.nameW = L"山民"; npc6.color = sf::Color(100, 120, 80);
        npc6.mapX = 3; npc6.mapY = 12; npc6.faceDir = 1; npc6.isMoving = false;
        m_tileMap.AddNPC(npc6);
    }
    // 嘉州城市场 NPC
    else if (mapId == "jiazhou_market") {
        MapNPC npc7; npc7.id = 7; npc7.name = "武器商";
        npc7.nameW = L"武器商"; npc7.color = sf::Color(180, 160, 100);
        npc7.mapX = 7; npc7.mapY = 4; npc7.faceDir = 2; npc7.isMoving = false;
        m_tileMap.AddNPC(npc7);

        MapNPC npc8; npc8.id = 8; npc8.name = "丹药师";
        npc8.nameW = L"丹药师"; npc8.color = sf::Color(100, 180, 140);
        npc8.mapX = 14; npc8.mapY = 4; npc8.faceDir = 2; npc8.isMoving = false;
        m_tileMap.AddNPC(npc8);

        MapNPC npc9; npc9.id = 9; npc9.name = "杂货商";
        npc9.nameW = L"杂货商"; npc9.color = sf::Color(160, 140, 100);
        npc9.mapX = 14; npc9.mapY = 11; npc9.faceDir = 3; npc9.isMoving = false;
        m_tileMap.AddNPC(npc9);

        MapNPC npc10; npc10.id = 10; npc10.name = "说书人";
        npc10.nameW = L"说书人"; npc10.color = sf::Color(140, 120, 170);
        npc10.mapX = 11; npc10.mapY = 8; npc10.faceDir = 0; npc10.isMoving = false;
        m_tileMap.AddNPC(npc10);

        // 车夫（传送到青牛镇）
        MapNPC npc13; npc13.id = 13; npc13.name = "车夫";
        npc13.nameW = L"车夫"; npc13.color = sf::Color(160, 140, 80);
        npc13.mapX = 5; npc13.mapY = 15; npc13.faceDir = 1; npc13.isMoving = false;
        m_tileMap.AddNPC(npc13);
    }
    // 乱星海 NPC
    else if (mapId == "luanshenghai") {
        MapNPC npc11; npc11.id = 11; npc11.name = "星海守护者";
        npc11.nameW = L"星海守护者"; npc11.color = sf::Color(80, 80, 160);
        npc11.mapX = 15; npc11.mapY = 12; npc11.faceDir = 1; npc11.isMoving = false;
        m_tileMap.AddNPC(npc11);
    }
    // 彩霞山 NPC
    else if (mapId == "caixia_mountain") {
        MapNPC npc100; npc100.id = 100; npc100.name = "守门弟子";
        npc100.nameW = L"守门弟子"; npc100.color = sf::Color(120, 120, 180);
        npc100.mapX = 14; npc100.mapY = 15; npc100.faceDir = 0; npc100.isMoving = false;
        m_tileMap.AddNPC(npc100);
    }
    // 炼骨崖 NPC
    else if (mapId == "liangu_cliff") {
        // 墨大夫 — 任务9"墨大夫的真相"激活后才出现在炼骨崖
        auto* quest9 = QuestSystem::Instance().GetQuest("quest_009_mo_truth");
        if (quest9 && (quest9->status == QuestStatus::Active ||
                       quest9->status == QuestStatus::Completed ||
                       quest9->status == QuestStatus::Rewarded)) {
            MapNPC npc200; npc200.id = 200; npc200.name = "墨大夫";
            npc200.nameW = L"墨大夫"; npc200.color = sf::Color(60, 60, 60);
            npc200.mapX = 12; npc200.mapY = 12; npc200.faceDir = 3; npc200.isMoving = false;
            m_tileMap.AddNPC(npc200);
        }
    }
    // 神手谷 NPC
    else if (mapId == "shenshu_valley") {
        // 墨大夫 — 任务9未激活时在神手谷；激活后转到炼骨崖
        auto* quest9 = QuestSystem::Instance().GetQuest("quest_009_mo_truth");
        if (!quest9 || quest9->status == QuestStatus::Locked) {
            MapNPC npc200s; npc200s.id = 200; npc200s.name = "墨大夫";
            npc200s.nameW = L"墨大夫"; npc200s.color = sf::Color(60, 60, 60);
            npc200s.mapX = 8; npc200s.mapY = 13; npc200s.faceDir = 3; npc200s.isMoving = false;
            m_tileMap.AddNPC(npc200s);
        }

        // 张铁 — 任务7激活后从七玄门移到神手谷
        auto* quest7 = QuestSystem::Instance().GetQuest("quest_007_mo_scheme");
        if (quest7 && (quest7->status == QuestStatus::Active ||
                       quest7->status == QuestStatus::Completed ||
                       quest7->status == QuestStatus::Rewarded)) {
            MapNPC npc202; npc202.id = 202; npc202.name = "张铁";
            npc202.nameW = L"张铁"; npc202.color = sf::Color(160, 120, 70);
            npc202.mapX = 10; npc202.mapY = 14; npc202.faceDir = 0; npc202.isMoving = false;
            m_tileMap.AddNPC(npc202);
        }
    }
    // 青牛镇 NPC
    else if (mapId == "qingniu_town") {
        MapNPC npc300; npc300.id = 300; npc300.name = "铁匠";
        npc300.nameW = L"铁匠"; npc300.color = sf::Color(140, 100, 50);
        npc300.mapX = 5; npc300.mapY = 5; npc300.faceDir = 0; npc300.isMoving = false;
        m_tileMap.AddNPC(npc300);

        MapNPC npc301; npc301.id = 301; npc301.name = "三叔";
        npc301.nameW = L"三叔"; npc301.color = sf::Color(60, 110, 60);
        npc301.mapX = 16; npc301.mapY = 5; npc301.faceDir = 3; npc301.isMoving = false;
        m_tileMap.AddNPC(npc301);

        MapNPC npc302; npc302.id = 302; npc302.name = "秘籍商人";
        npc302.nameW = L"秘籍商人"; npc302.color = sf::Color(140, 100, 160);
        npc302.mapX = 15; npc302.mapY = 12; npc302.faceDir = 3; npc302.isMoving = false;
        m_tileMap.AddNPC(npc302);

        MapNPC npc303; npc303.id = 303; npc303.name = "装备商";
        npc303.nameW = L"装备商"; npc303.color = sf::Color(150, 100, 50);
        npc303.mapX = 25; npc303.mapY = 18; npc303.faceDir = 2; npc303.isMoving = false;
        m_tileMap.AddNPC(npc303);

        MapNPC npc304; npc304.id = 304; npc304.name = "杂货商";
        npc304.nameW = L"杂货商"; npc304.color = sf::Color(160, 140, 100);
        npc304.mapX = 24; npc304.mapY = 17; npc304.faceDir = 0; npc304.isMoving = false;
        m_tileMap.AddNPC(npc304);
    }

    // 显示地图名称提示
    m_showMapName = true;
    m_mapNameTimer = 3.f; // 显示3秒

    // 重置遇敌冷却
    m_encounterCooldown = 2.f;

    // 在地图上生成敌人（如果有遇敌配置）
    m_enemySystem.ClearEnemies();
    if (newMap->hasRandomEncounter && !newMap->enemyPool.empty()) {
        m_enemySystem.SpawnEnemies(newMap->enemyPool,
                                   newMap->width, newMap->height);
    }
}

// ============================================================
//  Enter / Update / Render
// ============================================================
void WorldMapState::Enter() {
    m_encounterCooldown = 2.f;

    // 首次进入显示任务提示
    static bool s_firstEnter = true;
    if (s_firstEnter) {
        s_firstEnter = false;
        ShowFloatingText(L"按 Q 查看当前任务 | WASD/方向键移动 | 按 E/NPC附近按F交互");
    }
}

void WorldMapState::Update(float dt) {
    // 更新玩家动画
    m_player.Update(dt);

    // 更新摄像机跟随
    if (!m_player.IsMoving()) {
        m_camera.Update(dt,
                        m_player.GetPixelPos().x,
                        m_player.GetPixelPos().y);
    } else {
        // 玩家移动时，摄像机直接跟随（不用平滑）
        m_camera.SetTarget(m_player.GetPixelPos().x, m_player.GetPixelPos().y);
    }

    // 地图名称提示倒计时
    if (m_showMapName) {
        m_mapNameTimer -= dt;
        if (m_mapNameTimer <= 0.f) m_showMapName = false;
    }

    // 任务完成通知
    auto& questSys = QuestSystem::Instance();
    if (!questSys.m_lastCompletedQuestName.empty()) {
        std::wstring wName = Utf8ToWide(questSys.m_lastCompletedQuestName);
        ShowFloatingText(L"✔ 任务完成: " + wName + L" （按 Q 领取奖励）");
        questSys.m_lastCompletedQuestName.clear();
    }

    // 遇敌冷却
    if (m_encounterCooldown > 0)
        m_encounterCooldown -= dt;

    // 移动冷却
    if (m_moveCooldown > 0)
        m_moveCooldown -= dt;

    // 更新 NPC
    m_tileMap.UpdateNPCs(dt);

    // 更新地图敌人 AI（传入玩家像素位置）
    float px = m_player.GetPixelPos().x + TileSet::TILE_SIZE * 0.4f;
    float py = m_player.GetPixelPos().y + TileSet::TILE_SIZE * 0.35f;
    m_enemySystem.Update(dt, px, py);

    // 检测敌人与玩家碰撞 → 进入战斗
    if (m_encounterCooldown <= 0.f) {
        MapEnemy* hitEnemy = m_enemySystem.CheckPlayerCollision(px, py, 20.f);
        if (hitEnemy) {
            m_enemySystem.MarkInCombat(hitEnemy->id);
            OnStartCombat(hitEnemy->enemyId);
            m_encounterCooldown = 3.f;
        }
    }

    // 浮动提示倒计时
    if (m_floatTimer > 0.f)
        m_floatTimer -= dt;
}

// ============================================================
//  七玄门·后山 地图构建
// ============================================================
void WorldMapState::BuildQixuanmenBack() {
    MapData map;
    map.id = "qixuanmen_back";
    map.displayNameW = L"七玄门·后山";
    map.width = 20;
    map.height = 16;
    map.hasRandomEncounter = true;
    map.encounterRate = 15;
    map.enemyPool = {"mortal_thug", "qi_disciple", "yelang_thug"};  // 加入野狼帮
    map.defaultSpawnX = 10;
    map.defaultSpawnY = 14;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 南门 -> 回七玄门（传送门下移一格到南墙开口处，下方改为Door）

    // 内部山路
    for (int x = 8; x <= 12; ++x)
        map.tiles[10][x].type = TileType::Path;
    for (int y = 10; y <= 14; ++y)
        map.tiles[y][10].type = TileType::Path;

    // 从七玄门北门到达的落地格→就是回传门（南墙开口(15,10)，比原来下移一格）
    map.tiles[14][10].type = TileType::Path;  // 恢复为路径（原Door移至南墙）
    map.tiles[map.height-1][10].type = TileType::Door;
    map.tiles[map.height-1][10].warpTargetMap = "qixuanmen";
    map.tiles[map.height-1][10].warpTargetX = 12;
    map.tiles[map.height-1][10].warpTargetY = 1;  // 落点在门前一格

    // 树木装饰
    map.tiles[6][3].type = TileType::Tree;
    map.tiles[4][8].type = TileType::Tree;
    map.tiles[7][15].type = TileType::Tree;
    map.tiles[12][5].type = TileType::Tree;
    map.tiles[14][12].type = TileType::Tree;

    // 高草练级区
    for (int y = 5; y <= 12; ++y)
        for (int x = 12; x <= 17; ++x)
            map.tiles[y][x].type = TileType::Grass;
    map.tiles[8][14].encounterRate = 20;
    map.tiles[9][15].encounterRate = 20;
    map.tiles[10][14].encounterRate = 20;

    // 宝箱
    map.tiles[3][10].type = TileType::Chest;
    map.tiles[3][10].chestLootItemId = "hp_potion_small";
    map.tiles[3][10].chestLootCount = 3;

    // 神秘小瓶宝箱（原著：韩立在树林中踢到硬物，发现神秘小瓶）
    map.tiles[7][14].type = TileType::Chest;
    map.tiles[7][14].chestLootItemId = "mystic_bottle";
    map.tiles[7][14].chestLootCount = 1;

    // NPC：山民
    map.tiles[12][3].npcId = 6;

    // 北侧小路 → 通往彩霞山（七玄门主峰）
    // 横向连通：从主路(y=10, x=10~12)向东延伸到 x=16
    for (int x = 10; x <= 16; ++x)
        map.tiles[10][x].type = TileType::Path;
    // 纵向：从 y=10 向北到 y=6（与北段纵向路汇合）
    for (int y = 6; y <= 10; ++y)
        map.tiles[y][16].type = TileType::Path;
    // 北段：继续向北直到北门(y=0)，确保 y=1 也连通
    for (int y = 1; y <= 6; ++y)
        map.tiles[y][16].type = TileType::Path;


    // 从炼骨崖西门到达的落地格→就是回传门
    map.tiles[1][16].type = TileType::Door;
    map.tiles[1][16].warpTargetMap = "liangu_cliff";
    map.tiles[1][16].warpTargetX = 0;
    map.tiles[1][16].warpTargetY = 12;

    // 北门（通往彩霞山）— 在北侧围墙上开一个 Door
    map.tiles[0][16].type = TileType::Door;
    map.tiles[0][16].warpTargetMap = "caixia_mountain";
    map.tiles[0][16].warpTargetX = 14;
    map.tiles[0][16].warpTargetY = 22;

    // 西侧路（从彩霞山西门返回时的着陆点）
    for (int y = 1; y <= 3; ++y)
        map.tiles[y][16].type = TileType::Path;  // 北门向南延伸

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  嘉州城·市场 地图构建（安全区，无敌人）
// ============================================================
void WorldMapState::BuildJiazhouMarket() {
    MapData map;
    map.id = "jiazhou_market";
    map.displayNameW = L"嘉州城·市场";
    map.width = 22;
    map.height = 18;
    map.hasRandomEncounter = false;
    map.encounterRate = 0;
    map.enemyPool = {};
    map.defaultSpawnX = 11;
    map.defaultSpawnY = 16;

    // 初始化为路径（市场街道）
    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Path, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 南门 -> 回嘉州城（对应嘉州城北门2(0,16)，着陆在门内侧(16,1)）
    map.tiles[map.height-1][11].type = TileType::Door;
    map.tiles[map.height-1][11].warpTargetMap = "jiazhou";
    map.tiles[map.height-1][11].warpTargetX = 16;
    map.tiles[map.height-1][11].warpTargetY = 1;

    // 武器铺（左上）
    for (int y = 3; y <= 5; ++y)
        for (int x = 3; x <= 6; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 3; x <= 6; ++x) {
        map.tiles[3][x].type = TileType::Wall;
        map.tiles[5][x].type = TileType::Wall;
    }
    for (int y = 3; y <= 5; ++y) {
        map.tiles[y][3].type = TileType::Wall;
        map.tiles[y][6].type = TileType::Wall;
    }
    map.tiles[4][6].type = TileType::Door;
    map.tiles[4][7].npcId = 7;  // 武器商在店外

    // 丹药铺（右上）
    for (int y = 3; y <= 5; ++y)
        for (int x = 15; x <= 18; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 15; x <= 18; ++x) {
        map.tiles[3][x].type = TileType::Wall;
        map.tiles[5][x].type = TileType::Wall;
    }
    for (int y = 3; y <= 5; ++y) {
        map.tiles[y][15].type = TileType::Wall;
        map.tiles[y][18].type = TileType::Wall;
    }
    map.tiles[4][15].type = TileType::Door;
    map.tiles[4][14].npcId = 8;  // 丹药师在店外

    // 杂货铺（中央偏下）
    for (int y = 10; y <= 12; ++y)
        for (int x = 8; x <= 13; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 8; x <= 13; ++x) {
        map.tiles[10][x].type = TileType::Wall;
        map.tiles[12][x].type = TileType::Wall;
    }
    for (int y = 10; y <= 12; ++y) {
        map.tiles[y][8].type = TileType::Wall;
        map.tiles[y][13].type = TileType::Wall;
    }
    map.tiles[11][13].type = TileType::Door;
    map.tiles[11][14].npcId = 9;  // 杂货商在店外

    // 说书人（市场中央）
    map.tiles[8][11].npcId = 10;

    // 树木装饰
    map.tiles[2][2].type = TileType::Tree;
    map.tiles[2][19].type = TileType::Tree;
    map.tiles[15][2].type = TileType::Tree;
    map.tiles[15][19].type = TileType::Tree;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  乱星海 地图构建（后期高等级地图，需高境界进入）
// ============================================================
void WorldMapState::BuildLuanshenghai() {
    MapData map;
    map.id = "luanshenghai";
    map.displayNameW = L"乱星海";
    map.width = 30;
    map.height = 24;
    map.hasRandomEncounter = true;
    map.encounterRate = 50;
    map.enemyPool = {"yao_beast_qi", "blood_spider"};
    map.minRealmLevel = 6;
    map.defaultSpawnX = 15;
    map.defaultSpawnY = 22;

    // 以草地为主
    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Grass, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 南门 -> 暂时回七玄门（高境界玩家可用）
    map.tiles[map.height-1][15].type = TileType::Door;
    map.tiles[map.height-1][15].warpTargetMap = "qixuanmen";
    map.tiles[map.height-1][15].warpTargetX = 12;
    map.tiles[map.height-1][15].warpTargetY = 1;

    // 中央陆地（Ground）
    for (int y = 8; y <= 16; ++y)
        for (int x = 10; x <= 20; ++x)
            map.tiles[y][x].type = TileType::Ground;

    // 桥梁（Path）
    for (int y = 17; y <= 22; ++y)
        map.tiles[y][15].type = TileType::Path;

    // 高草（危险）
    for (int y = 9; y <= 15; ++y)
        for (int x = 11; x <= 19; ++x)
            map.tiles[y][x].type = TileType::Grass;
    map.tiles[10][15].encounterRate = 30;
    map.tiles[12][18].encounterRate = 30;

    // 宝箱
    map.tiles[10][15].type = TileType::Chest;
    map.tiles[10][15].chestLootItemId = "spirit_gathering_pill";
    map.tiles[10][15].chestLootCount = 5;
    map.tiles[12][18].type = TileType::Chest;
    map.tiles[12][18].chestLootItemId = "mp_potion_small";
    map.tiles[12][18].chestLootCount = 5;

    // 高等级 NPC
    map.tiles[12][15].npcId = 11;

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  彩霞山（七玄门主峰）地图构建
//  小说：韩立通过七玄门入门测试后进入彩霞山修炼
// ============================================================
void WorldMapState::BuildCaixiaMountain() {
    MapData map;
    map.id = "caixia_mountain";
    map.displayNameW = L"彩霞山·七玄门";
    map.width = 28;
    map.height = 24;
    map.hasRandomEncounter = true;
    map.encounterRate = 10;
    map.enemyPool = {"qi_disciple", "mortal_thug", "yelang_captain"};
    map.defaultSpawnX = 14;
    map.defaultSpawnY = 22;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 南门 -> 通往七玄门后山（传送到后山北门内侧下一格）
    map.tiles[map.height-1][14].type = TileType::Door;
    map.tiles[map.height-1][14].warpTargetMap = "qixuanmen_back";
    map.tiles[map.height-1][14].warpTargetX = 16;
    map.tiles[map.height-1][14].warpTargetY = 2;  // 落点在门前一格


    // （已删除）北门 -> 炼骨崖（原传送门改为墙壁）
    map.tiles[0][14].type = TileType::Wall;

    // （已删除）西门 -> 回七玄门后山（原传送门和专有路径已移除）
    // 西门墙格恢复为墙壁
    map.tiles[12][0].type = TileType::Wall;

    // === 主山道（南北贯通）===
    for (int y = 2; y < map.height - 2; ++y)
        map.tiles[y][14].type = TileType::Path;

    // 山门广场（南入口）
    for (int y = 18; y <= 22; ++y)
        for (int x = 11; x <= 17; ++x)
            if (map.tiles[y][x].type != TileType::Wall)
                map.tiles[y][x].type = TileType::Path;

    // === 演武殿（中央偏南）===
    for (int y = 13; y <= 17; ++y)
        for (int x = 9; x <= 19; ++x)
            map.tiles[y][x].type = TileType::Floor;
    // 演武殿外墙
    for (int x = 9; x <= 19; ++x) {
        map.tiles[13][x].type = TileType::Wall;
        map.tiles[17][x].type = TileType::Wall;
    }
    for (int y = 13; y <= 17; ++y) {
        map.tiles[y][9].type = TileType::Wall;
        map.tiles[y][19].type = TileType::Wall;
    }
    // 演武殿大门（南侧）
    map.tiles[17][14].type = TileType::Door;

    // === 藏经阁（东北侧）===
    for (int y = 4; y <= 8; ++y)
        for (int x = 21; x <= 25; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 21; x <= 25; ++x) {
        map.tiles[4][x].type = TileType::Wall;
        map.tiles[8][x].type = TileType::Wall;
    }
    for (int y = 4; y <= 8; ++y) {
        map.tiles[y][21].type = TileType::Wall;
        map.tiles[y][25].type = TileType::Wall;
    }
    map.tiles[8][23].type = TileType::Door;
    // 连接路径
    map.tiles[9][23].type = TileType::Path;
    for (int y = 9; y <= 12; ++y) map.tiles[y][14].type = TileType::Path;

    // === 弟子居所（西北侧）===
    for (int y = 4; y <= 8; ++y)
        for (int x = 3; x <= 7; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 3; x <= 7; ++x) {
        map.tiles[4][x].type = TileType::Wall;
        map.tiles[8][x].type = TileType::Wall;
    }
    for (int y = 4; y <= 8; ++y) {
        map.tiles[y][3].type = TileType::Wall;
        map.tiles[y][7].type = TileType::Wall;
    }
    map.tiles[8][5].type = TileType::Door;
    map.tiles[9][5].type = TileType::Path;
    for (int y = 9; y <= 12; ++y) map.tiles[y][5].type = TileType::Path;
    for (int x = 5; x <= 14; ++x) map.tiles[12][x].type = TileType::Path;

    // === 灵药园（东侧）===
    for (int y = 4; y <= 10; ++y)
        for (int x = 15; x <= 19; ++x)
            map.tiles[y][x].type = TileType::Grass;
    for (int x = 15; x <= 19; ++x) {
        map.tiles[4][x].type = TileType::Fence;
        map.tiles[10][x].type = TileType::Fence;
    }
    for (int y = 4; y <= 10; ++y) {
        map.tiles[y][15].type = TileType::Fence;
        map.tiles[y][19].type = TileType::Fence;
    }
    // 灵药园门
    map.tiles[10][17].type = TileType::Door;
    map.tiles[11][17].type = TileType::Path;
    for (int y = 11; y <= 12; ++y) map.tiles[y][17].type = TileType::Path;

    // === 树木装饰 ===
    map.tiles[3][16].type = TileType::Tree;
    map.tiles[6][18].type = TileType::Tree;
    map.tiles[11][2].type = TileType::Tree;
    map.tiles[15][22].type = TileType::Tree;
    map.tiles[20][10].type = TileType::Tree;
    map.tiles[20][18].type = TileType::Tree;

    // === 宝箱 ===
    map.tiles[6][11].type = TileType::Chest;   // 演武殿附近
    map.tiles[6][11].chestLootItemId = "spirit_gathering_pill";
    map.tiles[6][11].chestLootCount = 3;

    // === NPC：七玄门弟子 ===
    map.tiles[15][14].npcId = 100;   // 守门弟子

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  炼骨崖（采药场景）地图构建
//  小说第5-6章：墨大夫带韩立来此采药炼丹
// ============================================================
void WorldMapState::BuildLianguCliff() {
    MapData map;
    map.id = "liangu_cliff";
    map.displayNameW = L"炼骨崖";
    map.width = 16;
    map.height = 22;
    map.hasRandomEncounter = false;
    map.encounterRate = 0;
    map.enemyPool = {};
    map.defaultSpawnX = 7;
    map.defaultSpawnY = 20;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // 围墙（悬崖边）
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;       // 北边是峭壁
        map.tiles[map.height-1][x].type = TileType::Wall; // 南边出口
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;        // 西壁
        map.tiles[y][map.width-1].type = TileType::Wall; // 东壁
    }

    // === 峭壁区域（北部）===
    for (int y = 2; y <= 6; ++y)
        for (int x = 3; x <= 12; ++x)
            map.tiles[y][x].type = TileType::Wall;  // 峭壁不可通行
    // 峭壁边缘（可站立的窄路）
    map.tiles[7][4].type = TileType::Path;
    map.tiles[7][5].type = TileType::Path;
    map.tiles[7][6].type = TileType::Path;
    map.tiles[7][7].type = TileType::Path;
    map.tiles[7][8].type = TileType::Path;
    map.tiles[7][9].type = TileType::Path;
    map.tiles[7][10].type = TileType::Path;
    map.tiles[7][11].type = TileType::Path;

    // === 主通道（南北向）===
    for (int y = 8; y < map.height - 1; ++y)
        map.tiles[y][7].type = TileType::Path;

    // === 药圃区域（西侧，墨大夫的药地）===
    for (int y = 9; y <= 16; ++y)
        for (int x = 2; x <= 5; ++x)
            map.tiles[y][x].type = TileType::Grass;  // 种满灵草的田
    map.tiles[9][2].type = TileType::Fence;
    map.tiles[9][5].type = TileType::Fence;
    for (int y = 9; y <= 16; ++y) {
        map.tiles[y][1].type = TileType::Fence;
        map.tiles[y][6].type = TileType::Fence;
    }
    map.tiles[13][6].type = TileType::Door;  // 药圃门
    map.tiles[13][7].type = TileType::Path;

    // === 炼丹房（东南角）===
    for (int y = 10; y <= 14; ++y)
        for (int x = 10; x <= 14; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 10; x <= 14; ++x) {
        map.tiles[10][x].type = TileType::Wall;
        map.tiles[14][x].type = TileType::Wall;
    }
    for (int y = 10; y <= 14; ++y) {
        map.tiles[y][10].type = TileType::Wall;
        map.tiles[y][14].type = TileType::Wall;
    }
    map.tiles[14][12].type = TileType::Door;
    map.tiles[14][7].type = TileType::Path;

    // === 墨大夫住处（西南角）===
    for (int y = 17; y <= 20; ++y)
        for (int x = 2; x <= 5; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 2; x <= 5; ++x) {
        map.tiles[17][x].type = TileType::Wall;
        map.tiles[20][x].type = TileType::Wall;
    }
    for (int y = 17; y <= 20; ++y) {
        map.tiles[y][2].type = TileType::Wall;
        map.tiles[y][5].type = TileType::Wall;
    }
    map.tiles[20][4].type = TileType::Door;

    // === 小溪（东侧）===
    for (int y = 8; y <= 18; ++y)
        map.tiles[y][14].type = TileType::Water;

    // === 石桥跨溪 ===
    map.tiles[12][13].type = TileType::Path;
    map.tiles[12][14].type = TileType::Path;
    // 到炼丹房的路径
    for (int y = 12; y <= 14; ++y) map.tiles[y][12].type = TileType::Path;

    // === 装饰树木 ===
    map.tiles[8][2].type = TileType::Tree;
    map.tiles[18][10].type = TileType::Tree;
    map.tiles[16][13].type = TileType::Tree;

    // === NPC：墨大夫（在炼丹房内）===
    map.tiles[12][12].npcId = 200;  // 墨大夫

    // 从神手谷北门到达的落地格→就是回传门
    map.tiles[20][7].type = TileType::Door;
    map.tiles[20][7].warpTargetMap = "shenshu_valley";
    map.tiles[20][7].warpTargetX = 8;
    map.tiles[20][7].warpTargetY = 1;  // 落点在门前一格

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  神手谷（墨大夫隐居地）地图构建
//  小说第5-6章：韩立跟随墨大夫在此学医、练功
// ============================================================
void WorldMapState::BuildShenshuValley() {
    MapData map;
    map.id = "shenshu_valley";
    map.displayNameW = L"神手谷";
    map.width = 18;
    map.height = 20;
    map.hasRandomEncounter = false;
    map.encounterRate = 0;
    map.enemyPool = {};
    map.defaultSpawnX = 8;
    map.defaultSpawnY = 18;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Ground, 0, -1}));

    // 围墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 南门 -> 回七玄门西门（传送到西门内侧(1,10)，紧挨七玄门西门门格(0,10)）
    map.tiles[map.height-1][8].type = TileType::Door;
    map.tiles[map.height-1][8].warpTargetMap = "qixuanmen";
    map.tiles[map.height-1][8].warpTargetX = 1;
    map.tiles[map.height-1][8].warpTargetY = 10;

    // 北小路 -> 炼骨崖
    map.tiles[0][8].type = TileType::Door;
    map.tiles[0][8].warpTargetMap = "liangu_cliff";
    map.tiles[0][8].warpTargetX = 7;
    map.tiles[0][8].warpTargetY = 19;  // 落点在门前一格

    // === 主路（南北向）===
    for (int y = 2; y < map.height - 1; ++y)
        map.tiles[y][8].type = TileType::Path;

    // === 墨大夫的医馆/住处（中央偏南）===
    for (int y = 10; y <= 16; ++y)
        for (int x = 4; x <= 13; ++x)
            map.tiles[y][x].type = TileType::Floor;
    // 外墙
    for (int x = 4; x <= 13; ++x) {
        map.tiles[10][x].type = TileType::Wall;
        map.tiles[16][x].type = TileType::Wall;
    }
    for (int y = 10; y <= 16; ++y) {
        map.tiles[y][4].type = TileType::Wall;
        map.tiles[y][13].type = TileType::Wall;
    }
    // 大门（南面）
    map.tiles[16][8].type = TileType::Door;

    // === 医馆内部布局 ===
    // 药柜（北侧一排木制区域，用 Floor 表示）
    map.tiles[11][5].type = TileType::Floor;
    map.tiles[11][6].type = TileType::Floor;
    map.tiles[11][7].type = TileType::Floor;
    map.tiles[11][9].type = TileType::Floor;
    map.tiles[11][10].type = TileType::Floor;
    map.tiles[11][11].type = TileType::Floor;
    map.tiles[11][12].type = TileType::Floor;

    // 诊台（中间偏北）
    map.tiles[12][8].type = TileType::NPCSpot;   // 诊台位置

    // 书架（东侧）
    map.tiles[13][12].type = TileType::Floor;

    // === 韩立的小房间（医馆西侧）===
    for (int y = 12; y <= 15; ++y)
        for (int x = 1; x <= 3; ++x)
            map.tiles[y][x].type = TileType::Floor;
    for (int x = 1; x <= 3; ++x) {
        map.tiles[12][x].type = TileType::Wall;
        map.tiles[15][x].type = TileType::Wall;
    }
    for (int y = 12; y <= 15; ++y) {
        map.tiles[y][1].type = TileType::Wall;
        map.tiles[y][3].type = TileType::Wall;
    }
    map.tiles[14][4].type = TileType::Door;  // 小房间门

    // === 后院药圃（医馆北面）===
    for (int y = 5; y <= 9; ++y)
        for (int x = 5; x <= 12; ++x)
            map.tiles[y][x].type = TileType::Grass;   // 种药草
    // 药圃围栏
    for (int x = 5; x <= 12; ++x) {
        map.tiles[4][x].type = TileType::Fence;
        map.tiles[10][x].type = TileType::Fence;
    }
    for (int y = 4; y <= 10; ++y) {
        map.tiles[y][5].type = TileType::Fence;
        map.tiles[y][12].type = TileType::Fence;
    }
    map.tiles[7][12].type = TileType::Door;   // 药圃门

    // 连接路径：主路→医馆→药圃
    map.tiles[17][8].type = TileType::Path;
    for (int y = 8; y <= 16; ++y) map.tiles[y][8].type = TileType::Path;

    // === 小溪（东边缘）===
    for (int y = 2; y <= 18; ++y)
        map.tiles[y][15].type = TileType::Water;

    // 水井（东南角）
    map.tiles[14][14].type = TileType::Floor;
    map.tiles[14][14].npcId = 201;  // 水井标记

    // === 树木装饰 ===
    map.tiles[3][6].type = TileType::Tree;
    map.tiles[6][2].type = TileType::Tree;
    map.tiles[15][16].type = TileType::Tree;
    map.tiles[3][15].type = TileType::Tree;
    map.tiles[2][12].type = TileType::Tree;
    map.tiles[3][14].type = TileType::Tree;
    map.tiles[9][2].type = TileType::Tree;
    map.tiles[8][14].type = TileType::Tree;
    map.tiles[18][12].type = TileType::Tree;
    map.tiles[18][5].type = TileType::Tree;

    // === 石头装饰 ===
    map.tiles[6][14].type = TileType::Ground;  // 用Ground变体做石头
    map.tiles[9][16].type = TileType::Ground;
    map.tiles[17][6].type = TileType::Ground;

    // === 宝箱（后院角落）===
    map.tiles[6][6].type = TileType::Chest;
    map.tiles[6][6].chestLootItemId = "spirit_gathering_pill";
    map.tiles[6][6].chestLootCount = 5;  // 神手谷宝箱给好东西！

    // === NPC：墨大夫（在诊台旁）、张铁（在韩立小房间附近）===
    map.tiles[13][8].npcId = 200;  // 墨大夫（医馆内）
    map.tiles[14][2].npcId = 202;  // 张铁（韩立同窗）

    m_maps[map.id] = std::move(map);
}

// ============================================================
//  青牛镇 — 原著第2章场景
// ============================================================
void WorldMapState::BuildQingniuTown() {
    MapData map;
    map.id = "qingniu_town";
    map.displayNameW = L"青牛镇";
    map.width = 30;
    map.height = 25;
    map.hasRandomEncounter = false;
    map.encounterRate = 0;
    map.enemyPool = {};
    map.defaultSpawnX = 6;
    map.defaultSpawnY = 19;

    map.tiles.assign(map.height, std::vector<Tile>(map.width, {TileType::Grass, 0, -1}));

    // 四周边界墙
    for (int x = 0; x < map.width; ++x) {
        map.tiles[0][x].type = TileType::Wall;
        map.tiles[map.height-1][x].type = TileType::Wall;
    }
    for (int y = 0; y < map.height; ++y) {
        map.tiles[y][0].type = TileType::Wall;
        map.tiles[y][map.width-1].type = TileType::Wall;
    }

    // 镇门：只有东门（右门）→嘉州城；西门已移除
    // 北门（回七玄门）— 北墙开门，落地格用Path连通
    map.tiles[0][12].type = TileType::Door;
    map.tiles[0][12].warpTargetMap = "qixuanmen";
    map.tiles[0][12].warpTargetX = 12;
    map.tiles[0][12].warpTargetY = 18;
    map.tiles[1][12].type = TileType::Path;  // 门内侧路径
    // 东门（右门，行12列29）→嘉州城
    map.tiles[12][map.width-1].type = TileType::Door;
    map.tiles[12][map.width-1].warpTargetMap = "jiazhou";
    map.tiles[12][map.width-1].warpTargetX = 10;
    map.tiles[12][map.width-1].warpTargetY = 1;
    map.tiles[12][map.width-2].type = TileType::Path;
    // 西侧（原西边传送门位置）恢复为墙壁
    map.tiles[12][0].type = TileType::Wall;

    // 主街道（行12全部贯通）
    for (int x = 1; x < map.width-1; ++x)
        if (map.tiles[12][x].type != TileType::Door)
            map.tiles[12][x].type = TileType::Path;

    // 小河（列20, 行3~11）
    for (int y = 3; y <= 11; ++y) map.tiles[y][20].type = TileType::Water;

    // 小桥（行7~8, 列19~21）
    for (int x = 19; x <= 21; ++x) {
        map.tiles[7][x].type = TileType::Path;
        map.tiles[8][x].type = TileType::Path;
    }
    for (int y = 9; y <= 12; ++y) map.tiles[y][20].type = TileType::Path;

    // === 春香酒楼 (行3~9, 列11~18) ===
    for (int x = 11; x <= 18; ++x) { map.tiles[3][x].type = TileType::Wall; map.tiles[9][x].type = TileType::Wall; }
    for (int y = 3; y <= 9; ++y) { map.tiles[y][11].type = TileType::Wall; map.tiles[y][18].type = TileType::Wall; }
    for (int y = 4; y <= 8; ++y) for (int x = 12; x <= 17; ++x) map.tiles[y][x].type = TileType::Floor;
    map.tiles[9][14].type = TileType::Door;
    map.tiles[9][15].type = TileType::Door;
    // 柜台（木桌）
    map.tiles[5][12].type = TileType::Tree;
    map.tiles[5][13].type = TileType::Tree;
    map.tiles[5][14].type = TileType::Tree;
    map.tiles[5][15].type = TileType::Tree;
    map.tiles[5][16].type = TileType::Tree;
    map.tiles[5][17].type = TileType::Tree;
    // 酒楼入口路
    for (int y = 10; y <= 12; ++y) { map.tiles[y][14].type = TileType::Path; map.tiles[y][15].type = TileType::Path; }

    // === 韩家小院 (行14~21, 列2~8) ===
    for (int x = 2; x <= 8; ++x) { map.tiles[14][x].type = TileType::Wall; map.tiles[21][x].type = TileType::Wall; }
    for (int y = 14; y <= 21; ++y) { map.tiles[y][2].type = TileType::Wall; map.tiles[y][8].type = TileType::Wall; }
    for (int y = 15; y <= 20; ++y) for (int x = 3; x <= 7; ++x) map.tiles[y][x].type = TileType::Floor;
    // 内墙（分隔前后院）
    map.tiles[17][3].type = TileType::Wall; map.tiles[17][4].type = TileType::Wall;
    map.tiles[17][5].type = TileType::Wall; map.tiles[17][6].type = TileType::Wall; map.tiles[17][7].type = TileType::Wall;
    for (int y = 15; y <= 16; ++y) for (int x = 3; x <= 7; ++x) map.tiles[y][x].type = TileType::Floor;
    // 院门
    map.tiles[18][8].type = TileType::Door;
    // 出院路
    for (int y = 18; y >= 12; --y) { map.tiles[y][7].type = TileType::Path; map.tiles[y][6].type = TileType::Path; map.tiles[y][5].type = TileType::Path; }

    // === 杂货铺 (行15~20, 列22~27) ===
    for (int x = 22; x <= 27; ++x) { map.tiles[15][x].type = TileType::Wall; map.tiles[20][x].type = TileType::Wall; }
    for (int y = 15; y <= 20; ++y) { map.tiles[y][22].type = TileType::Wall; map.tiles[y][27].type = TileType::Wall; }
    for (int y = 16; y <= 19; ++y) for (int x = 23; x <= 26; ++x) map.tiles[y][x].type = TileType::Floor;
    map.tiles[20][24].type = TileType::Door;
    map.tiles[20][25].type = TileType::Door;
    for (int y = 21; y <= 22; ++y) { map.tiles[y][24].type = TileType::Path; map.tiles[y][25].type = TileType::Path; }

    // === 铁匠铺 (行3~7, 列3~7) ===
    for (int x = 3; x <= 7; ++x) { map.tiles[3][x].type = TileType::Wall; map.tiles[7][x].type = TileType::Wall; }
    for (int y = 3; y <= 7; ++y) { map.tiles[y][3].type = TileType::Wall; map.tiles[y][7].type = TileType::Wall; }
    for (int y = 4; y <= 6; ++y) for (int x = 4; x <= 6; ++x) map.tiles[y][x].type = TileType::Floor;
    map.tiles[5][5].type = TileType::Tree;  // 铁砧
    map.tiles[7][5].type = TileType::Door;
    for (int y = 8; y <= 12; ++y) map.tiles[y][5].type = TileType::Path;

    // === 装饰性元素 ===
    // 花丛
    map.tiles[2][16].type = TileType::Grass;  map.tiles[2][17].type = TileType::Grass;
    map.tiles[22][10].type = TileType::Grass;  map.tiles[23][10].type = TileType::Grass;
    map.tiles[10][25].type = TileType::Grass;  map.tiles[11][26].type = TileType::Grass;
    map.tiles[22][18].type = TileType::Grass;  map.tiles[22][19].type = TileType::Grass;
    map.tiles[13][9].type = TileType::Grass;  map.tiles[13][10].type = TileType::Grass;  map.tiles[14][10].type = TileType::Grass;
    map.tiles[13][20].type = TileType::Grass; map.tiles[13][21].type = TileType::Grass;
    // 石头
    map.tiles[6][10].type = TileType::Ground;
    map.tiles[7][11].type = TileType::Ground;
    // 篱笆
    map.tiles[19][2].type = TileType::Fence;
    map.tiles[19][3].type = TileType::Fence;
    // 树
    map.tiles[10][3].type = TileType::Tree;
    map.tiles[2][10].type = TileType::Tree;
    map.tiles[22][14].type = TileType::Tree;
    map.tiles[23][22].type = TileType::Tree;

    // === NPC ===
    // 铁匠 (铁匠铺内)
    map.tiles[5][5].npcId = 300;  // blacksmith
    // 三叔 (韩家小院附近)
    map.tiles[16][5].npcId = 301;  // uncle
    // 秘籍商人 (酒楼南侧)
    map.tiles[12][15].npcId = 302;  // technique_merchant
    // 装备商 (杂货铺东侧)
    map.tiles[18][25].npcId = 303;  // equip_merchant
    // 杂货商 (杂货铺内)
    map.tiles[17][24].npcId = 304;  // shopkeeper

    m_maps[map.id] = std::move(map);
}
void WorldMapState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(15, 25, 35));

    float camX = m_camera.GetX();
    float camY = m_camera.GetY();

    // 渲染地图
        if (m_currentMap)
        m_tileMap.Render(window, sf::Vector2f(camX, camY));

    // 渲染玩家
    m_player.Render(window, -camX, -camY);

    // 渲染 NPC
    if (m_currentMap)
        m_tileMap.RenderNPCs(window, sf::Vector2f(-camX, -camY), 0.f);

    // 渲染地图敌人
    RenderMapEnemies(window, -camX, -camY);

    // 渲染 HUD（屏幕空间）
    RenderHUD(window);

    // 渲染 NPC 名字（屏幕空间）
    RenderNPCNames(window);

    // 渲染交互UI（选项菜单/对话框）— 屏幕空间，最上层
    RenderInteractionUI(window);

    // 渲染浮动提示文字
    if (m_floatTimer > 0.f && !m_floatMsg.empty()) {
        float alpha = std::min(1.f, m_floatTimer / 1.f);
        m_floatText.setString(m_floatMsg);
        // 居中偏上显示
        sf::FloatRect bounds = m_floatText.getLocalBounds();
        m_floatText.setPosition(400.f - bounds.width / 2.f, 200.f);
        m_floatText.setFillColor(sf::Color(
            255, 220, 80,
            static_cast<sf::Uint8>(255 * alpha)
        ));
        // 发光背景
        sf::RectangleShape glowBg(sf::Vector2f(bounds.width + 30.f, 28.f));
        glowBg.setFillColor(sf::Color(20, 15, 5, static_cast<sf::Uint8>(180 * alpha)));
        glowBg.setOutlineThickness(1.f);
        glowBg.setOutlineColor(sf::Color(180, 140, 40, static_cast<sf::Uint8>(200 * alpha)));
        glowBg.setPosition(385.f - bounds.width / 2.f, 196.f);
        window.draw(glowBg);
        window.draw(m_floatText);
    }
}

// ============================================================
//  HUD 渲染
// ============================================================
void WorldMapState::RenderHUD(sf::RenderWindow& window) {
    // 顶部地图名称
    if (m_showMapName && m_currentMap) {
        sf::Text mapName;
        mapName.setFont(m_font);
        mapName.setString(m_currentMap->displayNameW);
        mapName.setCharacterSize(24);
        mapName.setFillColor(sf::Color(220, 200, 120, 230));
        sf::FloatRect bounds = mapName.getLocalBounds();
        mapName.setPosition((800 - bounds.width) / 2.f, 12.f);

        // 文字背景
        sf::RectangleShape nameBg(sf::Vector2f(bounds.width + 20.f, bounds.height + 8.f));
        nameBg.setFillColor(sf::Color(0, 0, 0, 140));
        nameBg.setPosition((800 - bounds.width) / 2.f - 10.f, 8.f);
        window.draw(nameBg);

        window.draw(mapName);
    }

    // 底部操作提示
    sf::Text hint;
    hint.setFont(m_font);
    hint.setString(L"WASD/方向键移动  F互动  E对话  Q任务  C修炼  I背包  Esc菜单");
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(120, 120, 120));
    hint.setPosition(40.f, 580.f);
    window.draw(hint);

    // 右侧玩家状态
    Player& player = GameSession::Instance().GetPlayer();
    sf::RectangleShape panel(sf::Vector2f(190.f, 100.f));
    panel.setFillColor(sf::Color(20, 30, 45, 210));
    panel.setPosition(600.f, 8.f);
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(60, 80, 100));
    window.draw(panel);

    // 状态文字
    sf::Text status;
    status.setFont(m_font);
    status.setCharacterSize(13);

    auto textY = 13.f;
    auto lineH = 17.f;

    // 名字
    status.setString(L"【" + player.GetName() + L"】");
    status.setFillColor(sf::Color(100, 200, 255));
    status.setPosition(610.f, textY);
    window.draw(status);
    textY += lineH;

    // 境界
    status.setString(L"境界：" + player.GetRealmName());
    status.setFillColor(sf::Color(200, 180, 120));
    status.setPosition(610.f, textY);
    window.draw(status);
    textY += lineH;

    // HP
    status.setString(L"HP: " + std::to_wstring(player.GetCurrentHp()) +
                       L"/" + std::to_wstring(player.GetMaxHp()));
    status.setFillColor(sf::Color(220, 80, 80));
    status.setPosition(610.f, textY);
    window.draw(status);
    textY += lineH;

    // MP
    status.setString(L"MP: " + std::to_wstring(player.GetCurrentMp()) +
                       L"/" + std::to_wstring(player.GetMaxMp()));
    status.setFillColor(sf::Color(80, 120, 220));
    status.setPosition(610.f, textY);
    window.draw(status);
    textY += lineH;

    // 灵石
    status.setString(L"灵石: " + std::to_wstring(player.GetGold()));
    status.setFillColor(sf::Color(220, 200, 80));
    status.setPosition(610.f, textY);
    window.draw(status);
    textY += lineH;

    // ===== 当前任务追踪器 =====
    const auto& allQuests = QuestSystem::Instance().GetAllQuests();
    std::wstring questTitle;
    std::wstring questDesc;
    for (const auto& q : allQuests) {
        if (q.status == QuestStatus::Active) {
            questTitle = Utf8ToWide(q.name);
            questDesc = Utf8ToWide(q.description);
            break;
        }
    }
    if (!questTitle.empty()) {
        // 任务标题
        sf::Text qt;
        qt.setFont(m_font);
        qt.setString(L"⚡ " + questTitle);
        qt.setCharacterSize(15);
        qt.setFillColor(sf::Color(255, 215, 80));
        qt.setPosition(10.f, 540.f);
        window.draw(qt);

        // 任务描述（单行截断）
        sf::Text qd;
        qd.setFont(m_font);
        // 最多显示两行（约54个中文字符）
        if (questDesc.size() > 54) questDesc = questDesc.substr(0, 54) + L"...";
        qd.setString(questDesc);
        qd.setCharacterSize(12);
        qd.setFillColor(sf::Color(180, 180, 160));
        qd.setPosition(10.f, 560.f);
        window.draw(qd);

        // 任务提示
        sf::Text qh;
        qh.setFont(m_font);
        qh.setString(L"按 Q 查看详情");
        qh.setCharacterSize(10);
        qh.setFillColor(sf::Color(100, 100, 100));
        qh.setPosition(10.f, 577.f);
        window.draw(qh);
    }
}

// ============================================================
//  渲染地图敌人
// ============================================================
void WorldMapState::RenderMapEnemies(sf::RenderWindow& window, float camOffsetX, float camOffsetY) {
    m_enemySystem.Render(window, camOffsetX, camOffsetY);
}

// ============================================================
//  渲染 NPC 名字标签
// ============================================================
void WorldMapState::RenderNPCNames(sf::RenderWindow& window) {
    const auto& npcs = m_tileMap.GetNPCs();
    float camX = m_camera.GetX();
    float camY = m_camera.GetY();

    for (const auto& npc : npcs) {
        // 世界坐标转屏幕坐标
        float sx = npc.mapX * TileSet::TILE_SIZE - camX + TileSet::TILE_SIZE / 4;
        float sy = npc.mapY * TileSet::TILE_SIZE - camY - 8;

        if (sx < -200 || sx > 900 || sy < -200 || sy > 700) continue;

        sf::Text nameTag;
        nameTag.setFont(m_font);
        nameTag.setString(npc.nameW);
        nameTag.setCharacterSize(11);
        nameTag.setFillColor(sf::Color(255, 255, 200));

        sf::FloatRect bounds = nameTag.getLocalBounds();
        nameTag.setPosition(sx + (TileSet::TILE_SIZE / 2.f - bounds.width / 2.f), sy);

        // 背景
        sf::RectangleShape bg(sf::Vector2f(bounds.width + 4, bounds.height + 2));
        bg.setFillColor(sf::Color(0, 0, 0, 150));
        bg.setPosition(sx + (TileSet::TILE_SIZE / 2.f - bounds.width / 2.f) - 2, sy - 1);
        window.draw(bg);

        window.draw(nameTag);
    }
}

// ============================================================
//  键盘输入处理
// ============================================================
void WorldMapState::OnKeyPressed(sf::Keyboard::Key key) {
    // 如果处于交互状态（选项菜单/对话），优先处理交互输入
    if (m_interactState != InteractState::None) {
        ProcessInteractionInput(key);
        return;
    }

    int dx = 0, dy = 0;

    switch (key) {
        // === 移动 ===
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            dy = -1;
            m_player.SetDirection(Dir::Up);
            break;
        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            dy = 1;
            m_player.SetDirection(Dir::Down);
            break;
        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            dx = -1;
            m_player.SetDirection(Dir::Left);
            break;
        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            dx = 1;
            m_player.SetDirection(Dir::Right);
            break;

        // === F键：与附近NPC互动（显示选项菜单）===
        case sf::Keyboard::F: {
            // 检查玩家自身及周围4格是否有NPC
            int checkDirs[5][2] = {{0,0},{0,-1},{0,1},{-1,0},{1,0}};
            const MapNPC* nearbyNpc = nullptr;
            for (int d = 0; d < 5; ++d) {
                int nx = m_playerTileX + checkDirs[d][0];
                int ny = m_playerTileY + checkDirs[d][1];
                const MapNPC* npc = m_tileMap.GetNPCAt(nx, ny);
                if (npc) {
                    nearbyNpc = npc;
                    break;
                }
            }
            if (nearbyNpc) {
                StartNPCOptions(nearbyNpc->id);
            }
            return;
        }

        // === 对话 ===
        case sf::Keyboard::E:
            TryMovePlayer(0, 0); // 触发当前格子的 NPC
            return;

        // === 修炼 ===
        case sf::Keyboard::Space:
            extern void OnOpenCultivationUI();
            OnOpenCultivationUI();
            return;

        // === 背包 ===
        case sf::Keyboard::I:
            extern void OnOpenInventory();
            OnOpenInventory();
            return;

        // === 任务 ===
        case sf::Keyboard::Q:
            extern void OnOpenQuestUI();
            OnOpenQuestUI();
            return;

        // === 打开设置（可存档/读档/返回主菜单）===
        case sf::Keyboard::Escape:
            GameStateManager::Instance().PushState(std::make_unique<SettingsState>());
            return;

        default:
            return;
    }

    if (dx != 0 || dy != 0) {
        TryMovePlayer(dx, dy);
    }
}

// ============================================================
//  尝试移动玩家
// ============================================================
void WorldMapState::TryMovePlayer(int dx, int dy) {
    // 如果正在移动中，忽略
    if (m_player.IsMoving()) return;

    // 移动节流
    if (m_moveCooldown > 0.f) return;
    m_moveCooldown = MOVE_INTERVAL;

    int newX = m_playerTileX + dx;
    int newY = m_playerTileY + dy;

    // 检查边界
    if (!m_currentMap || !m_currentMap->InBounds(newX, newY)) return;

    // 如果没有移动（dx=dy=0），只是触发当前格子
    if (dx == 0 && dy == 0) {
        HandleSpecialTile();
        return;
    }

    // 碰撞检测
    if (!m_tileMap.IsWalkable(newX, newY)) {
        // 碰到墙/树，不移动，但可以检查一下是否按 E 触发这一格的 NPC/Door
        HandleSpecialTile();
        return;
    }

    // 执行移动
    m_playerTileX = newX;
    m_playerTileY = newY;
    m_player.MoveTo(newX, newY);

    // 处理移动后的特殊格子
    HandleSpecialTile();
}

// ============================================================
//  处理特殊格子（Door/NPC/Grass/Chest）
// ============================================================
void WorldMapState::HandleSpecialTile() {
    if (!m_currentMap) return;

    auto& tile = m_currentMap->GetTile(m_playerTileX, m_playerTileY);
    TileType t = tile.type;

    // === Chest: 开箱获取物品 ===
    if (t == TileType::Chest) {
        if (!tile.chestOpened) {
            // 开箱！
            tile.chestOpened = true;
            tile.variant = 2;  // variant>=2 时渲染为打开状态

            // 持久化记录：将此宝箱标记为已开启（格式 "mapId@x,y"）
            std::string chestKey = m_currentMap->id + "@"
                + std::to_string(m_playerTileX) + ","
                + std::to_string(m_playerTileY);
            GameSession::Instance().GetOpenedChests().insert(chestKey);

            // 给予物品
            if (!tile.chestLootItemId.empty()) {
                extern void OnGiveItem(const std::string& itemId, int count);
                OnGiveItem(tile.chestLootItemId, tile.chestLootCount);
                // 通知任务系统（检测CollectItem类型任务）
                extern void OnGetItem(const std::string& itemId, int count);
                OnGetItem(tile.chestLootItemId, tile.chestLootCount);

                // 查询物品名用于提示
                const auto* itemData = ConfigManager::Instance().GetItem(tile.chestLootItemId);
                std::wstring itemName = itemData ? Utf8ToWide(itemData->name) : Utf8ToWide(tile.chestLootItemId);
                ShowFloatingText(L"获得 " + itemName + L" x" + std::to_wstring(tile.chestLootCount));
            } else {
                // 默认给些灵石
                GameSession::Instance().GetPlayer().AddGold(20 + rand() % 30);
                ShowFloatingText(L"获得了灵石！");
            }
        } else {
            ShowFloatingText(L"宝箱已经开启过了...");
        }
        return;  // 宝箱格不触发后续 Door/NPC 等逻辑
    }

    // === Door: 传送 ===
    if (t == TileType::Door && !tile.warpTargetMap.empty()) {
        SwitchToMap(tile.warpTargetMap, tile.warpTargetX, tile.warpTargetY);
        return;
    }

    // === Grass: 遇敌检查 ===
    if (t == TileType::Grass && tile.encounterRate > 0) {
        TryEncounter();
    }

    // === NPC: 检查当前格子或相邻格子是否有 NPC ===
    const MapNPC* npc = m_tileMap.GetNPCAt(m_playerTileX, m_playerTileY);
    if (!npc && t == TileType::NPCSpot) {
        // 检查周围4格是否有 NPC
        for (int d = 0; d < 4; ++d) {
            int nx = m_playerTileX + (d==2?-1:(d==3?1:0));
            int ny = m_playerTileY + (d==0?-1:(d==1?1:0));
            npc = m_tileMap.GetNPCAt(nx, ny);
            if (npc) break;
        }
    }

    // 如果按了 E 键，触发 NPC 对话
    // （这个逻辑在 OnKeyPressed 里判断 E 后再调用 TryMovePlayer(0,0) ）
    if (npc) {
        // 暂时只显示提示，实际对话在 E 键触发
        // 对话触发由 GameCallbacks 处理
    }
}

// ============================================================
//  触发遇敌
// ============================================================
void WorldMapState::TryEncounter() {
    if (!m_currentMap) return;
    if (!m_currentMap->hasRandomEncounter) return;
    if (m_encounterCooldown > 0.f) return;

    const auto& tile = m_currentMap->GetTile(m_playerTileX, m_playerTileY);
    int rate = tile.encounterRate > 0 ? tile.encounterRate : m_currentMap->encounterRate;

    int roll = rand() % 100;
    if (roll < rate) {
        if (!m_currentMap->enemyPool.empty()) {
            std::string enemyId = m_currentMap->enemyPool[rand() % m_currentMap->enemyPool.size()];
            OnStartCombat(enemyId);
        }
        m_encounterCooldown = 3.f;
    }
}

// ============================================================
//  NPC 整数 ID → 字符串 ID 映射（用于接入 DialogueState）
// ============================================================
static const std::map<int, std::string> s_npcIdToString = {
    { 1, "yaofang_huoji" },   // 药房伙计
    { 3, "li_feiyu" },         // 厉飞雨
    { 4, "zhang_tie" },        // 张铁
    { 5, "wan_xiaoshan" },     // 万小山
    { 6, "shanmin" },          // 山民
    { 7, "wuqishang" },        // 武器商
    { 8, "danyaoshi" },        // 丹药师
    { 9, "zahuoshang" },       // 杂货商
    { 10, "shuoshuren" },      // 说书人
    { 11, "xinghai_shouhuzhe" }, // 星海守护者
    { 14, "wang_hufa" },       // 王护法
    { 15, "yue_tangzhu" },     // 岳堂主
    { 100, "shoumen_dizi" },   // 守门弟子
    { 200, "mo_dafu" },        // 墨大夫（炼骨崖/神手谷）
    { 202, "zhang_tie" },      // 张铁（神手谷）
    { 301, "san_shu" },        // 三叔（青牛镇）
};

// ============================================================
//  对话数据（备用：当 DialogueState 无数据时使用）
// ============================================================
static const std::map<int, std::vector<std::wstring>> s_dialogueData = {
    { 3, { // 厉飞雨
            L"韩兄！今天练功如何？",
            L"我最近在研究一套新的拳法，感觉威力不错。",
            L"咱们一起修炼的日子真是让人怀念啊。",
            L"对了，听说墨大夫最近在炼什么新丹药？",
            L"有朝一日，我厉飞雨也要成为一方强者！",
        }},
    { 4, { // 张铁
            L"嘿嘿，韩兄弟，你来得正好！",
            L"我刚从山下回来，带了些好吃的。",
            L"这七玄门的饭真难吃啊……",
        }},
};

// ============================================================
//  切磋敌人ID映射
// ============================================================
static const std::map<int, std::string> s_duelEnemyIds = {
    { 3, "li_feiyu" },     // 厉飞雨切磋对手
    { 4, "zhang_tie" },    // 张铁切磋对手
    { 202, "zhang_tie" },  // 张铁（神手谷）
};

// ============================================================
//  开始与NPC交互（显示选项菜单）
// ============================================================
void WorldMapState::StartNPCOptions(int npcId) {
    m_interactState = InteractState::ShowOptions;
    m_nearbyNPCId = npcId;
    m_selectedOption = 0; // 默认选第一项
}

// ============================================================
//  开始对话 → 接入完整的 DialogueState 对话系统
// ============================================================
void WorldMapState::StartNPCDialogue(int npcId) {
    // 秘籍商人（ID=12）直接打开功法商店
    if (npcId == 12) {
        m_interactState = InteractState::None;
        extern void OnOpenShop(const std::wstring& title, const std::string& shopType);
        OnOpenShop(L"◆ 功法秘籍商 ◆", "technique");
        return;
    }
    // 车夫（ID=13）传送到青牛镇
    if (npcId == 13) {
        m_interactState = InteractState::None;
        extern void OnGoToQingniuTown();
        OnGoToQingniuTown();
        return;
    }
    // 青牛镇铁匠（ID=300）→ 武器商店
    if (npcId == 300) {
        m_interactState = InteractState::None;
        extern void OnOpenShop(const std::wstring& title, const std::string& shopType);
        OnOpenShop(L"◆ 铁匠铺 ◆", "weapon");
        return;
    }
    // 青牛镇秘籍商人（ID=302）→ 功法商店
    if (npcId == 302) {
        m_interactState = InteractState::None;
        extern void OnOpenShop(const std::wstring& title, const std::string& shopType);
        OnOpenShop(L"◆ 功法秘籍商 ◆", "technique");
        return;
    }
    // 青牛镇装备商（ID=303）→ 装备商店
    if (npcId == 303) {
        m_interactState = InteractState::None;
        extern void OnOpenShop(const std::wstring& title, const std::string& shopType);
        OnOpenShop(L"◆ 装备店 ◆", "equipment");
        return;
    }
    // 青牛镇杂货商（ID=304）→ 杂货商店
    if (npcId == 304) {
        m_interactState = InteractState::None;
        extern void OnOpenShop(const std::wstring& title, const std::string& shopType);
        OnOpenShop(L"◆ 杂货铺 ◆", "misc");
        return;
    }
    // 查找 NPC 字符串 ID
    auto idIt = s_npcIdToString.find(npcId);
    if (idIt != s_npcIdToString.end()) {
        // 有字符串 ID 映射 → 调用完整 DialogueState 系统
        m_interactState = InteractState::None;
        OnTalkToNPC(idIt->second);
        return;
    }
    // 无映射的 NPC → 使用备用简单对话数据
    auto it = s_dialogueData.find(npcId);
    if (it != s_dialogueData.end()) {
        m_dialogueLines = it->second;
    } else {
        m_dialogueLines = { L"这位朋友，有什么事吗？" };
    }
    m_interactState = InteractState::InDialogue;
    m_dialogueLineIndex = 0;
}

// ============================================================
//  开始切磋
// ============================================================
void WorldMapState::StartNPCDuel(int npcId) {
    auto it = s_duelEnemyIds.find(npcId);
    if (it != s_duelEnemyIds.end()) {
        // 关闭交互界面
        m_interactState = InteractState::None;
        // 启动战斗
        OnStartCombat(it->second);
    } else {
        // 该NPC不可切磋
        StartNPCDialogue(npcId); // 改为显示对话
        m_dialogueLines = { L"这个人似乎不想和你切磋。" };
        m_dialogueLineIndex = 0;
    }
}

// ============================================================
//  处理交互状态下的输入
// ============================================================
void WorldMapState::ProcessInteractionInput(sf::Keyboard::Key key) {
    switch (m_interactState) {

    case InteractState::ShowOptions:
        // 选项菜单状态：W/S选择，F/Enter确认，Esc取消
        switch (key) {
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            m_selectedOption = (m_selectedOption + 2) % 3; // 3个选项循环
            break;
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            m_selectedOption = (m_selectedOption + 1) % 3;
            break;
        case sf::Keyboard::F:
        case sf::Keyboard::Return: {
            int npcId = m_nearbyNPCId;
            switch (m_selectedOption) {
            case 0: StartNPCDialogue(npcId); break;   // 对话
            case 1: StartNPCDuel(npcId); break;       // 切磋
            case 2: m_interactState = InteractState::None; break; // 取消
            }
            break;
        }
        case sf::Keyboard::Escape:
            m_interactState = InteractState::None;
            break;
        default:
            break;
        }
        break;

    case InteractState::InDialogue:
        // 对话状态：Space/Enter翻页/关闭，Esc关闭
        switch (key) {
        case sf::Keyboard::Space:
        case sf::Keyboard::Return:
        case sf::Keyboard::F:
            m_dialogueLineIndex += 2; // 每页2行
            if (m_dialogueLineIndex >= m_dialogueLines.size()) {
                m_interactState = InteractState::None; // 对话结束
            }
            break;
        case sf::Keyboard::Escape:
            m_interactState = InteractState::None;
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

// ============================================================
//  浮动提示文字（宝箱获得物品等）
// ============================================================
void WorldMapState::ShowFloatingText(const std::wstring& msg) {
    m_floatMsg = msg;
    m_floatTimer = 3.f;  // 显示3秒
}

// ============================================================
//  渲染交互UI（选项菜单 + 对话框）
// ============================================================
void WorldMapState::RenderInteractionUI(sf::RenderWindow& window) {
    if (m_interactState == InteractState::None) return;

    if (m_interactState == InteractState::ShowOptions) {
        // === 渲染选项菜单 ===
        float panelW = 220.f;
        float panelH = 140.f;
        float panelX = (800 - panelW) / 2.f;
        float panelY = (600 - panelH) / 2.f;

        // 半透明背景面板
        sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
        panel.setFillColor(sf::Color(20, 25, 40, 235));
        panel.setPosition(panelX, panelY);
        panel.setOutlineThickness(2.f);
        panel.setOutlineColor(sf::Color(100, 140, 180));
        window.draw(panel);

        // 标题：获取NPC名字
        const MapNPC* targetNpc = nullptr;
        for (const auto& npc : m_tileMap.GetNPCs()) {
            if (npc.id == m_nearbyNPCId) { targetNpc = &npc; break; }
        }

        sf::Text title;
        title.setFont(m_font);
        if (targetNpc)
            title.setString(L"【" + targetNpc->nameW + L"】");
        else
            title.setString(L"【???】");
        title.setCharacterSize(16);
        title.setFillColor(sf::Color(255, 215, 100));
        title.setPosition(panelX + 15.f, panelY + 12.f);
        window.draw(title);

        // 分割线
        sf::RectangleShape line(sf::Vector2f(panelW - 30.f, 1.f));
        line.setFillColor(sf::Color(80, 100, 130));
        line.setPosition(panelX + 10.f, panelY + 38.f);
        window.draw(line);

        // 选项列表
        const wchar_t* options[3] = { L"[A] 对话", L"[B] 切磋", L"[C] 取消" };
        for (int i = 0; i < 3; ++i) {
            float optX = panelX + 25.f;
            float optY = panelY + 50.f + i * 28.f;

            // 高亮选中项背景
            if (i == m_selectedOption) {
                sf::RectangleShape selBg(sf::Vector2f(panelW - 50.f, 24.f));
                selBg.setFillColor(sf::Color(60, 90, 130, 150));
                selBg.setPosition(optX - 5.f, optY - 2.f);
                window.draw(selBg);

                // 左侧箭头
                sf::RectangleShape arrow(sf::Vector2f(6.f, 12.f));
                arrow.setFillColor(sf::Color(255, 200, 80));
                arrow.setPosition(optX + 5.f, optY + 5.f);
                window.draw(arrow);
            }

            // 选项文字
            sf::Text optText;
            optText.setFont(m_font);
            optText.setString(options[i]);
            optText.setCharacterSize(14);
            if (i == m_selectedOption)
                optText.setFillColor(sf::Color(255, 255, 230));
            else
                optText.setFillColor(sf::Color(170, 175, 185));
            optText.setPosition(optX + 18.f, optY + 3.f);
            window.draw(optText);
        }

        // 底部提示
        sf::Text hint;
        hint.setFont(m_font);
        hint.setString(L"↑↓ 选择  F 确认  Esc 取消");
        hint.setCharacterSize(11);
        hint.setFillColor(sf::Color(120, 125, 135));
        hint.setPosition(panelX + 25.f, panelY + panelH - 22.f);
        window.draw(hint);
    }
    else if (m_interactState == InteractState::InDialogue) {
        // === 渲染对话框 ===
        float boxW = 720.f;
        float boxH = 130.f;
        float boxX = 40.f;
        float boxY = 600 - boxH - 30.f;

        // 对话框半透明背景
        sf::RectangleShape dialogBox(sf::Vector2f(boxW, boxH));
        dialogBox.setFillColor(sf::Color(15, 20, 35, 240));
        dialogBox.setPosition(boxX, boxY);
        dialogBox.setOutlineThickness(2.f);
        dialogBox.setOutlineColor(sf::Color(80, 110, 150));
        window.draw(dialogBox);

        // 获取NPC名字做标题
        const MapNPC* targetNpc = nullptr;
        for (const auto& npc : m_tileMap.GetNPCs()) {
            if (npc.id == m_nearbyNPCId) { targetNpc = &npc; break; }
        }

        // 名字标签
        sf::Text nameTag;
        nameTag.setFont(m_font);
        if (targetNpc)
            nameTag.setString(L"◆ " + targetNpc->nameW);
        else
            nameTag.setString(L"◆ ???");
        nameTag.setCharacterSize(14);
        nameTag.setFillColor(sf::Color(100, 200, 255));
        nameTag.setPosition(boxX + 15.f, boxY + 8.f);
        window.draw(nameTag);

        // 分割线
        sf::RectangleShape dLine(sf::Vector2f(boxW - 30.f, 1.f));
        dLine.setFillColor(sf::Color(60, 85, 115));
        dLine.setPosition(boxX + 15.f, boxY + 32.f);
        window.draw(dLine);

        // 显示当前页的对话文本（每页最多3行）
        sf::Text dlgText;
        dlgText.setFont(m_font);
        dlgText.setCharacterSize(15);
        dlgText.setFillColor(sf::Color(225, 225, 220));

        size_t maxLinesPerPage = 3;
        for (size_t i = 0; i < maxLinesPerPage && m_dialogueLineIndex + i < m_dialogueLines.size(); ++i) {
            const std::wstring& line = m_dialogueLines[m_dialogueLineIndex + i];
            dlgText.setString(line);
            dlgText.setPosition(boxX + 20.f, boxY + 40.f + i * 26.f);
            window.draw(dlgText);
        }

        // 页码指示
        if (m_dialogueLines.size() > maxLinesPerPage) {
            int totalPages = (int)(m_dialogueLines.size() + maxLinesPerPage - 1) / (int)maxLinesPerPage;
            int curPage = (int)m_dialogueLineIndex / (int)maxLinesPerPage + 1;

            sf::Text pageHint;
            pageHint.setFont(m_font);
            pageHint.setString(std::to_wstring(curPage) + L" / " + std::to_wstring(totalPages));
            pageHint.setCharacterSize(11);
            pageHint.setFillColor(sf::Color(130, 135, 145));
            pageHint.setPosition(boxX + boxW - 55.f, boxY + 8.f);
            window.draw(pageHint);
        }

        // 底部提示
        sf::Text contHint;
        contHint.setFont(m_font);
        bool isLastPage = (m_dialogueLineIndex + maxLinesPerPage >= m_dialogueLines.size());
        contHint.setString(isLastPage ? L"按 Space/F/Esc 继续" : L"按 Space/F 翻页  Esc 关闭");
        contHint.setCharacterSize(11);
        contHint.setFillColor(sf::Color(130, 135, 145));
        contHint.setPosition(boxX + 20.f, boxY + boxH - 22.f);
        window.draw(contHint);
    }
}
