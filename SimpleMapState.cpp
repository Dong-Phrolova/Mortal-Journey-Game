#pragma execution_character_set("utf-8")
#include "SimpleMapState.h"
#include "GameCallbacks.h"
#include "MainMenuState.h"
#include "WorldMapState.h"
#include "SettingsState.h"
#include "GameState.h"
#include "TileSet.h"
#include "PlayerSprite.h"
#include "QuestSystem.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

// ============================================================
//  SimpleMapState 构造/进入
// ============================================================

SimpleMapState::SimpleMapState()
    : GameState()
    , m_map()
    , m_player()
    , m_cameraView(sf::FloatRect(0, 0, 800, 600))
{
    m_type = GameStateType::WorldMap;
}

void SimpleMapState::Enter() {
    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");
    m_showHint = true;
    m_hintTimer = 5.f;

    m_player.SetPosition(6, 19);
    m_player.dir = Direction::Down;

    // ===== 初始化青牛镇 NPCs (使用 wstring 名字，解决乱码) =====
    m_npcs.clear();
    m_showDialog = false;
    m_dialogIndex = 0;
    m_activeNpcIndex = -1;

    // 韩胖子（圆胖体型）
    m_npcs.push_back({
        L"韩胖子", "fatguy", 8, 18, 2,
        sf::Color(30, 20, 10),
        sf::Color(180, 120, 50),
        sf::Color(255, 220, 180),
        { L"韩立！要不要去后山掏鸟窝？", L"我爹说城里来了江湖骗子，让别去。", L"你最近怎么老往山里跑？" }
    });

    // 春香（酒楼老板娘）
    m_npcs.push_back({
        L"春香", "chunxiang", 14, 7, 1,
        sf::Color(20, 10, 5),
        sf::Color(200, 60, 80),
        sf::Color(255, 210, 175),
        { L"客官要不要来壶酒？", L"我们酒楼的桂花酿可是整条街最好的！", L"小女子春香，客官有事尽管说~" }
    });

    // 杂货铺掌柜
    m_npcs.push_back({
        L"掌柜", "shopkeeper", 24, 18, 0,
        sf::Color(50, 35, 20),
        sf::Color(100, 100, 160),
        sf::Color(240, 210, 170),
        { L"小兄弟，看看需要什么？", L"这符纸可是从城里来的货，贵了点但好用。", L"最近货不多，再过半月才有新货。" }
    });

    // 铁匠
    m_npcs.push_back({
        L"铁匠", "blacksmith", 5, 5, 4,
        sf::Color(25, 15, 8),
        sf::Color(140, 100, 50),
        sf::Color(210, 170, 130),
        { L"打把刀吗？材料自己带。", L"韩家小子又来了？没钱打什么铁！", L"这把刀用了好钢，砍柴都嫌浪费。" }
    });

    // 三叔 — 春香酒楼掌柜（带韩立来七玄门的人）
    m_npcs.push_back({
        L"三叔", "uncle", 5, 16, 5,
        sf::Color(60, 45, 25),
        sf::Color(60, 110, 60),
        sf::Color(235, 200, 160),
        { L"立儿，今晚去后山别让人看见了。", L"这瓶药膏你拿着，练功受伤了能用上。", L"我教你的吐纳法，每天至少练两个时辰。" }
    });

    // 三叔 — 春香酒楼掌柜（带韩立来七玄门的人）
    m_npcs.push_back({
        L"三叔", "uncle", 5, 16, 5,
        sf::Color(60, 45, 25),
        sf::Color(60, 110, 60),
        sf::Color(235, 200, 160),
        { L"立儿，你还记得小时候吗？你家那条件，七个娃一张嘴，你爹娘也难啊...",
          L"这次七玄门招内门弟子，五年一次的好机会！我可是好不容易才给你报上名。",
          L"江湖门派的日子不比村里，凡事多留个心眼，不行就回来，三叔还能亏待你不成？" }
    });

    // 秘籍商人
    m_npcs.push_back({
        L"秘籍商人", "technique_merchant", 15, 7, 3,
        sf::Color(40, 25, 15),
        sf::Color(140, 90, 140),
        sf::Color(230, 195, 155),
        { L"客官，想学点什么功法？", L"我这有上好的修仙秘籍，价格公道。", L"长春功是墨大夫那老头的入门功法，我这里免费送你一本。" }
    });

    // 装备店老板
    m_npcs.push_back({
        L"装备商", "equip_merchant", 25, 17, 3,
        sf::Color(35, 22, 12),
        sf::Color(150, 100, 50),
        sf::Color(235, 200, 160),
        { L"想要好装备？来对了地方！", L"一把好剑胜过十年苦练啊，哈哈。", L"看看有没有适合你的。" }
    });
}

// ============================================================
//  SimpleMap 实现 - 青牛镇地图
// ============================================================

SimpleMapState::SimpleMap::SimpleMap() {
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            m_data[y][x] = Grass;

    // 四周边界墙
    for (int i = 0; i < MAP_W; ++i) { m_data[0][i] = Wall; m_data[MAP_H-1][i] = Wall; }
    for (int i = 0; i < MAP_H; ++i) { m_data[i][0] = Wall; m_data[i][MAP_W-1] = Wall; }

    // 镇门
    m_data[12][0] = Door; m_data[12][1] = Path;
    m_data[12][MAP_W-1] = Door; m_data[12][MAP_W-2] = Path;

    // 主街道
    for (int x = 1; x < MAP_W-1; ++x)
        if (m_data[12][x] != Door) m_data[12][x] = Path;

    // 小河
    for (int y = 3; y <= 11; ++y) m_data[y][20] = Water;

    // 小桥
    m_data[7][19] = Bridge; m_data[7][20] = Bridge; m_data[7][21] = Bridge;
    m_data[8][19] = Bridge; m_data[8][20] = Bridge; m_data[8][21] = Bridge;
    for (int y = 9; y <= 12; ++y) m_data[y][20] = Path;

    // 春香酒楼 (x=11~18, y=3~9)
    for (int x = 11; x <= 18; ++x) { m_data[3][x] = Wall; m_data[9][x] = Wall; }
    for (int y = 3; y <= 9; ++y) { m_data[y][11] = Wall; m_data[y][18] = Wall; }
    for (int y = 4; y <= 8; ++y) for (int x = 12; x <= 17; ++x) m_data[y][x] = Floor;
    m_data[9][14] = Door; m_data[9][15] = Door;
    m_data[5][12] = Wood; m_data[5][13] = Wood; m_data[5][14] = Wood;
    m_data[5][15] = Wood; m_data[5][16] = Wood; m_data[5][17] = Wood;
    for (int y = 10; y <= 12; ++y) { m_data[y][14] = Path; m_data[y][15] = Path; }

    // 韩家小院 (x=2~8, y=14~21)
    for (int x = 2; x <= 8; ++x) { m_data[14][x] = Wall; m_data[21][x] = Wall; }
    for (int y = 14; y <= 21; ++y) { m_data[y][2] = Wall; m_data[y][8] = Wall; }
    for (int y = 15; y <= 20; ++y) for (int x = 3; x <= 7; ++x) m_data[y][x] = Floor;
    m_data[17][3] = Wall; m_data[17][4] = Wall; m_data[17][5] = Wall;
    m_data[17][6] = Wall; m_data[17][7] = Wall;
    for (int y = 15; y <= 16; ++y) for (int x = 3; x <= 7; ++x) m_data[y][x] = Floor;
    m_data[18][8] = Door;
    for (int y = 18; y >= 12; --y) { m_data[y][7] = Path; m_data[y][6] = Path; m_data[y][5] = Path; }

    // 杂货铺 (x=22~27, y=15~20)
    for (int x = 22; x <= 27; ++x) { m_data[15][x] = Wall; m_data[20][x] = Wall; }
    for (int y = 15; y <= 20; ++y) { m_data[y][22] = Wall; m_data[y][27] = Wall; }
    for (int y = 16; y <= 19; ++y) for (int x = 23; x <= 26; ++x) m_data[y][x] = Floor;
    m_data[20][24] = Door; m_data[20][25] = Door;
    for (int y = 21; y <= 22; ++y) { m_data[y][24] = Path; m_data[y][25] = Path; }

    // 铁匠铺 (x=3~7, y=3~7)
    for (int x = 3; x <= 7; ++x) { m_data[3][x] = Wall; m_data[7][x] = Wall; }
    for (int y = 3; y <= 7; ++y) { m_data[y][3] = Wall; m_data[y][7] = Wall; }
    for (int y = 4; y <= 6; ++y) for (int x = 4; x <= 6; ++x) m_data[y][x] = Floor;
    m_data[5][5] = Wood;
    m_data[7][5] = Door;
    for (int y = 8; y <= 12; ++y) m_data[y][5] = Path;

    // 装饰性元素
    m_data[2][16] = Flower;  m_data[2][17] = Flower;
    m_data[22][10] = Flower;  m_data[23][10] = Flower;
    m_data[10][25] = Flower;  m_data[11][26] = Flower;
    m_data[22][18] = Flower;  m_data[22][19] = Flower;

    // 花丛点缀
    m_data[13][9] = Flower;  m_data[13][10] = Flower;  m_data[14][10] = Flower;
    m_data[13][20] = Flower; m_data[13][21] = Flower;

    // 石头装饰
    m_data[6][10] = Rock;    m_data[7][11] = Rock;

    // 篱笆
    m_data[19][2] = Fence;   m_data[19][3] = Fence;
}

// ============================================================
//  精细地图渲染系统
// ============================================================

void SimpleMapState::SimpleMap::Render(sf::RenderTarget& target) const {
    static TileSet s_tileSet;  // 使用与主地图相同的TileSet纹理

    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            int type = m_data[y][x];
            float tx = (float)(x * TILE_SIZE);
            float ty = (float)(y * TILE_SIZE);
            auto seedRand = [](int a, int b, int mod) -> int {
                return ((a * 73 + b * 137 + a * b * 41) % mod);
            };

            sf::Sprite tileSprite;

            switch (type) {

            case Grass:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Grass, seedRand(x, y, 3)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Path:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Path, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Wall:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Wall, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Water:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Water, seedRand(x, y, 3)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Floor:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Floor, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Door:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Door, 0));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Wood:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Floor, 2));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Bridge:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Path, 2));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Flower:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Tree, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Rock:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Ground, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            case Fence:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Fence, seedRand(x, y, 2)));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;

            default:
                tileSprite.setTexture(s_tileSet.GetTile(::TileType::Grass, 0));
                tileSprite.setPosition(tx, ty);
                target.draw(tileSprite);
                break;
            }
        }
    }
}

bool SimpleMapState::SimpleMap::IsWalkable(int tileX, int tileY) const {
    if (tileX < 0 || tileX >= MAP_W || tileY < 0 || tileY >= MAP_H) return false;
    int type = m_data[tileY][tileX];
    return type != Wall && type != Water && type != Wood
        && type != Flower && type != Rock && type != Fence;
}

// ============================================================
//  SimplePlayer - 韩立角色精灵（精细版）
// ============================================================

SimpleMapState::SimplePlayer::SimplePlayer()
    : x(0), y(0), dir(Direction::Down), animTimer(0), animFrame(0)
{
}

void SimpleMapState::SimplePlayer::SetPosition(int tileX, int tileY) {
    x = (float)(tileX * TILE_SIZE);
    y = (float)(tileY * TILE_SIZE);
}

void SimpleMapState::SimplePlayer::UpdateAnim(float dt) {
    animTimer += dt;
    if (animTimer > 0.18f) { animTimer = 0.f; animFrame = 1 - animFrame; }
}

void SimpleMapState::SimplePlayer::TryMove(float dx, float dy, const SimpleMap& map) {
    float newX = x + dx, newY = y + dy;
    bool canMoveX = true, canMoveY = true;

    if (dx != 0.f) {
        int l = map.PixelToTile(newX), r = map.PixelToTile(newX + TILE_SIZE - 1.f);
        int t = map.PixelToTile(y), b = map.PixelToTile(y + TILE_SIZE - 1.f);
        for (int ty = t; ty <= b; ++ty) for (int tx = l; tx <= r; ++tx)
            if (!map.IsWalkable(tx, ty)) { canMoveX = false; goto skip_x; }
    skip_x:;
    }
    if (dy != 0.f) {
        int l = map.PixelToTile(x), r = map.PixelToTile(x + TILE_SIZE - 1.f);
        int t = map.PixelToTile(newY), b = map.PixelToTile(newY + TILE_SIZE - 1.f);
        for (int ty = t; ty <= b; ++ty) for (int tx = l; tx <= r; ++tx)
            if (!map.IsWalkable(tx, ty)) { canMoveY = false; goto skip_y; }
    skip_y:;
    }
    if (canMoveX) x = newX;
    if (canMoveY) y = newY;
}

// 前向声明：所有静态辅助渲染函数
static void drawRect(sf::RenderTarget&, sf::Vector2f, sf::Color, float, float);
static void drawCircle(sf::RenderTarget&, float, sf::Color, float, float);
static void renderPlayer(sf::RenderTarget&, float, float, Direction, int, float);
static void renderNPC(sf::RenderTarget&, float, float, int, sf::Color, sf::Color, sf::Color);

void SimpleMapState::SimplePlayer::Render(sf::RenderTarget& target) const {
    // 使用 PlayerSprite 的绘制函数，与主地图风格完全一致
    static PlayerSprite s_playerSprite;
    Direction d = dir;
    Dir playerDir = Dir::Down;
    switch (d) {
        case Direction::Down:  playerDir = Dir::Down; break;
        case Direction::Up:    playerDir = Dir::Up; break;
        case Direction::Left:  playerDir = Dir::Left; break;
        case Direction::Right: playerDir = Dir::Right; break;
    }
    s_playerSprite.SetDirection(playerDir);
    s_playerSprite.DrawCharacter(target, x - 8.f, y - 4.f);
}

// ============================================================
//  统一角色渲染 — 简洁像素风（与 PlayerSprite 风格一致）
//  核心部件：头(10x9) + 身(12x14) + 腿(3x8)x2 + 手臂 + 发髻
// ============================================================

static void renderPlayer(sf::RenderTarget& target, float px, float py,
                         Direction dir, int animFrame, float animTimer) {
    // 与主地图一致的配色（韩立：青色长袍）
    sf::Color robeMain(40, 120, 90);
    sf::Color robeDark(28, 85, 62);
    sf::Color robeLight(55, 150, 115);
    sf::Color skin(255, 215, 170);
    sf::Color hairBlack(30, 25, 22);
    sf::Color beltBrown(140, 90, 40);
    sf::Color shoeGray(60, 55, 50);

    // 格子内偏移（居中）
    float ox = px + 8.f;
    float oy = py + 4.f;

    // 行走动画偏移
    float legOffL = (animFrame == 1) ? -1.5f : ((animFrame == 0) ? 0.f : 1.5f);
    float legOffR = -legOffL;
    float bodyBob = (animFrame != 0) ? -0.8f : 0.f;

    // ====== 阴影 ======
    drawRect(target, sf::Vector2f(16.f, 3.f), sf::Color(0,0,0,50), ox + 2.f, oy + 25.f);

    // ====== 后腿 ======
    bool back = (dir == Direction::Up || dir == Direction::Left);
    float bx = back ? 4.f : 10.f;
    float fx = back ? 10.f : 4.f;
    drawRect(target, sf::Vector2f(3.f, 8.f), robeDark, ox + bx + legOffL, oy + 20.f + bodyBob);
    drawRect(target, sf::Vector2f(4.f, 2.f), shoeGray, ox + bx - 0.5f + legOffL, oy + 27.f + bodyBob);

    // ====== 身体 ======
    drawRect(target, sf::Vector2f(12.f, 14.f), robeMain, ox + 2.f, oy + 11.f + bodyBob);
    // 暗部右边缘
    drawRect(target, sf::Vector2f(3.f, 14.f), robeDark, ox + 11.f, oy + 11.f + bodyBob);
    // 高光左边缘
    drawRect(target, sf::Vector2f(2.f, 13.f), robeLight, ox + 2.f, oy + 11.f + bodyBob);

    // ====== 腰带 ======
    drawRect(target, sf::Vector2f(12.f, 2.f), beltBrown, ox + 2.f, oy + 19.f + bodyBob);

    // ====== 前腿 ======
    drawRect(target, sf::Vector2f(3.f, 8.f), robeMain, ox + fx + legOffR, oy + 20.f + bodyBob);
    drawRect(target, sf::Vector2f(4.f, 2.f), shoeGray, ox + fx - 0.5f + legOffR, oy + 27.f + bodyBob);

    // ====== 头 (10x9) ======
    drawRect(target, sf::Vector2f(10.f, 9.f), skin, ox + 3.f, oy + 2.f + bodyBob);

    // ====== 头发 ======
    drawRect(target, sf::Vector2f(10.f, 5.f), hairBlack, ox + 3.f, oy + bodyBob);
    // 发髻（修仙者特征）
    drawRect(target, sf::Vector2f(4.f, 3.f), hairBlack, ox + 6.f, oy - 2.f + bodyBob);
    // 发簪
    drawRect(target, sf::Vector2f(1.5f, 5.f), sf::Color(180,160,60), ox + 7.25f, oy - 2.f + bodyBob);

    // ====== 面部细节（根据方向）=====
    switch (dir) {
    case Direction::Down: { // 正面 — 眼睛+嘴
        drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(240,230,220), ox+4.5f, oy+6.5f+bodyBob);
        drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(240,230,220), ox+9.5f, oy+6.5f+bodyBob);
        drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   ox+5.f,  oy+7.f+bodyBob);
        drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   ox+10.f, oy+7.f+bodyBob);
        drawRect(target, sf::Vector2f(2.f, 1.f), sf::Color(180,120,100), ox+7.f, oy+9.f+bodyBob);
        break;
    }
    case Direction::Up: { // 背面 — 后脑勺+发髻
        drawRect(target, sf::Vector2f(10.f, 8.f), hairBlack, ox+3.f, oy+2.f+bodyBob);
        break;
    }
    case Direction::Left: { // 左侧 — 单眼
        drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(240,230,220), ox+4.f, oy+6.5f+bodyBob);
        drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   ox+4.2f,oy+7.f+bodyBob);
        break;
    }
    case Direction::Right: { // 右侧 — 单眼
        drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(240,230,220), ox+10.f, oy+6.5f+bodyBob);
        drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   ox+10.f,oy+7.f+bodyBob);
        break;
    }
    }

    // ====== 手臂（身体两侧垂下）=====
    float armX = (dir == Direction::Left) ? 0.f : 13.f;
    drawRect(target, sf::Vector2f(3.f, 9.f), robeDark, ox + armX, oy + 11.f + bodyBob);
    drawCircle(target, 2.8f, skin, ox + armX - 0.3f, oy + 18.f + bodyBob);
    float armX2 = (dir == Direction::Right) ? 0.f : 13.f;
    drawRect(target, sf::Vector2f(3.f, 9.f), robeDark, ox + armX2, oy + 11.f + bodyBob);
    drawCircle(target, 2.8f, skin, ox + armX2 - 0.3f, oy + 18.f + bodyBob);
}

// ============================================================
//  辅助绘制函数（避免重复代码）
// ============================================================

static void drawRect(sf::RenderTarget& target, sf::Vector2f size, sf::Color color, float x, float y) {
    sf::RectangleShape rect(size);
    rect.setFillColor(color);
    rect.setPosition(x, y);
    target.draw(rect);
}

static void drawCircle(sf::RenderTarget& target, float radius, sf::Color color, float x, float y) {
    sf::CircleShape circle(radius);
    circle.setFillColor(color);
    circle.setPosition(x, y);
    target.draw(circle);
}

// ============================================================
//  NPC 渲染
// ============================================================

// ============================================================
//  NPC 统一渲染 — 与玩家同风格的简洁像素人
//  通过体型参数区分：bodyW/bodyH 控制胖瘦，hasSkirt/hasHat 等特征
// ============================================================

void NPC::Render(sf::RenderTarget& target) const {
    float px = (float)(tileX * TILE_SIZE);
    float py = (float)(tileY * TILE_SIZE);
    renderNPC(target, px, py, npcType, clothesColor, hairColor, skinColor);
}

static void renderNPC(sf::RenderTarget& target, float px, float py, int type,
                       sf::Color cloth, sf::Color hair, sf::Color skin) {
    // 阴影
    drawRect(target, sf::Vector2f(16.f, 3.f), sf::Color(0,0,0,40), px + 8.f, py + 27.f);

    float ox = px + 8.f;  // 格子内偏移（与玩家一致）
    float oy = py + 4.f;

    // 暗部颜色
    sf::Color cDark(cloth.r - 20, cloth.g - 20, cloth.b - 18);

    // ===== 根据类型调整体型和特征 =====
    float bodyW = 12.f;   // 身宽
    float bodyH = 13.f;   // 身高
    float headW = 10.f;   // 头宽
    float legW = 3.f;     // 腿宽
    bool hasSkirt = false;
    bool hasHat = false;
    bool isFat = false;
    bool hasBeard = false;
    bool hasApron = false;

    switch (type) {
    case 0: break;  // 村民男 — 默认体型
    case 1: hasSkirt = true; break;  // 女性 — 长裙
    case 2: isFat = true; bodyW=14.f; bodyH=15.f; headW=11.f; legW=4.f; break;  // 胖子
    case 3: hasHat = true; break;    // 商人 — 帽子
    case 4: hasApron = true; break;  // 铁匠 — 围裙
    case 5: hasBeard = true; break;  // 长者 — 胡须
    default: break;
    }

    float bodyOffX = (16.f - bodyW) / 2.f; // 身体居中偏移

    // ===== 后腿 ======
    drawRect(target, sf::Vector2f(legW, 7.f), cDark, ox + bodyOffX + 2.f, oy + 19.f);
    drawRect(target, sf::Vector2f(legW + 1.f, 2.5f), sf::Color(55,45,35), ox + bodyOffX + 1.5f, oy + 25.f);

    // ===== 身体 ======
    drawRect(target, sf::Vector2f(bodyW, bodyH), cloth, ox + bodyOffX, oy + 10.f);
    drawRect(target, sf::Vector2f(3.f, bodyH), cDark, ox + bodyOffX + bodyW - 3.f, oy + 10.f);

    // 围裙（铁匠）
    if (hasApron) {
        drawRect(target, sf::Vector2f(bodyW - 4.f, bodyH - 3.f), sf::Color(150,110,55),
                 ox + bodyOffX + 2.f, oy + 12.f);
    }

    // 裙子（女性）
    if (hasSkirt) {
        drawRect(target, sf::Vector2f(bodyW + 2.f, 9.f), sf::Color(cloth.r-15,cloth.g-10,cloth.b-5),
                 ox + bodyOffX - 1.f, oy + 15.f);
    }

    // ===== 腰带 ======
    drawRect(target, sf::Vector2f(bodyW, 2.f), sf::Color(130,95,50), ox + bodyOffX, oy + 17.f);

    // ===== 前腿 ======
    if (isFat) {
        drawRect(target, sf::Vector2f(legW, 6.f), cDark, ox + bodyOffX + bodyW - legW - 2.f, oy + 20.f);
        drawRect(target, sf::Vector2f(legW + 1.f, 2.5f), sf::Color(50,40,30), ox + bodyOffX + bodyW - legW - 2.5f, oy + 25.f);
    } else {
        drawRect(target, sf::Vector2f(legW, 7.f), cloth, ox + bodyOffX + bodyW - legW - 2.f, oy + 19.f);
        drawRect(target, sf::Vector2f(legW + 1.f, 2.5f), sf::Color(55,45,35), ox + bodyOffX + bodyW - legW - 2.5f, oy + 25.f);
    }

    // ===== 头 ======
    float headOffX = (16.f - headW) / 2.f;
    drawRect(target, sf::Vector2f(headW, (isFat?10.f:9.f)), skin, ox + headOffX, oy + 1.f);

    // 头发
    drawRect(target, sf::Vector2f(headW, 4.f), hair, ox + headOffX, oy);

    // 商人帽
    if (hasHat) {
        drawRect(target, sf::Vector2f(headW + 4.f, 3.f), sf::Color(50,40,32), ox + headOffX - 2.f, oy - 3.f);
        drawRect(target, sf::Vector2f(headW, 4.f), sf::Color(60,48,38), ox + headOffX - 1.f, oy - 6.f);
    }

    // 长者花白胡子
    if (hasBeard) {
        drawCircle(target, 3.5f, sf::Color(180,170,160), ox + headOffX + headW/2.f + 1.f, oy + 7.f);
        drawRect(target, sf::Vector2f(3.f, 3.f), sf::Color(170,160,150), ox + headOffX + headW/2.f, oy + 8.f);
    }

    // 女性长发
    if (hasSkirt) {
        drawCircle(target, 4.5f, hair, ox + headOffX - 1.f, oy + 2.f);
        drawCircle(target, 4.5f, hair, ox + headOffX + headW - 3.f, oy + 2.f);
        drawRect(target, sf::Vector2f(headW, 3.f), hair, ox + headOffX, oy - 1.f);
    }

    // ===== 面部（统一正面）=====
    float eyeY = oy + 5.f;
    float eyeLx = ox + headOffX + 2.f;
    float eyeRx = ox + headOffX + headW - 4.f;

    // 眼白+眼珠
    drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(245,235,220), eyeLx, eyeY);
    drawRect(target, sf::Vector2f(2.f, 2.f), sf::Color(245,235,220), eyeRx, eyeY);
    drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   eyeLx+0.4f, eyeY+0.5f);
    drawRect(target, sf::Vector2f(1.2f,1.2f), sf::Color(30,25,20),   eyeRx+0.4f, eyeY+0.5f);

    // 嘴
    drawRect(target, sf::Vector2f(2.f, 1.f), sf::Color(170,115,95),
             ox + headOffX + headW/2.f - 1.f, oy + (isFat?10.f:9.f));

    // ===== 手臂 ======
    drawRect(target, sf::Vector2f(3.f, 8.f), cDark, ox + bodyOffX - 2.f, oy + 11.f);
    drawCircle(target, 2.6f, skin, ox + bodyOffX - 2.3f, oy + 17.f);
    drawRect(target, sf::Vector2f(3.f, 8.f), cDark, ox + bodyOffX + bodyW - 1.f, oy + 11.f);
    drawCircle(target, 2.6f, skin, ox + bodyOffX + bodyW - 1.3f, oy + 17.f);
}

// ============================================================
//  NPC 名字标签（修复乱码）
// ============================================================

void NPC::RenderNameTag(sf::RenderTarget& target, const sf::Font& font) const {
    if (font.getInfo().family.empty()) return;

    std::wstring displayName = L"[" + nameW + L"]";
    sf::Text nameLabel(displayName, font, 12);
    nameLabel.setFillColor(sf::Color(255, 255, 200));

    float textWidth = nameLabel.getLocalBounds().width;
    float nameX = (float)(tileX * 32) + 16.f - textWidth / 2.f;
    float nameY = (float)(tileY * 32) - 15.f;

    // 描边效果（4方向偏移）
    nameLabel.setFillColor(sf::Color(0, 0, 0, 180));
    nameLabel.setPosition(nameX - 1.f, nameY - 1.f); target.draw(nameLabel);
    nameLabel.setPosition(nameX + 1.f, nameY - 1.f); target.draw(nameLabel);
    nameLabel.setPosition(nameX - 1.f, nameY + 1.f); target.draw(nameLabel);
    nameLabel.setPosition(nameX + 1.f, nameY + 1.f); target.draw(nameLabel);

    // 最终名字
    nameLabel.setFillColor(sf::Color(255, 255, 200));
    nameLabel.setPosition(nameX, nameY);
    target.draw(nameLabel);
}

const std::wstring& NPC::GetDialog(int idx) const {
    static const std::wstring empty = L"";
    if (idx >= 0 && idx < (int)dialog.size()) return dialog[idx];
    return empty;
}

// ============================================================
//  SimpleMapState 核心方法
// ============================================================

void SimpleMapState::Update(float dt) {
    if (m_showHint) { m_hintTimer -= dt; if (m_hintTimer <= 0.f) m_showHint = false; }
    m_player.UpdateAnim(dt);

    // 任务完成通知
    auto& questSys = QuestSystem::Instance();
    if (!questSys.m_lastCompletedQuestName.empty()) {
        std::string qn = questSys.m_lastCompletedQuestName;
        m_hintTimer = 5.f; m_showHint = true;
        questSys.m_lastCompletedQuestName.clear();
    }

    // 自动检测：走到镇门自动传送
    int ptx = SimpleMap::PixelToTile(m_player.x);
    int pty = SimpleMap::PixelToTile(m_player.y);
    if ((pty == 12 && ptx <= 1) || (pty == 12 && ptx >= MAP_W - 2)) {
        // 已经在镇门位置
    }
}

void SimpleMapState::HandleInput() {}

void SimpleMapState::OnKeyPressed(sf::Keyboard::Key key) {
    // ===== 功能键（与 WorldMapState 统一）=====
    switch (key) {
    case sf::Keyboard::Escape:
        GameStateManager::Instance().PushState(std::make_unique<SettingsState>());
        return;
    case sf::Keyboard::I:
        OnOpenInventory();
        return;
    case sf::Keyboard::Space:
    case sf::Keyboard::C:
        OnOpenCultivationUI();
        return;
    case sf::Keyboard::M:
        return;
    case sf::Keyboard::Q:
        OnOpenQuestUI();
        return;
    default: break;
    }

    if (m_showDialog) {
        switch (key) {
        case sf::Keyboard::Space:
        case sf::Keyboard::Enter: {
            m_dialogIndex++;
            const NPC& npc = m_npcs[m_activeNpcIndex];
            if (m_dialogIndex >= npc.GetDialogCount()) { m_showDialog = false; m_activeNpcIndex = -1; }
            else { m_dialogText = npc.GetDialog(m_dialogIndex); }
            return;
        }
        case sf::Keyboard::Escape:
            m_showDialog = false; m_activeNpcIndex = -1; return;
        default: break;
        }
        return;
    }

    int ptx = SimpleMap::PixelToTile(m_player.x);
    int pty = SimpleMap::PixelToTile(m_player.y);

    if (key == sf::Keyboard::E) {
        // 检查镇门（返回主地图）
        if ((pty == 12 && ptx <= 1) || (pty == 12 && ptx >= MAP_W - 2)) {
            GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
            return;
        }

        // NPC交互
        for (int i = 0; i < (int)m_npcs.size(); ++i) {
            int dx = abs(ptx - m_npcs[i].tileX), dy = abs(pty - m_npcs[i].tileY);
            if (dx <= 2 && dy <= 2) {
                if (m_npcs[i].name == "technique_merchant") { OnOpenShop(L"◆ 功法秘籍商 ◆", "technique"); return; }
                if (m_npcs[i].name == "equip_merchant")     { OnOpenShop(L"◆ 装备店 ◆", "equipment"); return; }
                if (m_npcs[i].name == "shopkeeper")         { OnOpenShop(L"◆ 杂货铺 ◆", "misc"); return; }
                if (m_npcs[i].name == "blacksmith")         { OnOpenShop(L"◆ 铁匠铺 ◆", "weapon"); return; }
                m_showDialog = true; m_dialogIndex = 0;
                m_dialogText = m_npcs[i].GetDialog(0); m_activeNpcIndex = i; return;
            }
        }
        return;
    }

    // 移动（与主地图一致：先移动，再检测门）
    bool moved = false;
    switch (key) {
    case sf::Keyboard::Up: case sf::Keyboard::W:
        m_player.dir = Direction::Up; m_player.TryMove(0.f, -SPEED, m_map); moved = true; break;
    case sf::Keyboard::Down: case sf::Keyboard::S:
        m_player.dir = Direction::Down; m_player.TryMove(0.f, SPEED, m_map); moved = true; break;
    case sf::Keyboard::Left: case sf::Keyboard::A:
        m_player.dir = Direction::Left; m_player.TryMove(-SPEED, 0.f, m_map); moved = true; break;
    case sf::Keyboard::Right: case sf::Keyboard::D:
        m_player.dir = Direction::Right; m_player.TryMove(SPEED, 0.f, m_map); moved = true; break;
    default: break;
    }

    // 移动后检测是否站在门上，自动传送
    if (moved) {
        int nx = SimpleMap::PixelToTile(m_player.x);
        int ny = SimpleMap::PixelToTile(m_player.y);
        if ((ny == 12 && nx <= 1) || (ny == 12 && nx >= MAP_W - 2)) {
            GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
        }
    }
}

void SimpleMapState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 30, 40));

    float pcx = m_player.x + TILE_SIZE / 2.f;
    float pcy = m_player.y + TILE_SIZE / 2.f;
    int mpW = MAP_W * TILE_SIZE, mpH = MAP_H * TILE_SIZE;
    float hw = 400.f, hh = 300.f;
    float minX = std::min(hw, (float)mpW - hw), maxX = std::max(hw, (float)mpW - hw);
    float minY = std::min(hh, (float)mpH - hh), maxY = std::max(hh, (float)mpH - hh);
    m_cameraView.setCenter(std::clamp(pcx, minX, maxX), std::clamp(pcy, minY, maxY));
    window.setView(m_cameraView);

    m_map.Render(window);

    for (const auto& npc : m_npcs) npc.Render(window);
    if (m_font.getInfo().family.size() > 0) {
        for (const auto& npc : m_npcs) npc.RenderNameTag(window, m_font);
    }

    m_player.Render(window);

    window.setView(window.getDefaultView());

    // 当前任务追踪
    const auto& allQuests = QuestSystem::Instance().GetAllQuests();
    for (const auto& q : allQuests) {
    if (q.status == QuestStatus::Active) {
        // 简单ASCII/UTF-8转宽字符串
        std::wstring wname(q.name.begin(), q.name.end());
        sf::Text qt;
            qt.setFont(m_font);
            qt.setString(L"⚡ " + wname);
            qt.setCharacterSize(14);
            qt.setFillColor(sf::Color(255, 215, 80));
            qt.setPosition(10.f, 540.f);
            window.draw(qt);
            break;
        }
    }

    if (m_showHint && m_font.getInfo().family.size() > 0) {
        sf::Text hint(L"方向键/WASD 移动 | E 交互/离开 | I 物品 | C 修炼 | Esc 菜单", m_font, 15);
        hint.setFillColor(sf::Color(220, 220, 220));
        hint.setPosition(10.f, 562.f);
        window.draw(hint);
    }

    if (m_showDialog && m_activeNpcIndex >= 0 && m_activeNpcIndex < (int)m_npcs.size()) {
        const NPC& npc = m_npcs[m_activeNpcIndex];

        sf::RectangleShape dlgBg(sf::Vector2f(760.f, 105.f));
        dlgBg.setFillColor(sf::Color(15, 10, 5, 235));
        dlgBg.setOutlineThickness(2.f);
        dlgBg.setOutlineColor(sf::Color(180, 140, 70));
        dlgBg.setPosition(20.f, 468.f);
        window.draw(dlgBg);

        // 对话内容
        std::wstring titleStr = L"【" + npc.nameW + L"】";
        sf::Text nText(titleStr, m_font, 17);
        // 名字描边
        nText.setFillColor(sf::Color(80, 50, 0));
        nText.setPosition(37.f, 479.f); window.draw(nText);
        nText.setPosition(39.f, 481.f); window.draw(nText);
        nText.setFillColor(sf::Color(255, 210, 100));
        nText.setPosition(38.f, 480.f);
        window.draw(nText);

        sf::Text dText(m_dialogText, m_font, 16);
        dText.setFillColor(sf::Color(245, 240, 228));
        dText.setPosition(38.f, 506.f);
        window.draw(dText);

        sf::Text hDlg(L"[空格/回车] 继续 | [ESC] 关闭", m_font, 13);
        hDlg.setFillColor(sf::Color(140, 130, 110));
        hDlg.setPosition(620.f, 548.f);
        window.draw(hDlg);
    }
}
