#pragma execution_character_set("utf-8")
#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include "CultivationSystem.h"

// ============ 游戏状态 ============
enum class GameStateType {
    MainMenu,       // 主菜单
    WorldMap,       // 大地图探索
    Combat,         // 回合制战斗
    Dialogue,       // 对话/剧情
    Inventory,      // 背包/物品
    CultivationUI,  // 修炼界面
    Shop,           // 商店
    QuestUI,        // 任务界面
    Settings,       // 设置界面
    Paused,         // 暂停
    GameOver,       // 死亡
    Victory         // 胜利结算
};

// 状态基类
class GameState {
public:
    virtual ~GameState() = default;
    virtual void Enter() {}
    virtual void Exit() {}
    virtual void Update(float dt) = 0;
    virtual void HandleInput() = 0;
    virtual void Render(class sf::RenderWindow& window) = 0;

    GameStateType GetType() const { return m_type; }

protected:
    GameStateType m_type;
};

// 状态管理器（单例）
class GameStateManager {
public:
    static GameStateManager& Instance();

    void PushState(std::unique_ptr<GameState> state);
    void PopState();
    void SwitchState(std::unique_ptr<GameState> state);
    GameState* GetCurrentState() const;
    GameStateType GetCurrentStateType() const;

    // 请求退出游戏
    void RequestExit() { m_exitRequested = true; }
    bool IsExitRequested() const { return m_exitRequested; }

    void Update(float dt);
    void HandleInput();
    void Render(sf::RenderWindow& window);

private:
    std::vector<std::unique_ptr<GameState>> m_stack;
    bool m_exitRequested = false;
};

