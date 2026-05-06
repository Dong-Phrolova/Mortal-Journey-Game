#pragma execution_character_set("utf-8")
#include "GameState.h"
#include <algorithm>

// ============ GameStateManager ============
GameStateManager& GameStateManager::Instance() {
    static GameStateManager inst;
    return inst;
}

void GameStateManager::PushState(std::unique_ptr<GameState> state) {
    if (!m_stack.empty()) m_stack.back()->Exit();
    state->Enter();
    m_stack.push_back(std::move(state));
}

void GameStateManager::PopState() {
    if (m_stack.empty()) return;
    m_stack.back()->Exit();
    m_stack.pop_back();
    if (!m_stack.empty()) m_stack.back()->Enter();
}

void GameStateManager::SwitchState(std::unique_ptr<GameState> state) {
    while (!m_stack.empty()) {
        m_stack.back()->Exit();
        m_stack.pop_back();
    }
    state->Enter();
    m_stack.push_back(std::move(state));
}

GameState* GameStateManager::GetCurrentState() const {
    return m_stack.empty() ? nullptr : m_stack.back().get();
}

GameStateType GameStateManager::GetCurrentStateType() const {
    if (m_stack.empty()) return GameStateType::MainMenu;
    return m_stack.back()->GetType();
}

void GameStateManager::Update(float dt) {
    if (!m_stack.empty()) m_stack.back()->Update(dt);
}

void GameStateManager::HandleInput() {
    if (!m_stack.empty()) m_stack.back()->HandleInput();
}

void GameStateManager::Render(sf::RenderWindow& window) {
    // 从底到顶渲染所有状态（支持叠加层如暂停菜单）
    for (const auto& s : m_stack) {
        s->Render(window);
    }
}
