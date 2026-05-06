#pragma execution_character_set("utf-8")
#include "TileSet.h"
#include <cstdlib>

// ============================================================
//  TileSet 构造 — 预渲染所有图块
// ============================================================
TileSet::TileSet() {
    m_rt.create(TILE_SIZE, TILE_SIZE);
    GenerateAll();
}

void TileSet::GenerateAll() {
    // 每种类型生成 4 个变体（0~3）
    int variants[] = { 0, 1, 2, 3 };
    for (int v : variants) {
        m_rt.clear(sf::Color::Transparent);

        RenderGround(m_rt, v);
        m_tiles[MakeKey(TileType::Ground, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderPath(m_rt, v);
        m_tiles[MakeKey(TileType::Path, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderFloor(m_rt, v);
        m_tiles[MakeKey(TileType::Floor, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderWall(m_rt, v);
        m_tiles[MakeKey(TileType::Wall, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderWater(m_rt, v);
        m_tiles[MakeKey(TileType::Water, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderTree(m_rt, v);
        m_tiles[MakeKey(TileType::Tree, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderDoor(m_rt, v);
        m_tiles[MakeKey(TileType::Door, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderNPCSpot(m_rt, v);
        m_tiles[MakeKey(TileType::NPCSpot, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderChest(m_rt, v);
        m_tiles[MakeKey(TileType::Chest, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderGrass(m_rt, v);
        m_tiles[MakeKey(TileType::Grass, v)] = m_rt.getTexture();

        m_rt.clear(sf::Color::Transparent);
        RenderFence(m_rt, v);
        m_tiles[MakeKey(TileType::Fence, v)] = m_rt.getTexture();
    }
}

const sf::Texture& TileSet::GetTile(TileType type, int variant) const {
    static sf::Texture dummy; // 空纹理 fallback
    auto it = m_tiles.find(MakeKey(type, variant));
    if (it != m_tiles.end()) return it->second;
    it = m_tiles.find(MakeKey(type, 0));
    if (it != m_tiles.end()) return it->second;
    return dummy;
}

// ============================================================
//  辅助函数：画像素点
// ============================================================
void TileSet::Px(sf::RenderTarget& rt, float x, float y, const sf::Color& c,
                 float w, float h) {
    sf::RectangleShape rect(sf::Vector2f(w, h));
    rect.setPosition(x, y);
    rect.setFillColor(c);
    rt.draw(rect);
}

// ============================================================
//  草地 Ground — 绿色基调，随机草点装饰
// ============================================================
void TileSet::RenderGround(sf::RenderTexture& rt, int variant) {
    // 基底绿色
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60)); // #4C993C
    rt.draw(base);

    // 随机深浅色块增加质感（根据 variant 固定种子）
    srand(static_cast<unsigned>(variant * 137 + 42));
    for (int i = 0; i < 12; i++) {
        int px = rand() % TILE_SIZE;
        int py = rand() % TILE_SIZE;
        int shade = rand() % 3; // 0=深, 1=中, 2=亮
        if (shade == 0)
            Px(rt, px, py, sf::Color(56, 123, 44), 2, 2);
        else if (shade == 1)
            Px(rt, px, py, sf::Color(96, 173, 80), 2, 2);
        else
            Px(rt, px, py, sf::Color(120, 190, 100), 2, 2);

        // 偶尔加一根小草
        if (rand() % 5 == 0) {
            Px(rt, px, py - 1, sf::Color(50, 140, 35), 1, 3);
            Px(rt, px + 1, py - 2, sf::Color(55, 150, 40), 1, 3);
        }
    }
}

// ============================================================
//  泥土路 Path — 棕色调，有碎石纹理
// ============================================================
void TileSet::RenderPath(sf::RenderTexture& rt, int variant) {
    // 基底棕色
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(160, 130, 90)); // #A0825A
    rt.draw(base);

    srand(static_cast<unsigned>(variant * 211 + 17));
    for (int i = 0; i < 15; i++) {
        int px = rand() % TILE_SIZE;
        int py = rand() % TILE_SIZE;
        int c = rand() % 100 - 50; // 色偏移
        Px(rt, px, py,
           sf::Color(
               std::min(255, std::max(0, 160 + c)),
               std::min(255, std::max(0, 130 + c)),
               std::min(255, std::max(0, 70 + c / 2))),
           2 + rand() % 3, 2 + rand() % 3);
    }
}

// ============================================================
//  石地板 Floor — 室内灰色砖块
// ============================================================
void TileSet::RenderFloor(sf::RenderTexture& rt, int variant) {
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(180, 175, 165)); // #B4AFA5
    rt.draw(base);

    // 砖缝网格
    sf::Color seam(150, 145, 135);
    // 中横线
    Px(rt, 0, 15.5f, seam, 32.f, 1.f);
    // 中竖线
    Px(rt, 15.5f, 0, seam, 1.f, 16.f);
    Px(rt, 15.5f, 16, seam, 1.f, 16.f);

    // 四个象限的微调色差
    Px(rt, 2, 2, sf::Color(170, 165, 155), 12, 12);
    Px(rt, 18, 2, sf::Color(188, 183, 173), 12, 12);
    Px(rt, 2, 18, sf::Color(188, 183, 173), 12, 12);
    Px(rt, 18, 18, sf::Color(170, 165, 155), 12, 12);
}

// ============================================================
//  石墙 Wall — 灰色砖墙，带阴影
// ============================================================
void TileSet::RenderWall(sf::RenderTexture& rt, int variant) {
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(110, 105, 100)); // #6E6964
    rt.draw(base);

    // 砖缝
    sf::Color dark(85, 80, 75);
    sf::Color light(135, 130, 125);

    // 横向砖缝
    Px(rt, 0, 7, dark, 32, 1);
    Px(rt, 0, 15, dark, 32, 1);
    Px(rt, 0, 23, dark, 32, 1);

    // 纵向砖缝（交错）
    Px(rt, 10, 0, dark, 1, 8);
    Px(rt, 22, 0, dark, 1, 8);
    Px(rt, 4, 8, dark, 1, 8);
    Px(rt, 16, 8, dark, 1, 8);
    Px(rt, 28, 8, dark, 1, 8);
    Px(rt, 10, 16, dark, 1, 8);
    Px(rt, 22, 16, dark, 1, 8);
    Px(rt, 4, 24, dark, 1, 8);
    Px(rt, 16, 24, dark, 1, 8);
   Px(rt, 28, 24, dark, 1, 8);

    // 高光边
    Px(rt, 0, 0, light, 32, 1);
    Px(rt, 0, 0, light, 1, 32);
}

// ============================================================
//  水 Water — 动态蓝绿色
// ============================================================
void TileSet::RenderWater(sf::RenderTexture& rt, int variant) {
    sf::Color deepBlue(40, 90, 150);   // #285A96
    sf::Color midBlue(60, 130, 180);   // #3C82B4
    sf::Color lightBlue(100, 170, 210); // #64AAD2

    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(midBlue);
    rt.draw(base);

    srand(static_cast<unsigned>(variant * 73 + 99));
    // 波纹
    for (int i = 0; i < 20; i++) {
        int px = rand() % 30;
        int py = rand() % 30;
        int len = 3 + rand() % 8;
        bool horiz = rand() % 2 == 0;
        sf::Color c = (rand() % 2 == 0) ? lightBlue : deepBlue;
        if (horiz)
            Px(rt, px, py, c, (float)len, 1.f);
        else
            Px(rt, px, py, c, 1.f, (float)len);
    }

    // 高光反射
    Px(rt, 4 + variant, 4, sf::Color(180, 220, 255, 200), 6, 2);
    Px(rt, 20 - variant, 14, sf::Color(180, 220, 255, 150), 4, 1);
}

// ============================================================
//  树 Tree — 深绿树冠+棕色树干（正常朝上）
// ============================================================
void TileSet::RenderTree(sf::RenderTexture& rt, int variant) {
    // 先画草地底层
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60));
    rt.draw(base);

    // 树干（偏下居中）
    Px(rt, 13, 20, sf::Color(120, 80, 40), 6, 9);   // 主干
    Px(rt, 12, 25, sf::Color(100, 65, 30), 8, 4);   // 底座加宽

    // 树冠颜色
    sf::Color leafDark(30, 110, 35);    // 深绿
    sf::Color leafMid(45, 140, 45);     // 中绿
    sf::Color leafLight(65, 170, 60);   // 亮绿

    // === 上层小冠（树顶）===
    for (int dy = 1; dy <= 10; dy++)
        for (int dx = 8; dx <= 23; dx++) {
            float cx = 16.f, cy = 5.f;
            float dist = sqrtf((dx - cx)*(dx - cx) + (dy - cy)*(dy - cy));
            if (dist <= 6.f && dist > 4.f)
                Px(rt, dx, dy, leafDark, 1, 1);
            else if (dist <= 4.f && dist > 2.f)
               Px(rt, dx, dy, leafMid, 1, 1);
            else if (dist <= 2.f)
                Px(rt, dx, dy, leafLight, 1, 1);
        }

    // === 中层主冠（最大最圆）===
    for (int dy = 6; dy <= 18; dy++)
        for (int dx = 3; dx <= 28; dx++) {
            float cx = 16.f, cy = 11.f;
            float dist = sqrtf((dx - cx)*(dx - cx) + (dy - cy)*(dy - cy));
            if (dist <= 10.f && dist > 7.f)
                Px(rt, dx, dy, leafDark, 1, 1);
            else if (dist <= 7.f && dist > 4.f)
                Px(rt, dx, dy, leafMid, 1, 1);
            else if (dist <= 4.f)
                Px(rt, dx, dy, leafLight, 1, 1);
        }

    // === 下层冠（连接树干）===
    for (int dy = 14; dy <= 21; dy++)
        for (int dx = 7; dx <= 24; dx++) {
            float cx = 16.f, cy = 18.f;
            float dist = sqrtf((dx - cx)*(dx - cx) + (dy - cy)*(dy - cy));
            if (dist <= 6.f && dist > 4.f)
                Px(rt, dx, dy, leafDark, 1, 1);
            else if (dist <= 4.f)
                Px(rt, dx, dy, leafMid, 1, 1);
        }

    // 树顶高光
    Px(rt, 15, 0, leafLight, 3, 2);
    Px(rt, 14, 0, leafMid, 5, 1);
}

// ============================================================
//  门 Door — 木门/传送门
// ============================================================
void TileSet::RenderDoor(sf::RenderTexture& rt, int variant) {
    // 地面底
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60));
    rt.draw(base);

    // 门框
    Px(rt, 6, 2, sf::Color(100, 55, 25), 20, 28); // 门板底色

    // 门板木纹
    Px(rt, 8, 4, sf::Color(130, 75, 35), 16, 11);  // 上半门板
    Px(rt, 8, 17, sf::Color(130, 75, 35), 16, 11); // 下半门板

    // 门框边缘
    Px(rt, 6, 2, sf::Color(80, 42, 18), 20, 1);     // 顶边
    Px(rt, 6, 29, sf::Color(80, 42, 18), 20, 1);     // 底边
    Px(rt, 6, 2, sf::Color(80, 42, 18), 1, 28);      // 左边
    Px(rt, 25, 2, sf::Color(80, 42, 18), 1, 28);     // 右边

    // 门把手（金色圆）
    Px(rt, 23, 15, sf::Color(220, 185, 50), 2, 2);
    Px(rt, 23, 15, sf::Color(255, 230, 100), 1, 1);   // 高光

    // 门发光效果提示传送
    if (variant >= 2) {
        // 发光边框
        Px(rt, 5, 1, sf::Color(100, 180, 255, 150), 22, 1);
        Px(rt, 5, 30, sf::Color(100, 180, 255, 150), 22, 1);
        Px(rt, 5, 1, sf::Color(100, 180, 255, 150), 1, 30);
        Px(rt, 27, 1, sf::Color(100, 180, 255, 150), 1, 30);
    }
}

// ============================================================
//  NPC站位 NPCSpot — 特殊地面标记
// ============================================================
void TileSet::RenderNPCSpot(sf::RenderTexture& rt, int variant) {
    // 正常草地
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60));
    rt.draw(base);

    // 圆形标记（暗示有人站这里）
    sf::Color mark(255, 255, 200, 100);
    int cx = 16, cy = 24;
    for (int dy = -6; dy <= 6; dy++)
        for (int dx = -6; dx <= 6; dx++) {
            float d = sqrtf(float(dx*dx + dy*dy));
            if (d <= 6.f && d >= 4.f)
                Px(rt, cx + dx, cy + dy, mark, 1, 1);
        }
}

// ============================================================
//  宝箱 Chest — 金黄色宝箱
// ============================================================
void TileSet::RenderChest(sf::RenderTexture& rt, int variant) {
    // 地面
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60));
    rt.draw(base);

    // 箱体
    Px(rt, 8, 18, sf::Color(180, 130, 40), 16, 12);  // 箱身
    Px(rt, 7, 14, sf::Color(200, 150, 50), 18, 5);    // 箱盖
    Px(rt, 7, 13, sf::Color(160, 115, 30), 18, 1);    // 盖沿

    // 金属装饰条
    Px(rt, 7, 19, sf::Color(215, 185, 80), 16, 1);    // 中横条
    Px(rt, 7, 27, sf::Color(215, 185, 80), 16, 1);    // 底横条
    Px(rt, 14, 14, sf::Color(215, 185, 80), 4, 15);    // 中竖条（锁扣）

    // 锁
    Px(rt, 15, 21, sf::Color(100, 80, 30), 2, 3);

    // 开启状态（variant >= 2 时盖子打开）
    if (variant >= 2) {
        // 盖子打开效果
        Px(rt, 8, 8, sf::Color(200, 150, 50), 18, 5);
    } else {
        // 关闭状态高光
        Px(rt, 9, 15, sf::Color(230, 190, 80), 16, 1);
    }
}

// ============================================================
//  高草 Grass — 长草（遇敌触发区）
// ============================================================
void TileSet::RenderGrass(sf::RenderTexture& rt, int variant) {
    // 草地基底
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(68, 145, 52)); // 比 Ground 更深绿
    rt.draw(base);

    // 密集的长草
    srand(static_cast<unsigned>(variant * 311 + 7));
    sf::Color grassLight(80, 170, 55);
    sf::Color grassMid(55, 140, 38);
    sf::Color grassDark(35, 115, 25);

    for (int i = 0; i < 25; i++) {
        int px = rand() % 30;
        int py = rand() % 30;
        int h = 3 + rand() % 6; // 草高
        sf::Color c = (i % 3 == 0) ? grassLight : ((i % 3 == 1) ? grassMid : grassDark);

        for (int j = 0; j < h; j++) {
            int offset = (j < h / 2) ? 0 : ((j % 2 == 0) ? 1 : -1);
            Px(rt, px + offset, py - j, c, 1, 1);
        }
    }
}

// ============================================================
//  栅栏 Fence — 木栅栏
// ============================================================
void TileSet::RenderFence(sf::RenderTexture& rt, int variant) {
    // 地面
    sf::RectangleShape base(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    base.setFillColor(sf::Color(76, 153, 60));
    rt.draw(base);

    // 横栏两根
    Px(rt, 0, 10, sf::Color(140, 100, 50), 32, 3);
    Px(rt, 0, 22, sf::Color(140, 100, 50), 32, 3);

    // 竖桩
    for (int x = 2; x < 32; x += 8) {
        Px(rt, x, 4, sf::Color(120, 82, 38), 3, 27);  // 桩
        Px(rt, x, 4, sf::Color(155, 115, 62), 1, 27); // 高光
        Px(rt, x, 4, sf::Color(100, 65, 28), 3, 2);    // 桩顶削尖
    }

    // 横栏高光
    Px(rt, 0, 10, sf::Color(170, 125, 65), 32, 1);
    Px(rt, 0, 22, sf::Color(170, 125, 65), 32, 1);
}
