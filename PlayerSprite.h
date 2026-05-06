#pragma once
#include <SFML/Graphics.hpp>

// 角色朝向
enum class Dir : int {
    Down = 0,
    Up = 1,
    Left = 2,
    Right = 3
};

// 玩家像素角色精灵（程序化绘制，4方向 + 行走动画）
class PlayerSprite {
public:
    static const int SPRITE_W = 16; // 角色宽度(像素)
    static const int SPRITE_H = 24; // 角色高度(像素)

    PlayerSprite();

    // 设置位置（瓦片坐标 -> 自动转为像素坐标）
    void SetTilePosition(int tileX, int tileY);

    // 获取当前位置的瓦片坐标
    int GetTileX() const { return m_tileX; }
    int GetTileY() const { return m_tileY; }

    // 移动动画：目标格子位置
    void MoveTo(int tileX, int tileY);
    bool IsMoving() const { return m_moving; }

    // 设置方向
    void SetDirection(Dir dir) { m_dir = dir; }
    Dir GetDirection() const { return m_dir; }

    // 每帧更新（处理移动插值、动画帧）
    void Update(float dt);

    // 渲染到 target（传入 cameraOffset）
    void Render(sf::RenderTarget& target, float cameraX, float cameraY);

    // 实际绘制角色（公开给其他状态使用，如SimpleMapState）
    void DrawCharacter(sf::RenderTarget& target, float px, float py);

    // 获取实际像素位置
    sf::Vector2f GetPixelPos() const { return m_pixelPos; }

private:
    int m_tileX = 1, m_tileY = 1;   // 当前格子坐标
    sf::Vector2f m_pixelPos;         // 当前像素位置（用于平滑移动）
    Dir m_dir = Dir::Down;
    bool m_moving = false;

    // 动画参数
    float m_animTimer = 0.f;         // 动画计时器
    int m_animFrame = 0;             // 0=站立, 1=迈步左, 2=迈步右
    float m_moveSpeed = 160.f;       // 像素/秒 的移动速度

    // 移动插值
    sf::Vector2f m_moveFrom;         // 起点
    sf::Vector2f m_moveTo;           // 终点
    float m_moveProgress = 0.f;      // 0~1

    // 预渲染角色纹理（避免每帧重建矩形）
    void PreRenderCharacter();
    std::vector<sf::Texture> m_charTextures; // [dir*3+frame] 共12帧
};
