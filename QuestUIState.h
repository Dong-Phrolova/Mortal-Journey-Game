#pragma once
#pragma execution_character_set("utf-8")
#include "GameState.h"
#include "QuestSystem.h"
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================
//  任务界面（按 Q 键打开）
//  显示当前主线任务列表、任务详情、奖励预览
// ============================================================
class QuestUIState : public GameState {
public:
    QuestUIState();

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override {}
    void Render(sf::RenderWindow& window) override;
    void OnKeyPressed(sf::Keyboard::Key key);

private:
    sf::Font m_font;

    // UI 元素
    sf::Text m_titleText;           // 标题
    sf::Text m_questListText[10];   // 任务列表（最多显示10个）
    sf::Text m_detailTitle;         // 详情标题
    sf::Text m_detailLines[8];      // 详情行
    sf::Text m_rewardLabel;         // 奖励标签
    sf::Text m_rewardLines[4];      // 奖励内容
    sf::Text m_hintText;           // 操作提示
    sf::Text m_emptyHint;          // 无任务提示

    int m_selectedIndex = 0;        // 当前选中的任务索引
    float m_animTimer = 0.f;

    // 辅助方法
    std::wstring StatusString(QuestStatus status) const;
    void RenderQuestList(sf::RenderWindow& window, int& activeCount);
    void RenderQuestDetail(sf::RenderWindow& window, const QuestData* quest);
};
