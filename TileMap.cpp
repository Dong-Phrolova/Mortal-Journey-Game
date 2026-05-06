#pragma execution_character_set("utf-8")
#include "TileMap.h"
#include <cmath>

// ============================================================
//  TileMap 构造/加载
// ============================================================
TileMap::TileMap() {}

void TileMap::LoadMap(const MapData* mapData) {
    m_mapData = mapData;
}

// ============================================================
//  碰撞检测
// ============================================================
bool TileMap::IsWalkable(int x, int y) const {
    if (!m_mapData || !m_mapData->InBounds(x, y)) return false;
    return IsTileWalkable(m_mapData->tiles[y][x].type);
}

TileType TileMap::GetTileType(int x, int y) const {
    if (!m_mapData || !m_mapData->InBounds(x, y)) return TileType::Wall;
    return m_mapData->tiles[y][x].type;
}

const Tile* TileMap::GetDoorAt(int x, int y) const {
    if (!m_mapData || !m_mapData->InBounds(x, y)) return nullptr;
    const auto& t = m_mapData->tiles[y][x];
    if (t.type == TileType::Door)
        return &t;
    return nullptr;
}

const MapNPC* TileMap::GetNPCAt(int x, int y) const {
    for (const auto& npc : m_npcs) {
        if (npc.mapX == x && npc.mapY == y)
            return &npc;
    }
    return nullptr;
}

// ============================================================
//  NPC 管理
// ============================================================
void TileMap::AddNPC(const MapNPC& npc) {
    m_npcs.push_back(npc);
}

void TileMap::ClearNPCs() {
    m_npcs.clear();
}

void TileMap::RemoveNPC(int id) {
    for (auto it = m_npcs.begin(); it != m_npcs.end(); ++it) {
        if (it->id == id) {
            m_npcs.erase(it);
            break;
        }
    }
}

// ============================================================
//  渲染地图
// ============================================================
void TileMap::Render(sf::RenderTarget& target, const sf::Vector2f& cameraOffset) {
    if (!m_mapData) return;

    int ts = TileSet::TILE_SIZE;
    // cameraOffset 是视口左上角的世界坐标
    // 屏幕坐标 = 世界坐标 - cameraOffset
    float offX = -cameraOffset.x;
    float offY = -cameraOffset.y;

    // 计算可见范围（只渲染屏幕内的瓦片，优化性能）
    int startX = std::max(0, (int)(-offX / ts));
    int startY = std::max(0, (int)(-offY / ts));
    int endX = std::min(m_mapData->width,
                        (int)((target.getSize().x - offX) / ts) + 1);
    int endY = std::min(m_mapData->height,
                        (int)((target.getSize().y - offY) / ts) + 1);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            DrawTile(target, m_mapData->tiles[y][x],
                     (float)(x * ts) + offX,
                     (float)(y * ts) + offY);
        }
    }
}

void TileMap::DrawTile(sf::RenderTarget& target, const Tile& tile,
                        int screenX, int screenY) {
    int typeIdx = static_cast<int>(tile.type);
    if (typeIdx < 0 || typeIdx >= static_cast<int>(TileType::COUNT)) return;

    sf::Sprite sprite(m_tileset.GetTile(tile.type, tile.variant));
    sprite.setPosition((float)screenX, (float)screenY);
    target.draw(sprite);
}

// ============================================================
//  NPC 更新与渲染
// ============================================================
void TileMap::UpdateNPCs(float dt) {
    // 预留：后续可做 NPC 自动走动动画
}

void TileMap::RenderNPCs(sf::RenderTarget& target, const sf::Vector2f& cameraOffset,
                         float /*deltaTime*/) {
    for (const auto& npc : m_npcs) {
        // cameraOffset 已是负值（-camX, -camY），直接加即可
        int screenX = npc.mapX * TileSet::TILE_SIZE + (int)cameraOffset.x;
        int screenY = npc.mapY * TileSet::TILE_SIZE + (int)cameraOffset.y;
        DrawNPCSprite(target, npc, screenX, screenY);
    }
}

// 像素风格 NPC 绘制（类似 PlayerSprite 但更简单）
void TileMap::DrawNPCSprite(sf::RenderTarget& target, const MapNPC& npc,
                            int screenX, int screenY) {
    int ts = TileSet::TILE_SIZE;
    // 角色偏移：角色比格子小，居中偏下
    int ox = screenX + ts / 4;
    int oy = screenY + 4;

    sf::Color mainColor = npc.color;       // 衣服主色
    sf::Color skinColor(255, 215, 170);   // 肤色
    sf::Color hairColor(40, 30, 20);      // 深棕头发
    sf::Color darkShade(mainColor.r * 7 / 10,
                       mainColor.g * 7 / 10,
                       mainColor.b * 7 / 10);  // 暗部

    // 阴影
    sf::RectangleShape shadow(sf::Vector2f(16.f, 3.f));
    shadow.setPosition((float)(ox + 2), (float)(oy + 25));
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    target.draw(shadow);

    // 身体 (12x14)
    sf::RectangleShape body(sf::Vector2f(12.f, 14.f));
    body.setPosition((float)(ox + 2), (float)(oy + 11));
    body.setFillColor(mainColor);
    target.draw(body);

    // 身体暗部（右侧）
    sf::RectangleShape bodyDark(sf::Vector2f(4.f, 14.f));
    bodyDark.setPosition((float)(ox + 10), (float)(oy + 11));
    bodyDark.setFillColor(darkShade);
    target.draw(bodyDark);

    // 头 (10x9)
    sf::RectangleShape head(sf::Vector2f(10.f, 9.f));
    head.setPosition((float)(ox + 3), (float)(oy + 2));
    head.setFillColor(skinColor);
    target.draw(head);

    // 头发
    sf::RectangleShape hair(sf::Vector2f(10.f, 5.f));
    hair.setPosition((float)(ox + 3), (float)(oy));
    hair.setFillColor(hairColor);
    target.draw(hair);

    // 眼睛（两个像素点）
    sf::RectangleShape eye(sf::Vector2f(1.5f, 1.5f));
    eye.setFillColor(sf::Color(20, 15, 10));

    switch (npc.faceDir) {
    case 0: // 下 - 正面
        eye.setPosition((float)(ox + 5), (float)(oy + 6)); target.draw(eye);
        eye.setPosition((float)(ox + 9), (float)(oy + 6)); target.draw(eye);
        break;
    case 1: // 上 - 背面（看不到脸）
        break;
    case 2: // 左侧
        eye.setPosition((float)(ox + 4), (float)(oy + 6)); target.draw(eye);
        break;
    case 3: // 右侧
        eye.setPosition((float)(ox + 9), (float)(oy + 6)); target.draw(eye);
        break;
    }

    // 名字标签
    static sf::Font font;
    // 使用系统字体会在后面统一处理，这里先用简单方式跳过名字渲染
}
