#pragma execution_character_set("utf-8")
#include "MainMenuState.h"
#include "GameState.h"
#include "GameCallbacks.h"
#include "GameSession.h"
#include <cmath>

MainMenuState::MainMenuState() {
    m_type = GameStateType::MainMenu;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    // 标题
    m_title.setFont(m_font);
    m_title.setString(L"凡 人 修 仙 传");
    m_title.setCharacterSize(52);
    m_title.setFillColor(sf::Color(220, 190, 100));
    sf::FloatRect bounds = m_title.getLocalBounds();
    m_title.setPosition((800 - bounds.width) / 2.f, 150.f);

    const wchar_t* labels[4] = {
        L"新 游 戏", L"继 续 游 戏", L"设 置", L"退 出 游 戏"
    };
    for (int i = 0; i < 4; ++i) {
        m_options[i].setFont(m_font);
        m_options[i].setString(labels[i]);
        m_options[i].setCharacterSize(26);
        m_options[i].setPosition(300.f, 290.f + i * 55.f);
    }

    m_hint.setFont(m_font);
    m_hint.setString(L"↑↓ 选择   Enter 确认");
    m_hint.setCharacterSize(16);
    m_hint.setFillColor(sf::Color(140, 140, 140));
    m_hint.setPosition(280.f, 545.f);

    m_version.setFont(m_font);
    m_version.setString(L"v0.2 α");
    m_version.setCharacterSize(14);
    m_version.setFillColor(sf::Color(90, 90, 90));
    m_version.setPosition(640.f, 578.f);

    m_saveInfo.setFont(m_font);
    m_saveInfo.setCharacterSize(14);
    m_saveInfo.setFillColor(sf::Color(120, 160, 120));
    m_saveInfo.setPosition(300.f, 340.f); // 继续游戏选项下方
}

void MainMenuState::Enter() {
    m_selected = 0;
    m_animTimer = 0.f;
    // 检测是否有存档文件
    m_hasSave = GameSession::Instance().HasSaveFile("save.json");
}

void MainMenuState::Update(float dt) {
    m_animTimer += dt;
    for (int i = 0; i < 4; ++i) {
        if (i == m_selected) {
            float a = 0.7f + 0.3f * std::sin(m_animTimer * 4.f);
            m_options[i].setFillColor(sf::Color(
                (sf::Uint8)(255 * a), (sf::Uint8)(215 * a), (sf::Uint8)(80 * a)));
            m_options[i].setStyle(sf::Text::Bold);
        } else {
            // 如无存档，"继续游戏"显示为灰色
            if (i == Continue && !m_hasSave) {
                m_options[i].setFillColor(sf::Color(90, 90, 90));
            } else {
                m_options[i].setFillColor(sf::Color(170, 170, 170));
            }
            m_options[i].setStyle(sf::Text::Regular);
        }
    }
}

void MainMenuState::HandleInput() {}

void MainMenuState::OnKeyPressed(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Up:   m_selected = (m_selected - 1 + 4) % 4; break;
        case sf::Keyboard::Down: m_selected = (m_selected + 1) % 4; break;
        case sf::Keyboard::Return: OnConfirm(); break;
        default: break;
    }
}

void MainMenuState::OnConfirm() {
    switch (m_selected) {
        case NewGame:
            OnNewGameSelected();
            break;
        case Continue:
            if (m_hasSave) {
                OnContinueGameSelected();
            }
            // 没有存档时不响应
            break;
        case Settings:
            OnOpenSettings();
            break;
        case Quit:
            GameStateManager::Instance().RequestExit();
            break;
    }
}

void MainMenuState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(18, 18, 42));

    // 装饰线
    sf::RectangleShape line(sf::Vector2f(500.f, 2.f));
    line.setFillColor(sf::Color(90, 70, 45, 160));
    line.setPosition(150.f, 240.f); window.draw(line);
    line.setPosition(150.f, 520.f); window.draw(line);

    window.draw(m_title);

    // 光标
    sf::Text cursor(L"▶", m_font, 22);
    cursor.setFillColor(sf::Color(220, 190, 80));
    cursor.setPosition(260.f, 293.f + m_selected * 55.f);
    window.draw(cursor);

    for (int i = 0; i < 4; ++i)
        window.draw(m_options[i]);

    // 存档状态提示（在"继续游戏"旁）
    if (!m_hasSave) {
        m_saveInfo.setString(L"（暂无存档）");
        m_saveInfo.setFillColor(sf::Color(120, 100, 100));
        m_saveInfo.setPosition(460.f, 342.f);
        window.draw(m_saveInfo);
    } else {
        m_saveInfo.setString(L"✓ 存档可用");
        m_saveInfo.setFillColor(sf::Color(100, 200, 120));
        m_saveInfo.setPosition(460.f, 342.f);
        window.draw(m_saveInfo);
    }

    window.draw(m_hint);
    window.draw(m_version);
}
