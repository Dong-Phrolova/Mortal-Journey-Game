#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// 地图上的可见敌人（不是战斗状态，是世界地图上走的敌人）

enum class EnemyAIState {
    Patrol,     // 巡逻：在出生点附近随机移动
    Chase,      // 追踪：发现玩家，向玩家移动
    Combat      // 进入战斗（由外部处理）
};

struct MapEnemy {
    int id;                     // 唯一ID
    std::string enemyId;        // 敌人模板ID（对应enemyPool中的ID）
    std::string name;           // 显示名称
    std::wstring nameW;         // 宽字符名称

    // 位置和移动
    float x, y;                 // 像素坐标（浮点，支持平滑移动）
    int spawnX, spawnY;         // 出生格字坐标
    float targetX, targetY;     // 移动目标像素坐标
    float speed;                // 移动速度（像素/秒）

    // 外观
    sf::Color mainColor;        // 主色调
    int faceDir;                // 朝向: 0=下 1=上 2=左 3=右

    // AI
    EnemyAIState aiState = EnemyAIState::Patrol;
    float patrolRadius;         // 巡逻半径（格子数）
    float detectRange;          // 检测玩家范围（像素）
    float chaseSpeed;           // 追踪速度
    float patrolTimer = 0.f;    // 巡逻计时器
    float stateTimer = 0.f;     // 状态计时器

    // 战斗
    bool inCombat = false;      // 是否已进入战斗（防止重复触发）
};

// 地图敌人管理器
class MapEnemySystem {
public:
    MapEnemySystem();

    // 在当前地图生成敌人
    void SpawnEnemies(const std::vector<std::string>& enemyPool,
                      int mapW, int mapH);

    // 清除所有敌人
    void ClearEnemies();

    // 更新所有敌人AI
    void Update(float dt, float playerX, float playerY);

    // 渲染所有敌人
    void Render(sf::RenderTarget& target, float camOffsetX, float camOffsetY);

    // 检查与玩家的碰撞（返回碰到的敌人指针， nullptr 表示无碰撞）
    MapEnemy* CheckPlayerCollision(float playerX, float playerY, float collisionDist);

    // 获取敌人列表
    const std::vector<MapEnemy>& GetEnemies() const { return m_enemies; }

    // 标记敌人进入战斗
    void MarkInCombat(int enemyId);

private:
    std::vector<MapEnemy> m_enemies;
    int m_nextId = 1;

    // 选择巡逻目标点
    void PickPatrolTarget(MapEnemy& enemy);

    // 绘制一个敌人精灵
    void DrawEnemySprite(sf::RenderTarget& target, const MapEnemy& enemy,
                         float screenX, float screenY);
};
