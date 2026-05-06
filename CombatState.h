#pragma once
#pragma execution_character_set("utf-8")
#include "GameState.h"
#include "Player.h"
#include "Enemy.h"
#include "Combat.h"
#include "ItemSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// 战斗阶段
enum class CombatPhase {
    PlayerTurn,      // 玩家选择主菜单
    SelectSkill,     // 选择技能子菜单
    SelectItem,      // 选择道具子菜单
    EnemyTurn,       // 敌人行动（延迟动画）
    ShowMessage,     // 展示战斗消息（等待按键继续）
    CombatEnd        // 战斗结束
};

// 主菜单选项
enum class MainAction { Attack = 0, Skill = 1, Item = 2, Run = 3 };

// 战斗技能数据（接入功法系统）
struct CombatSkill {
    std::string id;
    std::wstring name;
    std::wstring desc;
    int mpCost = 0;
    float dmgMult = 1.0f;   // 伤害倍率
    int healAmount = 0;      // 回复量（>0表示治疗）
    bool isHeal = false;
};

class CombatState : public GameState {
public:
    CombatState(const std::wstring& enemyName, int hp, int mp,
                int atk, int def, EnemyType type = EnemyType::Mortal);

    void Enter() override;
    void Exit() override;
    void Update(float dt) override;
    void HandleInput() override {}
    void Render(sf::RenderWindow& window) override;
    void OnKeyPressed(sf::Keyboard::Key key);

    bool IsVictory() const { return m_result == CombatResult::Victory; }
    CombatPhase GetPhase() const { return m_phase; }
    int GetExpReward() const { return m_expReward; }
    int GetGoldReward() const { return m_goldReward; }

private:
    // ===== 核心逻辑 =====
    void BuildSkillList();       // 根据玩家功法构建技能列表
    void BuildItemList();        // 获取背包中可用消耗品
    void ExecutePlayerAttack();
    void ExecutePlayerSkill(int skillIndex);
    void ExecutePlayerItem(int itemIndex);
    void ExecuteEnemyAction();
    void CheckCombatEnd();

    // ===== 消息队列 =====
    void PushMessage(const std::wstring& msg);
    void ShowNextMessage();

    // ===== 奖励 =====
    void GiveRewards();

    // ===== 渲染子函数 =====
    void RenderBackground(sf::RenderWindow& w);
    void RenderPlayerSprite(sf::RenderWindow& w);
    void RenderEnemySprite(sf::RenderWindow& w);
    void RenderStatusBars(sf::RenderWindow& w);
    void RenderMessageBox(sf::RenderWindow& w);
    void RenderMainMenu(sf::RenderWindow& w);
    void RenderSkillMenu(sf::RenderWindow& w);
    void RenderItemMenu(sf::RenderWindow& w);
    void RenderEndScreen(sf::RenderWindow& w);

    // ===== 辅助绘图 =====
    void DrawBar(sf::RenderWindow& w, float x, float y, float w2, float h,
                 float ratio, sf::Color fg, sf::Color bg);
    void DrawTextShadow(sf::RenderWindow& w, sf::Text& t, float x, float y,
                        const std::wstring& str, sf::Color col, sf::Color shadow);
    void DrawBox(sf::RenderWindow& w, float x, float y, float bw, float bh,
                 sf::Color fill, sf::Color border, float thickness = 2.f);

    // ===== 数据成员 =====
    Player* m_player = nullptr;
    std::unique_ptr<Enemy> m_enemy;

    CombatPhase m_phase = CombatPhase::PlayerTurn;
    CombatResult m_result = CombatResult::Ongoing;
    int m_round = 0;

    // 主菜单光标
    int m_mainCursor = 0;       // 0=攻击 1=技能 2=道具 3=逃跑
    int m_skillCursor = 0;
    int m_itemCursor = 0;

    // 技能列表（从功法动态构建）
    std::vector<CombatSkill> m_skills;

    // 道具列表（背包中的消耗品）
    struct UsableItem {
        std::string itemId;
        std::wstring name;
        int count = 0;
        std::wstring effect;    // 效果描述文字
        EffectType effectType = EffectType::None;
        int effectValue = 0;
    };
    std::vector<UsableItem> m_items;

    // 消息队列（逐条显示，按键继续）
    std::vector<std::wstring> m_msgQueue;
    std::wstring m_currentMsg;
    CombatPhase m_afterMsgPhase = CombatPhase::PlayerTurn;

    // 奖励
    int m_expReward = 10;
    int m_goldReward = 5;

    // 动画
    float m_animTimer = 0.f;
    float m_hitShakeTimer = 0.f;  // 受击抖动
    bool m_playerHit = false;
    bool m_enemyHit = false;

    // 字体
    sf::Font m_font;

    // 战斗日志（底部滚动，保留最近6条）
    std::vector<std::wstring> m_battleLog;
    void AddLog(const std::wstring& line);

    // 粒子特效（简单闪光）
    struct Particle {
        float x, y, vx, vy, life, maxLife;
        sf::Color color;
    };
    std::vector<Particle> m_particles;
    void SpawnHitParticles(float cx, float cy, sf::Color col);
    void UpdateParticles(float dt);
    void RenderParticles(sf::RenderWindow& w);
};
