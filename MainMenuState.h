#pragma once
#include "GameState.h"
#include <SFML/Graphics.hpp>

class MainMenuState : public GameState {
public:
    MainMenuState();

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override;
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    enum MenuOption { NewGame, Continue, Settings, Quit };
    int m_selected = 0;
    bool m_hasSave = false;   // 是否有存档文件

    sf::Font m_font;
    sf::Text m_title;
    sf::Text m_options[4];
    sf::Text m_hint;
    sf::Text m_version;
    sf::Text m_saveInfo;      // 存档状态提示

    float m_animTimer = 0.f;

    void OnConfirm();
};
