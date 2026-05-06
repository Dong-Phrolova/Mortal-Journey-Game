#pragma once
#include "GameState.h"
#include <SFML/Graphics.hpp>

class SettingsState : public GameState {
public:
    SettingsState();

    void Enter()  override;
    void Exit()   override;
    void Update(float dt) override;
    void HandleInput() override {}
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    void SaveToSlot(int slot);
    void LoadFromSlot(int slot);
    void ReturnToMainMenu();
    void QuitGame();

    sf::Font m_font;

    sf::Text m_titleText;
    sf::Text m_hintText;
    sf::Text m_statusText;
    sf::Text m_slotLabels[3];
    sf::Text m_slotInfo[3];

    int m_selectedItem = 0;

    float m_statusTimer = 0.f;
    bool m_pendingLoad = false;

    // 菜单项说明
    // 0-2: 存档槽位 3-4: 音量 5: 返回主菜单 6: 退出游戏
    static constexpr int MENU_ITEMS = 7;
};
