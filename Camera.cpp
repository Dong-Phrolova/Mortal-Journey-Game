#include "Camera.h"
#include "TileSet.h"
#include <cmath>

Camera::Camera(int mapW, int mapH, int screenW, int screenH)
    : m_mapWidth(mapW), m_mapHeight(mapH)
    , m_screenWidth(screenW), m_screenHeight(screenH)
{
    m_position = sf::Vector2f(0.f, 0.f);
}

void Camera::SetMapSize(int w, int h) {
    m_mapWidth = w;
    m_mapHeight = h;
}

void Camera::SetTarget(float x, float y) {
    // 让目标居中于屏幕：视口左上角 = 目标中心 - 屏幕半尺寸
    float targetCamX = x - m_screenWidth / 2.f;
    float targetCamY = y - m_screenHeight / 2.f;
    m_position.x = targetCamX;
    m_position.y = targetCamY;
    ClampPosition();
}

void Camera::Update(float dt, float targetX, float targetY) {
    // 目标视口位置（让玩家居中）
    float targetCamX = targetX - m_screenWidth / 2.f;
    float targetCamY = targetY - m_screenHeight / 2.f;

    // 平滑插值（基于时间的 Lerp，帧率无关）
    float factor = 1.f - std::pow(1.f - m_smoothSpeed * dt, 1.f);
    factor = std::min(1.f, factor);

    m_position.x += (targetCamX - m_position.x) * factor;
    m_position.y += (targetCamY - m_position.y) * factor;

    ClampPosition();
}

void Camera::ClampPosition() {
    int mapPixelW = m_mapWidth * TileSet::TILE_SIZE;
    int mapPixelH = m_mapHeight * TileSet::TILE_SIZE;

    // 当地图比屏幕小时，让地图居中显示
    // 此时 camera position 为负值（地图向右/下偏移）
    if (mapPixelW <= m_screenWidth) {
        m_position.x = (float)(mapPixelW - m_screenWidth) / 2.f; // 负值或零
    } else {
        // 左边界
        if (m_position.x < 0)
            m_position.x = 0;
        // 右边界
        if (m_position.x > mapPixelW - m_screenWidth)
            m_position.x = (float)(mapPixelW - m_screenWidth);
    }

    if (mapPixelH <= m_screenHeight) {
        m_position.y = (float)(mapPixelH - m_screenHeight) / 2.f; // 负值或零
    } else {
        // 上边界
        if (m_position.y < 0)
            m_position.y = 0;
        // 下边界
        if (m_position.y > mapPixelH - m_screenHeight)
            m_position.y = (float)(mapPixelH - m_screenHeight);
    }
}
