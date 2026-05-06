#pragma once
#pragma execution_character_set("utf-8")
#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// 瓦片类型（改名为 SimpleMapTile，避免与 TileSet.h 的 TileType 冲突）
enum SimpleMapTile : int {
    Grass  = 0,   // 草地 (可走)
    Path   = 1,   // 土路 (可走)
    Wall   = 2,   // 墙壁 (不可走)
    Water  = 3,   // 水面 (不可走)
    Floor  = 4,   // 室内地板 (可走)
    Door   = 5,   // 门框 (可走，特殊标记)
    Wood   = 6,   // 木制家具/柜台 (不可走)
    Bridge = 7,   // 小桥 (可走)
    Flower = 8,   // 花丛装饰
    Rock   = 9,   // 石头装饰
    Fence  = 10   // 栅栏/篱笆 (不可走)
};

// 方向枚举
enum class Direction { Down, Left, Right, Up };

// ============ NPC 结构体（青牛镇村民）============
struct NPC {
    std::wstring nameW;               // NPC 名字(宽字符，解决乱码)
    std::string name;                  // ASCII 兼容名(内部标识)
    int tileX, tileY;                 // 瓦片坐标
    int npcType;                      // NPC外观类型: 0=村民男,1=村民女,2=胖子,3=商人,4=铁匠,5=长者
    sf::Color hairColor;              // 头发颜色
    sf::Color clothesColor;           // 衣服颜色
    sf::Color skinColor;              // 肤色
    std::vector<std::wstring> dialog; // 对话内容

    void Render(sf::RenderTarget& target) const;
    void RenderNameTag(sf::RenderTarget& target, const sf::Font& font) const;
    const std::wstring& GetDialog(int idx) const;
    int GetDialogCount() const { return (int)dialog.size(); }
};

// ============ 简化地图状态（独立于 WorldMapState）============

// 地图尺寸常量
static constexpr int MAP_W = 30;       // 地图宽度(瓦片数)
static constexpr int MAP_H = 25;       // 地图高度(瓦片数)
static constexpr int TILE_SIZE = 32;   // 每个瓦片像素
static constexpr float SPEED = 2.7f;    // 每帧移动像素数（与主地图160px/s一致）

class SimpleMapState : public GameState {
public:
    SimpleMapState();
    ~SimpleMapState() override = default;

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override;
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    // ============ SimpleMap（内部地图）============
    class SimpleMap {
    public:
        int m_data[MAP_H][MAP_W];  // [行][列] = m_data[y][x]

        SimpleMap();

        // 渲染整个地图（精细纹理版）
        void Render(sf::RenderTarget& target) const;

        // 碰撞查询：某瓦片坐标是否可通行
        bool IsWalkable(int tileX, int tileY) const;

        // 像素坐标转瓦片坐标
        static int PixelToTile(float pixel) { return (int)(pixel / TILE_SIZE); }
    };

    // ============ SimplePlayer（内部玩家 - 韩立）============
    class SimplePlayer {
    public:
        float x, y;           // 像素位置（左上角）
        Direction dir;
        float animTimer;      // 行走动画计时器
        int animFrame;        // 动画帧 (0/1)

        SimplePlayer();

        void TryMove(float dx, float dy, const SimpleMap& map);
        void SetPosition(int tileX, int tileY);
        void UpdateAnim(float dt);
        void Render(sf::RenderTarget& target) const;
    };

    // ============ 成员变量 ============
    SimpleMap      m_map;
    SimplePlayer   m_player;
    sf::View       m_cameraView;
    sf::Font       m_font;

    bool m_showHint = true;
    float m_hintTimer = 0.f;

    // ============ NPC 系统 ============
    std::vector<NPC> m_npcs;
    bool            m_showDialog;
    std::wstring    m_dialogText;
    int             m_dialogIndex;
    int             m_activeNpcIndex;
};
