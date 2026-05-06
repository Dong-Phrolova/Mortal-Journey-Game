#pragma once
#include "Player.h"
#include "Enemy.h"
#include <SFML/Graphics.hpp>

// 战斗结果
enum class CombatResult { Ongoing, Victory, Defeat };

// 战斗状态（用于渲染）
struct CombatUIData {
    bool inCombat = false;
    int round = 0;
    int playerChoice = -1;   // 0=攻击 1=疗伤 2=技能
    int enemyAction   = -1;
    int lastDamageToEnemy = 0;
    int lastDamageToPlayer = 0;
    std::wstring logLine;
};

// 初始化战斗 UI 资源
bool InitCombatUI();

// 战斗主循环（SFML 事件驱动）
CombatResult RunCombat(Player& player, Enemy& enemy, sf::RenderWindow& window);

// 控制台版战斗（兼容旧逻辑，可删除）
void StartBattle(Player& player, Enemy& enemy);
