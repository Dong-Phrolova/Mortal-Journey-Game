#pragma execution_character_set("utf-8")
#include <SFML/Graphics.hpp>
#include <iostream>
#include <locale>
#include <cstdlib>
#include <ctime>
#include <windows.h>   // GetModuleFileNameW / SetCurrentDirectoryW
#include "GameState.h"
#include "MainMenuState.h"
#include "WorldMapState.h"
#include "CombatState.h"
#include "DialogueState.h"
#include "CultivationUIState.h"
#include "InventoryState.h"
#include "SettingsState.h"
#include "GameCallbacks.h"
#include "ConfigManager.h"
#include "AudioManager.h"
#include "GameSession.h"
#include "ShopState.h"
#include "QuestUIState.h"
#include "StoryIntroState.h"

int main() {
    // 把工作目录切换到 exe 所在目录，确保 songs/、config/ 等相对路径可用
    {
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        // 截断到最后一个反斜杠，得到目录
        for (int i = (int)wcslen(exePath) - 1; i >= 0; --i) {
            if (exePath[i] == L'\\' || exePath[i] == L'/') {
                exePath[i] = L'\0';
                break;
            }
        }
        SetCurrentDirectoryW(exePath);
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // 加载配置
    if (!ConfigManager::Instance().LoadAll("config")) {
        std::cerr << "[ERROR] 配置加载失败！" << std::endl;
        return -1;
    }

    // 创建窗口（中文支持）
    sf::RenderWindow window(sf::VideoMode(800, 600), L"凡人修仙传 RPG");
    window.setFramerateLimit(60);

    // 初始化音频：启动 BGM
    AudioManager::Instance().PlayBGM("songs/bgufan.wav");

    // 初始化状态机：从主菜单开始
    auto& gsm = GameStateManager::Instance();
    gsm.PushState(std::make_unique<MainMenuState>());

    sf::Clock deltaClock;
    bool wasCombatEnded = false;   // 用于战斗结束后自动存档

    // ===== 主游戏循环 =====
    while (window.isOpen() && !gsm.IsExitRequested()) {
        float dt = deltaClock.restart().asSeconds();

        // 事件处理
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // 退出时自动存档（仅当游戏进行中）
                if (gsm.GetCurrentStateType() != GameStateType::MainMenu) {
                    GameSession::Instance().SaveGame("save.json");
                }
                window.close();
                break;
            }

            if (event.type == sf::Event::KeyPressed) {
                auto* state = gsm.GetCurrentState();
                if (state) {
                    if (auto* menu = dynamic_cast<MainMenuState*>(state)) {
                        menu->OnKeyPressed(event.key.code);
                    } else if (auto* map = dynamic_cast<WorldMapState*>(state)) {
                        map->OnKeyPressed(event.key.code);
                    } else if (auto* combat = dynamic_cast<CombatState*>(state)) {
                        // 战斗结束按 Enter 退出时检查是否需要存档
                        bool wasEnd = (combat->GetPhase() == CombatPhase::CombatEnd);
                        combat->OnKeyPressed(event.key.code);
                        // 如果状态发生切换（战斗已弹出），自动存档
                        if (wasEnd && gsm.GetCurrentState() != state) {
                            GameSession::Instance().SaveGame("save.json");
                        }
                    } else if (auto* dialogue = dynamic_cast<DialogueState*>(state)) {
                        dialogue->OnKeyPressed(event.key.code);
                    } else if (auto* culti = dynamic_cast<CultivationUIState*>(state)) {
                        culti->OnKeyPressed(event.key.code);
                    } else if (auto* inv = dynamic_cast<InventoryState*>(state)) {
                        inv->OnKeyPressed(event.key.code);
                    } else if (auto* settings = dynamic_cast<SettingsState*>(state)) {
                        settings->OnKeyPressed(event.key.code);
                    } else if (auto* shop = dynamic_cast<ShopState*>(state)) {
                        shop->OnKeyPressed(event.key.code);
                    } else if (auto* questUI = dynamic_cast<QuestUIState*>(state)) {
                        questUI->OnKeyPressed(event.key.code);
                    } else if (auto* story = dynamic_cast<StoryIntroState*>(state)) {
                        story->OnKeyPressed(event.key.code);
                    }
                } else {
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                }
            }
        }

        // 更新 + 渲染
        gsm.Update(dt);
        gsm.Render(window);

        window.display();
    }

    // 退出前保存音量设置
    AudioManager::Instance().SaveSettings();

    return 0;
}
