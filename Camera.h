#pragma once
#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(int mapW, int mapH, int screenW = 800, int screenH = 600);

    void SetMapSize(int w, int h);

    // 设置目标位置（玩家像素坐标），直接瞬移到目标
    void SetTarget(float x, float y);

    // 平滑更新到目标位置
    void Update(float dt, float targetX, float targetY);

    // 获取视口左上角的世界坐标
    // 渲染公式: screenPos = worldPos - cameraOffset
    sf::Vector2f GetOffset() const { return m_position; }

    float GetX() const { return m_position.x; }
    float GetY() const { return m_position.y; }

private:
    int m_mapWidth, m_mapHeight;
    int m_screenWidth, m_screenHeight;

    sf::Vector2f m_position;     // 视口左上角对应的世界坐标
    float m_smoothSpeed = 12.f;  // 跟随平滑系数

    void ClampPosition();
};
