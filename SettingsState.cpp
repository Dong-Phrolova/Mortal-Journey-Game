#pragma execution_character_set("utf-8")
#include "SettingsState.h"
#include "GameState.h"
#include "MainMenuState.h"
#include "WorldMapState.h"
#include "AudioManager.h"
#include "GameSession.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

static std::string SlotPath(int slot) {
    return "save_slot" + std::to_string(slot) + ".json";
}

// 地图 ID → 中文名称映射
static std::wstring MapIdToChinese(const std::string& id) {
    static const std::map<std::string, std::wstring> MAP_NAMES = {
        {"qixuanmen",      L"七玄门·外门"},
        {"qingniu_town",   L"青牛镇"},
        {"jiazhou",        L"嘉州城"},
        {"jiazhou_market", L"嘉州城·市场"},
        {"huangfengu",     L"黄枫谷"},
        {"xuese",          L"血色禁地"},
        {"shenshu_valley", L"神手谷"},
        {"qixuanmen_back", L"七玄门·后山"},
        {"caixia_mountain",L"彩霞山"},
        {"liangu_cliff",   L"炼骨崖"},
        {"luanshenghai",   L"乱星海"},
    };
    auto it = MAP_NAMES.find(id);
    return (it != MAP_NAMES.end()) ? it->second : std::wstring(id.begin(), id.end());
}

// 修炼境界 → 中文
static std::wstring RealmToChinese(int major, int qiLayer) {
    static const std::wstring realms[] = {
        L"凡人", L"练气", L"筑基", L"结丹", L"元婴", L"化神"
    };
    if (major < 0 || major > 5) return L"???";
    return realms[major] + L"·" + std::to_wstring(qiLayer) + L"层";
}

static std::wstring SlotLabel(int slot) {
    std::string path = SlotPath(slot);
    std::error_code ec;
    auto ftime = std::filesystem::last_write_time(path, ec);
    if (ec) return L"(空)";
    // 尝试从存档读取基本信息
    std::ifstream ifs(path);
    if (!ifs.is_open()) return L"(空)";
    nlohmann::json j;
    try { ifs >> j; } catch (...) { return L"(空)"; }
    if (!j.contains("currentMap")) return L"(空)";
    std::string mapName = j["currentMap"].get<std::string>();
    std::wstring mapCN = MapIdToChinese(mapName);
    int gold = j.contains("player") && j["player"].contains("gold") ? j["player"]["gold"].get<int>() : 0;

    // 读取修炼等级
    int major = 0, qiLayer = 1;
    if (j.contains("culti")) {
        major = j["culti"].value("major", 0);
        qiLayer = j["culti"].value("qiLayer", 1);
    }
    std::wstring realmStr = RealmToChinese(major, qiLayer);

    // 读取任务进度
    int questDone = 0, questTotal = 0;
    if (j.contains("quests")) {
        for (const auto& qj : j["quests"]) {
            questTotal++;
            int status = qj.value("status", 0);
            if (status >= 2) questDone++;  // Completed(2) or Rewarded(3)
        }
    }

    std::wstring info = L"  " + mapCN;
    info += L"  |  " + realmStr;
    info += L"  |  任务 " + std::to_wstring(questDone) + L"/" + std::to_wstring(questTotal);
    info += L"  |  灵石:" + std::to_wstring(gold);
    return info;
}

SettingsState::SettingsState() {
    m_type = GameStateType::Settings;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    m_titleText.setFont(m_font);
    m_titleText.setString(L"— 菜  单 —");
    m_titleText.setCharacterSize(30);
    m_titleText.setFillColor(sf::Color(200, 180, 120));
    m_titleText.setPosition(330.f, 80.f);

    // 存档槽位标签
    float startY = 135.f;
    for (int i = 0; i < 3; ++i) {
        m_slotLabels[i].setFont(m_font);
        m_slotLabels[i].setCharacterSize(15);
        m_slotLabels[i].setPosition(200.f, startY + i * 60.f);

        m_slotInfo[i].setFont(m_font);
        m_slotInfo[i].setCharacterSize(13);
        m_slotInfo[i].setFillColor(sf::Color(130, 130, 150));
        m_slotInfo[i].setPosition(200.f, startY + i * 60.f + 22.f);
    }

    m_hintText.setFont(m_font);
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(120, 120, 130));
    m_hintText.setPosition(140.f, 540.f);

    m_statusText.setFont(m_font);
    m_statusText.setCharacterSize(16);
    m_statusText.setPosition(300.f, 510.f);
}

void SettingsState::Enter() {
    m_selectedItem = 0;
    m_statusTimer = 0.f;

    // 刷新存档槽位信息
    for (int i = 0; i < 3; ++i) {
        auto info = SlotLabel(i);
        m_slotLabels[i].setString(L"存档 " + std::to_wstring(i + 1));
        m_slotInfo[i].setString(info);
    }
}

void SettingsState::Exit() {
    AudioManager::Instance().SaveSettings();
}

void SettingsState::Update(float dt) {
    if (m_statusTimer > 0.f) m_statusTimer -= dt;
    if (m_pendingLoad) {
        m_pendingLoad = false;
        GameStateManager::Instance().SwitchState(std::make_unique<WorldMapState>());
    }
}

void SettingsState::OnKeyPressed(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::Up:
        m_selectedItem = (m_selectedItem - 1 + MENU_ITEMS) % MENU_ITEMS;
        break;
    case sf::Keyboard::Down:
        m_selectedItem = (m_selectedItem + 1) % MENU_ITEMS;
        break;
    case sf::Keyboard::Return:
    case sf::Keyboard::Space:
        if (m_selectedItem >= 0 && m_selectedItem <= 2) {
            // 存档槽：同时存档和读档功能
            // Enter 键存档
            SaveToSlot(m_selectedItem);
        } else if (m_selectedItem == 3) {
            // 返回游戏
            GameStateManager::Instance().PopState();
        } else if (m_selectedItem == 4) {
            // 返回主菜单
            ReturnToMainMenu();
            return;
        } else if (m_selectedItem == 5) {
            // 退出游戏
            QuitGame();
            return;
        }
        break;
    case sf::Keyboard::Escape:
        GameStateManager::Instance().PopState();
        break;
    case sf::Keyboard::L: {
        // 加载选中槽位
        LoadFromSlot(m_selectedItem);
        break;
    }
    default:
        break;
    }
}

void SettingsState::SaveToSlot(int slot) {
    std::string path = SlotPath(slot);
    GameSession::Instance().SaveGame(path);
    m_statusText.setString(L"✓ 已保存到存档 " + std::to_wstring(slot + 1));
    m_statusText.setFillColor(sf::Color(100, 230, 130));
    m_statusTimer = 2.5f;
    // 刷新信息
    auto info = SlotLabel(slot);
    m_slotInfo[slot].setString(info);
}

void SettingsState::LoadFromSlot(int slot) {
    std::string path = SlotPath(slot);
    if (GameSession::Instance().LoadGame(path)) {
        m_statusText.setString(L"✓ 读取了存档 " + std::to_wstring(slot + 1) + L"，进入游戏");
        m_statusText.setFillColor(sf::Color(100, 200, 255));
        m_statusTimer = 2.5f;
        // 延迟切换到游戏（下次Update检查）
        m_pendingLoad = true;
    } else {
        m_statusText.setString(L"✗ 存档 " + std::to_wstring(slot + 1) + L" 不存在");
        m_statusText.setFillColor(sf::Color(230, 120, 100));
        m_statusTimer = 2.5f;
    }
}

void SettingsState::ReturnToMainMenu() {
    GameStateManager::Instance().SwitchState(std::make_unique<MainMenuState>());
}

void SettingsState::QuitGame() {
    // 退出前自动存档
    GameSession::Instance().SaveGame("save_slot0.json");
    GameStateManager::Instance().RequestExit();
}

void SettingsState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(18, 18, 42));

    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(10, 10, 25, 200));
    window.draw(overlay);

    // 主面板
    sf::RectangleShape panel(sf::Vector2f(500.f, 420.f));
    panel.setFillColor(sf::Color(22, 28, 45, 240));
    panel.setOutlineThickness(1.5f);
    panel.setOutlineColor(sf::Color(60, 70, 90));
    panel.setPosition(150.f, 55.f);
    window.draw(panel);

    window.draw(m_titleText);

    float startY = 135.f;

    // 存档槽位 0-2
    for (int i = 0; i < 3; ++i) {
        bool sel = (m_selectedItem == i);
        m_slotLabels[i].setFillColor(sel ? sf::Color(255, 220, 100) : sf::Color(185, 190, 200));
        m_slotLabels[i].setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        // 加选择器
        std::wstring prefix = sel ? L"▶  " : L"   ";
        m_slotLabels[i].setString(prefix + L"存档 " + std::to_wstring(i + 1));
        window.draw(m_slotLabels[i]);
        window.draw(m_slotInfo[i]);

        // 分隔线
        if (i < 2) {
            sf::RectangleShape sep(sf::Vector2f(400.f, 0.5f));
            sep.setPosition(200.f, startY + i * 60.f + 52.f);
            sep.setFillColor(sf::Color(60, 70, 90, 150));
            window.draw(sep);
        }
    }

    // 菜单选项 3-5
    struct MenuItem { std::wstring label; };
    MenuItem items[] = {
        { L"返回游戏" },
        { L"返回主菜单" },
        { L"退出游戏" }
    };

    for (int i = 0; i < 3; ++i) {
        int idx = 3 + i;
        bool sel = (m_selectedItem == idx);
        sf::Text item;
        item.setFont(m_font);
        item.setString((sel ? L"▶  " : L"   ") + items[i].label);
        item.setCharacterSize(15);
        item.setFillColor(sel ? sf::Color(255, 220, 100) : sf::Color(185, 190, 200));
        item.setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        item.setPosition(200.f, startY + 195.f + i * 40.f);
        window.draw(item);
    }

    // 提示
    m_hintText.setString(L"↑↓ 选择  Enter 存档  L键读档  Esc返回游戏");
    window.draw(m_hintText);

    // 状态提示
    if (m_statusTimer > 0.f) {
        window.draw(m_statusText);
    }
}
