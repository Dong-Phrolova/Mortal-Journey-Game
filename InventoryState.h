#pragma once
#include "GameState.h"
#include "ItemSystem.h"
#include <SFML/Graphics.hpp>

class InventoryState : public GameState {
public:
    InventoryState();

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override;
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    void UseSelectedItem();
    void DiscardSelectedItem();
    void EquipSelectedItem();
    void SellSelectedItem();

    InventorySystem* m_inventory;
    int m_selected = 0;
    int m_scrollOffset = 0;   // 滚动偏移量

    sf::Font m_font;
    sf::Text m_titleText;
    sf::Text m_itemList[12];     // 可见列表
    sf::Text m_itemDetail;       // 物品详情
    sf::Text m_goldText;
    sf::Text m_hintText;
    sf::Text m_msgText;
    float m_msgTimer = 0.f;
};
