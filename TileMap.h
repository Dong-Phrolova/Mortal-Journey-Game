#pragma once
#include "TileSet.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// 单个瓦片数据
struct Tile {
    TileType type = TileType::Ground;
    int variant = 0;            // 同类型的视觉变体 (0~3)
    int npcId = -1;             // 该格上的NPC (-1=无)
    std::string warpTargetMap;  // 传送目标地图ID（仅Door有效）
    int warpTargetX = -1;       // 传送后玩家 X 坐标
    int warpTargetY = -1;       // 传送后玩家 Y 坐标
    int encounterRate = 0;      // 该格遇敌概率(0~100)，仅Grass有效
    // 宝箱字段
    bool chestOpened = false;          // 是否已开启
    std::string chestLootItemId;       // 宝箱内物品ID
    int chestLootCount = 1;            // 物品数量
};

// 地图定义数据
struct MapData {
    std::string id;             // 地图唯一ID
    std::string displayName;    // 显示名称（ASCII）
    std::wstring displayNameW;  // 宽字符显示名称
    int width = 0;              // 宽度（格数）
    int height = 0;             // 高度（格数）
    std::vector<std::vector<Tile>> tiles;
    int playerStartX = 1;       // 玩家进入时的初始X
    int playerStartY = 1;       // 玩家进入时的初始Y
    int defaultSpawnX = 1;      // 默认出生点 X
    int defaultSpawnY = 1;      // 默认出生点 Y
    int minRealmLevel = 0;      // 进入所需最低境界等级(0=无限制)

    // 遇敌配置
    bool hasRandomEncounter = false;
    int encounterRate = 0;     // 0~100
    std::vector<std::string> enemyPool;

    // 获取指定坐标的瓦片（带边界检查）
    const Tile& GetTile(int x, int y) const {
        static Tile emptyTile;
        if (!InBounds(x, y)) return emptyTile;
        return tiles[y][x];
    }
    Tile& GetTile(int x, int y) {
        static Tile emptyTile;
        if (!InBounds(x, y)) return emptyTile;
        return tiles[y][x];
    }

    bool InBounds(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height;
    }
};

// NPC 定义（在地图上显示）
struct MapNPC {
    int id;                     // NPC唯一ID
    std::string name;           // 名字
    std::wstring nameW;         // 宽字符名字
    sf::Color color;            // 角色主色调（像素绘制用）
    int mapX, mapY;             // 在当前地图上的位置
    int faceDir;                // 朝向: 0=下 1=上 2=左 3=右
    bool isMoving;              // 是否在走动

    // 对话触发回调（由外部设置）
    std::function<void()> onTalk;
};

// 瓦片地图系统
class TileMap {
public:
    TileMap();

    // 加载/切换地图
    void LoadMap(const MapData* mapData);

    // 获取当前地图数据
    const MapData* GetCurrentMap() const { return m_mapData; }
    MapData* GetCurrentMap() { return const_cast<MapData*>(m_mapData); }

    // 渲染整张地图到 target，使用 camera 的视口偏移
    void Render(sf::RenderTarget& target, const sf::Vector2f& cameraOffset);

    // 碰撞检测
    bool IsWalkable(int x, int y) const;

    // 获取某位置的瓦片类型
    TileType GetTileType(int x, int y) const;

    // 获取 Door 传送目标
    const Tile* GetDoorAt(int x, int y) const;

    // 获取某位置的 NPC
    const MapNPC* GetNPCAt(int x, int y) const;

    // 添加/清除 NPC
    void AddNPC(const MapNPC& npc);
    void ClearNPCs();
    void RemoveNPC(int id);  // 按ID移除NPC

    // 渲染所有 NPC
    void RenderNPCs(sf::RenderTarget& target, const sf::Vector2f& cameraOffset,
                    float deltaTime);

    // 更新 NPC 动画
    void UpdateNPCs(float dt);

    // 获取所有 NPC
    const std::vector<MapNPC>& GetNPCs() const { return m_npcs; }

private:
    const MapData* m_mapData = nullptr;
    TileSet m_tileset;
    std::vector<MapNPC> m_npcs;

    // 绘制单个瓦片
    void DrawTile(sf::RenderTarget& target, const Tile& tile,
                  int screenX, int screenY);

    // 绘制单个 NPC 像素角色
    void DrawNPCSprite(sf::RenderTarget& target, const MapNPC& npc,
                       int screenX, int screenY);
};
