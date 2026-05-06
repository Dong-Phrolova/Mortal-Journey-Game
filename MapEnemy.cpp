#pragma execution_character_set("utf-8")
#include "MapEnemy.h"
#include "TileSet.h"
#include <cmath>
#include <cstdlib>

// ============================================================
//  MapEnemySystem 构造
// ============================================================
MapEnemySystem::MapEnemySystem() {}

void MapEnemySystem::ClearEnemies() {
    m_enemies.clear();
}

// ============================================================
//  在地图上生成敌人
// ============================================================
void MapEnemySystem::SpawnEnemies(const std::vector<std::string>& enemyPool,
                                   int mapW, int mapH) {
    if (enemyPool.empty()) return;

    // 根据地图大小决定敌人数量（每张地图最多5个）
    int area = mapW * mapH;
    int count = std::min(5, std::max(1, area / 80));

    for (int i = 0; i < count; i++) {
        MapEnemy enemy;
        enemy.id = m_nextId++;
        enemy.enemyId = enemyPool[i % enemyPool.size()];

        // 根据敌人ID设置外观和属性
        if (enemy.enemyId == "mortal_thug") {
            enemy.name = "流氓";
            enemy.nameW = L"流氓";
            enemy.mainColor = sf::Color(140, 100, 80);   // 棕色衣服
            enemy.speed = 35.f;
            enemy.detectRange = 70.f;
            enemy.chaseSpeed = 75.f;
            enemy.patrolRadius = 2.5f;
        } else if (enemy.enemyId == "qi_disciple") {
            enemy.name = "黄枫谷弟子";
            enemy.nameW = L"黄枫谷弟子";
            enemy.mainColor = sf::Color(70, 130, 180);   // 蓝色道袍
            enemy.speed = 40.f;
            enemy.detectRange = 80.f;
            enemy.chaseSpeed = 85.f;
            enemy.patrolRadius = 3.f;
        } else if (enemy.enemyId == "spirit_beast") {
            enemy.name = "灵兽";
            enemy.nameW = L"灵兽";
            enemy.mainColor = sf::Color(100, 160, 100);   // 绿色皮毛
            enemy.speed = 50.f;
            enemy.detectRange = 75.f;
            enemy.chaseSpeed = 95.f;
            enemy.patrolRadius = 4.f;
        } else if (enemy.enemyId == "blood_spider") {
            enemy.name = "血蛛";
            enemy.nameW = L"血蛛";
            enemy.mainColor = sf::Color(160, 50, 70);     // 血红色
            enemy.speed = 55.f;
            enemy.detectRange = 85.f;
            enemy.chaseSpeed = 105.f;
            enemy.patrolRadius = 3.f;
        } else if (enemy.enemyId == "yao_beast_qi") {
            enemy.name = "妖兽";
            enemy.nameW = L"妖兽";
            enemy.mainColor = sf::Color(140, 80, 140);    // 紫色
            enemy.speed = 60.f;
            enemy.detectRange = 90.f;
            enemy.chaseSpeed = 115.f;
            enemy.patrolRadius = 4.f;
        } else if (enemy.enemyId == "yelang_thug") {
            // ===== 野狼帮打手（小说第3-4章对立势力）=====
            enemy.name = "野狼帮众";
            enemy.nameW = L"野狼帮众";
            enemy.mainColor = sf::Color(80, 60, 50);     // 深褐/黑衣
            enemy.speed = 42.f;
            enemy.detectRange = 85.f;
            enemy.chaseSpeed = 90.f;
            enemy.patrolRadius = 3.5f;
        } else if (enemy.enemyId == "yelang_captain") {
            // 野狼帮小头目
            enemy.name = "野狼头目";
            enemy.nameW = L"野狼头目";
            enemy.mainColor = sf::Color(120, 40, 30);    // 暗红衣
            enemy.speed = 48.f;
            enemy.detectRange = 100.f;
            enemy.chaseSpeed = 98.f;
            enemy.patrolRadius = 4.f;
        } else if (enemy.enemyId == "yelang_boss") {
            // 野狼帮老大（Boss级）
            enemy.name = "野狼帮主";
            enemy.nameW = L"野狼帮主";
            enemy.mainColor = sf::Color(160, 20, 20);    // 血红衣+金边
            enemy.speed = 55.f;
            enemy.detectRange = 120.f;
            enemy.chaseSpeed = 110.f;
            enemy.patrolRadius = 5.f;
        } else {
            // 默认敌人
            enemy.name = "怪物";
            enemy.nameW = L"怪物";
            enemy.mainColor = sf::Color(150, 80, 80);
            enemy.speed = 40.f;
            enemy.detectRange = 120.f;
            enemy.chaseSpeed = 90.f;
            enemy.patrolRadius = 3.f;
        }

        // 随机出生位置（避免在边缘）
        int ts = TileSet::TILE_SIZE;
        enemy.spawnX = 2 + rand() % (mapW - 4);
        enemy.spawnY = 2 + rand() % (mapH - 4);

        // 像素坐标（格子中心偏移）
        enemy.x = (float)(enemy.spawnX * ts) + ts * 0.4f;
        enemy.y = (float)(enemy.spawnY * ts) + ts * 0.25f;

        enemy.targetX = enemy.x;
        enemy.targetY = enemy.y;
        enemy.faceDir = 0; // 朝下
        enemy.aiState = EnemyAIState::Patrol;
        enemy.inCombat = false;
        enemy.patrolTimer = 0.f;
        enemy.stateTimer = 0.f;

        m_enemies.push_back(enemy);
    }
}

// ============================================================
//  更新所有敌人 AI
// ============================================================
void MapEnemySystem::Update(float dt, float playerX, float playerY) {
    for (auto& enemy : m_enemies) {
        if (enemy.inCombat) continue; // 已进战斗的跳过

        float dx = playerX - enemy.x;
        float dy = playerY - enemy.y;
        float distToPlayer = sqrtf(dx*dx + dy*dy);

        switch (enemy.aiState) {
        case EnemyAIState::Patrol: {
            // 检测是否发现玩家
            if (distToPlayer < enemy.detectRange) {
                enemy.aiState = EnemyAIState::Chase;
                enemy.stateTimer = 0.f;
                break;
            }

            // 巡逻逻辑
            enemy.patrolTimer -= dt;
            if (enemy.patrolTimer <= 0.f) {
                PickPatrolTarget(enemy);
                enemy.patrolTimer = 2.f + (rand() % 30) / 10.f; // 2~5秒后重新选择目标
            }

            // 向巡逻目标移动
            float tdx = enemy.targetX - enemy.x;
            float tdy = enemy.targetY - enemy.y;
            float tdist = sqrtf(tdx*tdx + tdy*tdy);

            if (tdist > 3.f) {
                float moveAmt = enemy.speed * dt;
                enemy.x += (tdx / tdist) * moveAmt;
                enemy.y += (tdy / tdist) * moveAmt;

                // 更新朝向
                if (fabsf(tdx) > fabsf(tdy))
                    enemy.faceDir = (tdx > 0) ? 3 : 2; // 右/左
                else
                    enemy.faceDir = (tdy > 0) ? 0 : 1; // 下/上
            }
            break;
        }

        case EnemyAIState::Chase: {
            // 追踪玩家
            if (distToPlayer > enemy.detectRange * 2.f) {
                // 玩家跑远了，回到巡逻
                enemy.aiState = EnemyAIState::Patrol;
                enemy.patrolTimer = 0.f;
                break;
            }

            if (distToPlayer > 2.f) {
                float moveAmt = enemy.chaseSpeed * dt;
                enemy.x += (dx / distToPlayer) * moveAmt;
                enemy.y += (dy / distToPlayer) * moveAmt;

                // 更新朝向（面向玩家）
                if (fabsf(dx) > fabsf(dy))
                    enemy.faceDir = (dx > 0) ? 3 : 2;
                else
                    enemy.faceDir = (dy > 0) ? 0 : 1;
            }
            break;
        }

        default:
            break;
        }

        enemy.stateTimer += dt;
    }
}

// ============================================================
//  选择巡逻目标点
// ============================================================
void MapEnemySystem::PickPatrolTarget(MapEnemy& enemy) {
    int ts = TileSet::TILE_SIZE;
    float radiusPixels = enemy.patrolRadius * ts;

    // 在巡逻半径内随机选一点
    float angle = (rand() % 360) * (3.14159f / 180.f);
    float r = radiusPixels * (0.3f + (rand() % 70) / 100.f); // 半径的 30%~100%

    float spawnCenterX = (float)(enemy.spawnX * ts) + ts * 0.5f;
    float spawnCenterY = (float)(enemy.spawnY * ts) + ts * 0.5f;

    enemy.targetX = spawnCenterX + cosf(angle) * r;
    enemy.targetY = spawnCenterY + sinf(angle) * r;
}

// ============================================================
//  检查与玩家的碰撞
// ============================================================
MapEnemy* MapEnemySystem::CheckPlayerCollision(float playerX, float playerY,
                                                 float collisionDist) {
    for (auto& enemy : m_enemies) {
        if (enemy.inCombat) continue;

        float dx = playerX - enemy.x;
        float dy = playerY - enemy.y;
        float dist = sqrtf(dx*dx + dy*dy);

        if (dist < collisionDist) {
            return &enemy;
        }
    }
    return nullptr;
}

void MapEnemySystem::MarkInCombat(int enemyId) {
    for (auto& e : m_enemies) {
        if (e.id == enemyId) {
            e.inCombat = true;
            break;
        }
    }
}

// ============================================================
//  渲染所有敌人
// ============================================================
void MapEnemySystem::Render(sf::RenderTarget& target, float camOffsetX, float camOffsetY) {
    for (const auto& enemy : m_enemies) {
        if (enemy.inCombat) continue;

        float screenX = enemy.x + camOffsetX;
        float screenY = enemy.y + camOffsetY;

        // 视野裁剪
        if (screenX < -64 || screenX > 864 || screenY < -64 || screenY > 664)
            continue;

        DrawEnemySprite(target, enemy, screenX, screenY);
    }
}

// ============================================================
//  绘制敌人像素精灵（类似 NPC 但更凶恶的外观）
// ============================================================
void MapEnemySystem::DrawEnemySprite(sf::RenderTarget& target, const MapEnemy& enemy,
                                      float screenX, float screenY) {
    // 偏移：角色居中于格子
    float ox = screenX;
    float oy = screenY;

    sf::Color mainC = enemy.mainColor;
    sf::Color darkC(mainC.r * 6 / 10, mainC.g * 6 / 10, mainC.b * 6 / 10);
    sf::Color lightC(std::min(255, mainC.r + 50),
                     std::min(255, mainC.g + 50),
                     std::min(255, mainC.b + 50));

    // 阴影
    sf::RectangleShape shadow(sf::Vector2f(18.f, 3.f));
    shadow.setPosition(ox + 3.f, oy + 27.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    target.draw(shadow);

    // === 根据敌人类型有略微不同的外观 ===
    bool isBeast = (enemy.enemyId == "spirit_beast" ||
                   enemy.enemyId == "yao_beast_qi");
    bool isSpider = (enemy.enemyId == "blood_spider");

    if (isSpider) {
        // === 血蛛：圆形身体+腿 ===
        // 身体（圆/椭圆）
        sf::RectangleShape body(sf::Vector2f(16.f, 12.f));
        body.setPosition(ox + 2.f, oy + 10.f);
        body.setFillColor(mainC);
        target.draw(body);

        // 头
        sf::RectangleShape head(sf::Vector2f(10.f, 8.f));
        head.setPosition(ox + 5.f, oy + 3.f);
        head.setFillColor(darkC);
        target.draw(head);

        // 眼睛（红色发光）
        sf::RectangleShape eyeL(sf::Vector2f(3.f, 3.f));
        eyeL.setFillColor(sf::Color(255, 50, 50));
        eyeL.setPosition(ox + 6.f, oy + 5.f);
        target.draw(eyeL);

        sf::RectangleShape eyeR(sf::Vector2f(3.f, 3.f));
        eyeR.setFillColor(sf::Color(255, 50, 50));
        eyeR.setPosition(ox + 13.f, oy + 5.f);
        target.draw(eyeR);

        // 腿（8条腿）
        sf::Color legC(80, 30, 30);
        float legPos[4][2] = {{ox+2,oy+18},{ox+7,oy+20},{ox+13,oy+20},{ox+18,oy+18}};
        for (int i = 0; i < 4; i++) {
            sf::RectangleShape leg(sf::Vector2f(2.f, 6.f));
            leg.setFillColor(legC);
            leg.setPosition(legPos[i][0], legPos[i][1]);
            target.draw(leg);
            leg.setSize(sf::Vector2f(2.f, 5.f));
            leg.setPosition(legPos[i][0] + (i<2?2:-2), legPos[i][1] + 4.f);
            target.draw(leg);
        }
    } else if (isBeast) {
        // === 灵兽/妖兽：四足野兽造型 ===
        // 身体（椭圆形状）
        sf::RectangleShape body(sf::Vector2f(18.f, 11.f));
        body.setPosition(ox + 1.f, oy + 12.f);
        body.setFillColor(mainC);
        target.draw(body);

        // 头
        sf::RectangleShape head(sf::Vector2f(10.f, 9.f));
        head.setPosition(ox + (enemy.faceDir==2 ? 0:8), oy + 5.f);
        head.setFillColor(lightC);
        target.draw(head);

        // 耳朵
        sf::RectangleShape ear(sf::Vector2f(3.f, 4.f));
        ear.setFillColor(darkC);
        ear.setPosition(ox + 5.f, oy + 2.f);
        target.draw(ear);
        ear.setPosition(ox + 12.f, oy + 2.f);
        target.draw(ear);

        // 腿（4条）
        sf::Color legC(darkC);
        sf::RectangleShape leg(sf::Vector2f(3.f, 6.f));
        leg.setFillColor(legC);
        leg.setPosition(ox + 3.f, oy + 22.f); target.draw(leg);
        leg.setPosition(ox + 14.f, oy + 22.f); target.draw(leg);
        leg.setPosition(ox + 5.f, oy + 21.f); target.draw(leg);
        leg.setPosition(ox + 12.f, oy + 21.f); target.draw(leg);

        // 眼睛
        sf::RectangleShape eye(sf::Vector2f(2.f, 2.f));
        eye.setFillColor(sf::Color(220, 255, 150)); // 兽眼发光
        float ex = (enemy.faceDir == 2) ? ox + 2.f : ox + 14.f;
        eye.setPosition(ex, oy + 8.f);
        target.draw(eye);

        // 尾巴
        sf::RectangleShape tail(sf::Vector2f(2.f, 6.f));
        tail.setFillColor(darkC);
        tail.setPosition(ox + (enemy.faceDir==3?0:18), oy + 14.f);
        target.draw(tail);

    } else {
        // === 人形敌人：流氓/弟子 ===

        // 后腿
        sf::RectangleShape backLeg(sf::Vector2f(3.f, 8.f));
        backLeg.setFillColor(darkC);
        backLeg.setPosition(ox + 4.f, oy + 19.f);
        target.draw(backLeg);

        // 身体 (13x14)
        sf::RectangleShape body(sf::Vector2f(13.f, 14.f));
        body.setPosition(ox + 2.5f, oy + 10.f);
        body.setFillColor(mainC);
        target.draw(body);

        // 暗部
        sf::RectangleShape bodyDark(sf::Vector2f(3.f, 14.f));
        bodyDark.setPosition(ox + 12.f, oy + 10.f);
        bodyDark.setFillColor(darkC);
        target.draw(bodyDark);

        // 前腿
        sf::RectangleShape frontLeg(sf::Vector2(3.f, 8.f));
        frontLeg.setFillColor(mainC);
        frontLeg.setPosition(ox + 10.f, oy + 19.f);
        target.draw(frontLeg);

        // 头 (10x9)
        sf::RectangleShape head(sf::Vector2f(10.f, 9.f));
        head.setPosition(ox + 4.f, oy + 1.f);
        head.setFillColor(sf::Color(220, 180, 150));
        target.draw(head);

        // 头发（深色）
        sf::RectangleShape hair(sf::Vector2f(10.f, 5.f));
        hair.setPosition(ox + 4.f, oy - 2.f);
        hair.setFillColor(sf::Color(40, 30, 25));
        target.draw(hair);

        // 眼睛（红色/敌意眼神）
        sf::Color eyeCol(180, 40, 40);
        switch (enemy.faceDir) {
        case 0: { // 下-正面
            sf::RectangleShape el(sf::Vector2f(1.8f, 1.8f));
            el.setFillColor(eyeCol);
            el.setPosition(ox + 5.5f, oy + 5.5f); target.draw(el);
            el.setPosition(ox + 10.5f, oy + 5.5f); target.draw(el);
            break;
        }
        case 2: { // 左
            sf::RectangleShape el(sf::Vector2f(1.8f, 1.8f));
            el.setFillColor(eyeCol);
            el.setPosition(ox + 4.5f, oy + 5.5f); target.draw(el);
            break;
        }
        case 3: { // 右
            sf::RectangleShape el(sf::Vector2f(1.8f, 1.8f));
            el.setFillColor(eyeCol);
            el.setPosition(ox + 11.f, oy + 5.5f); target.draw(el);
            break;
        }
        case 1: // 上-背面看不到
            break;
        }

        // 敌人标识：头顶红色感叹号（追踪状态时显示）
        if (enemy.aiState == EnemyAIState::Chase) {
            sf::RectangleShape excl(sf::Vector2f(2.f, 6.f));
            excl.setFillColor(sf::Color(255, 255, 0));
            excl.setPosition(ox + 8.f, oy - 9.f);
            target.draw(excl);

            sf::RectangleShape dot(sf::Vector2f(3.f, 2.f));
            dot.setFillColor(sf::Color(255, 255, 0));
            dot.setPosition(ox + 7.5f, oy - 10.f);
            target.draw(dot);
        }
    }
}
