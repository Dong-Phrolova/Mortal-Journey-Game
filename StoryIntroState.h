#pragma execution_character_set("utf-8")
#pragma once
#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class StoryIntroState : public GameState {
public:
    StoryIntroState();
    virtual ~StoryIntroState() {}

    void Enter();
    void Update(float dt);
    void HandleInput();
    void OnKeyPressed(sf::Keyboard::Key key);
    void Render(sf::RenderWindow& window);

private:
    sf::Font m_font;
    std::vector<std::wstring> m_storyPages;  // 剧情页面
    int m_currentPage;                         // 当前页面索引
    float m_textTimer;                         // 文字显示计时器
    bool m_showContinue;                       // 是否显示"继续"提示

    sf::Text m_titleText;
    sf::Text m_storyText;
    sf::Text m_continueText;
    sf::RectangleShape m_bgBox;
};
