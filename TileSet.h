#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

// 瓦片类型
enum class TileType : int {
    Ground = 0,    // 草地 - 可行走
    Path,          // 泥土路 - 可行走
    Floor,         // 石地板(室内) - 可行走
    Wall,          // 石墙 - 不可走
    Water,         // 水 - 不可走
    Tree,          // 树 - 不可走(装饰)
    Door,          // 门/传送点 - 可行走+传送
    NPCSpot,       // NPC站位 - 可行走
    Chest,         // 宝箱 - 可行走
    Grass,         // 高草 - 可行走+遇敌
    Fence,         // 栅栏 - 不可走
    COUNT          // 类型总数
};

// 判断瓦片是否可通行
inline bool IsTileWalkable(TileType t) {
    return t == TileType::Ground || t == TileType::Path ||
           t == TileType::Floor   || t == TileType::Door ||
           t == TileType::NPCSpot || t == TileType::Chest ||
           t == TileType::Grass;
}

// 判断瓦片是否触发遇敌
inline bool TileTriggersEncounter(TileType t) {
    return t == TileType::Grass;
}

// 图块集：程序化生成像素风纹理
class TileSet {
public:
    static const int TILE_SIZE = 32; // 每个图块 32x32 像素

    TileSet();

    // 获取指定类型的图块纹理（支持 variant 变体）
    const sf::Texture& GetTile(TileType type, int variant = 0) const;

    // 预渲染所有图块到独立纹理（避免运行时重建）
    void GenerateAll();

private:
    std::unordered_map<int, sf::Texture> m_tiles; // key = (int(type)*100 + variant)
    sf::RenderTexture m_rt; // 用于离屏渲染的临时缓冲

    // 绘制单个图块到 RenderTexture
    void RenderGround(sf::RenderTexture& rt, int variant);
    void RenderPath(sf::RenderTexture& rt, int variant);
    void RenderFloor(sf::RenderTexture& rt, int variant);
    void RenderWall(sf::RenderTexture& rt, int variant);
    void RenderWater(sf::RenderTexture& rt, int variant);
    void RenderTree(sf::RenderTexture& rt, int variant);
    void RenderDoor(sf::RenderTexture& rt, int variant);
    void RenderNPCSpot(sf::RenderTexture& rt, int variant);
    void RenderChest(sf::RenderTexture& rt, int variant);
    void RenderGrass(sf::RenderTexture& rt, int variant);
    void RenderFence(sf::RenderTexture& rt, int variant);

    static int MakeKey(TileType type, int variant) {
        return (static_cast<int>(type)) * 100 + variant;
    }

    // 辅助：画一个像素点（小矩形）
    static void Px(sf::RenderTarget& rt, float x, float y, const sf::Color& c,
                   float w = 1.f, float h = 1.f);
};
