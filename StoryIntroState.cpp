#pragma execution_character_set("utf-8")
#include "StoryIntroState.h"
#include "GameState.h"
#include "ConfigManager.h"
#include <cmath>
#include "GameSession.h"
#include "WorldMapState.h"

StoryIntroState::StoryIntroState() {
    m_type = GameStateType::Dialogue;  // 复用Dialogue类型
    m_currentPage = 0;
    m_textTimer = 0.f;
    m_showContinue = false;
}

void StoryIntroState::Enter() {
    // 加载字体
    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    // 初始化剧情页面
    m_storyPages.clear();
    m_storyPages.push_back(
        L"凡人修仙传\n\n"
        L"韩立，一个普通的山村少年，\n"
        L"因资质平平本无缘仙道。\n\n"
        L"然而命运弄人，他被送入七玄门，\n"
        L"成为了墨大夫的弟子..."
    );

    m_storyPages.push_back(
        L"七玄门·外门\n\n"
        L"七玄门是越国的一个小门派，\n"
        L"虽不入流，却也是韩立唯一的希望。\n\n"
        L"墨大夫声称要传授他功法，\n"
        L"但韩立总觉得这位师父...\n"
        L"似乎隐藏着什么秘密。"
    );

    m_storyPages.push_back(
        L"第一章：初入仙途\n\n"
        L"今天是韩立入门的第三天。\n"
        L"墨大夫说要开始传授他修炼口诀，\n"
        L"但奇怪的是，师父让他先去...\n"
        L"后山采药。\n\n"
        L"按 Space/Enter 开始游戏"
    );

    m_currentPage = 0;
    m_textTimer = 0.f;
    m_showContinue = false;

    // 设置UI
    m_bgBox.setSize(sf::Vector2f(740.f, 500.f));
    m_bgBox.setFillColor(sf::Color(10, 12, 20, 245));
    m_bgBox.setOutlineThickness(2.f);
    m_bgBox.setOutlineColor(sf::Color(80, 100, 140));

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(28);
    m_titleText.setFillColor(sf::Color(255, 215, 120));

    m_storyText.setFont(m_font);
    m_storyText.setCharacterSize(18);
    m_storyText.setFillColor(sf::Color(220, 220, 210));
    m_storyText.setLineSpacing(1.5f);

    m_continueText.setFont(m_font);
    m_continueText.setCharacterSize(14);
    m_continueText.setFillColor(sf::Color(150, 150, 150));
}

void StoryIntroState::Update(float dt) {
    m_textTimer += dt;

    // 文字显示一段时间后，显示继续提示
    if (m_textTimer > 0.5f) {
        m_showContinue = true;
    }
}

void StoryIntroState::HandleInput() {
    // 由 OnKeyPressed 处理
}

void StoryIntroState::OnKeyPressed(sf::Keyboard::Key key) {
    if (!m_showContinue) return;  // 文字还没显示完，不接受输入

    switch (key) {
    case sf::Keyboard::Space:
    case sf::Keyboard::Return: {
        m_currentPage++;
        if (m_currentPage >= (int)m_storyPages.size()) {
            // 剧情结束，初始化游戏并进入地图
            GameSession::Instance().NewGame();
            GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
            return;
        }
        m_textTimer = 0.f;
        m_showContinue = false;
        break;
    }
    case sf::Keyboard::Escape: {
        // 跳过剧情，直接初始化游戏并进入地图
        GameSession::Instance().NewGame();
        GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
        break;
    }
    default:
        break;
    }
}

void StoryIntroState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(10, 12, 20));

    // 背景框
    m_bgBox.setPosition(30.f, 50.f);
    window.draw(m_bgBox);

    // 标题
    if (m_currentPage == 0) {
        m_titleText.setString(L"凡人修仙传");
        m_titleText.setPosition(250.f, 80.f);
        window.draw(m_titleText);
    }

    // 剧情文字
    m_storyText.setString(m_storyPages[m_currentPage]);
    m_storyText.setPosition(80.f, 180.f);
    window.draw(m_storyText);

    // 继续提示
    if (m_showContinue && m_currentPage < (int)m_storyPages.size() - 1) {
        m_continueText.setString(L"按 Space/Enter 继续 — Esc 跳过");
        m_continueText.setPosition(280.f, 520.f);
        window.draw(m_continueText);
    } else if (m_showContinue && m_currentPage == (int)m_storyPages.size() - 1) {
        m_continueText.setString(L"按 Space/Enter 开始游戏 — Esc 跳过");
        m_continueText.setPosition(280.f, 520.f);
        window.draw(m_continueText);
    }
}
