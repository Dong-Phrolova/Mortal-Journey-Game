#pragma execution_character_set("utf-8")
#include "Combat.h"
#include <iostream>
#include <limits>

// ============ 控制台版战斗（兼容旧逻辑）============
void StartBattle(Player& player, Enemy& enemy) {
    std::wcout << L"\n========== 战斗开始 ==========\n";
    std::wcout << player.GetName() << L" VS " << enemy.GetName() << L"！\n\n";

    while (player.IsAlive() && enemy.IsAlive()) {
        // 玩家回合
        std::wcout << L"--- " << player.GetName() << L" 的回合 ---\n";
        std::wcout << L"生命值: " << player.GetCurrentHp() << L"/" << player.GetMaxHp()
                  << L" | 法力值: " << player.GetCurrentMp() << L"/" << player.GetMaxMp() << L"\n";
        std::wcout << enemy.GetName() << L" 生命值: " << enemy.GetHp() << L"/" << enemy.GetMaxHp() << L"\n";
        std::wcout << L"\n1.普通攻击  2.疗伤(消耗20法力)  3.重击(消耗30法力)\n选择: ";

        int choice;
        std::wcin >> choice;

        if (choice == 1) {
            enemy.TakeDamage(player.GetAttack());
        } else if (choice == 2) {
            if (player.GetCurrentMp() >= 20) {
                player.SetCurrentMp(player.GetCurrentMp() - 20);
                player.SetCurrentHp(player.GetCurrentHp() + 30);
                std::wcout << player.GetName() << L" 恢复了30点生命！\n";
            } else {
                std::wcout << L"法力不足！\n";
            }
        } else if (choice == 3) {
            if (player.GetCurrentMp() >= 30) {
                player.SetCurrentMp(player.GetCurrentMp() - 30);
                enemy.TakeDamage(player.GetAttack() * 2);
            } else {
                std::wcout << L"法力不足！\n";
            }
        }

        if (!enemy.IsAlive()) {
            std::wcout << L"\n========== 胜利！==========\n";
            break;
        }

        // 敌人回合
        int act = enemy.ChooseAction();
        if (act == 0) {
            player.TakeDamage(enemy.GetAttack());
        } else if (act == 1) {
            std::wcout << enemy.GetName() << L" 进行防御！\n";
            player.TakeDamage(enemy.GetAttack() / 2);
        } else {
            std::wcout << enemy.GetName() << L" 使用特殊攻击！\n";
            player.TakeDamage(enemy.GetAttack() * 2);
        }

        if (!player.IsAlive()) {
            std::wcout << L"\n========== 失败 ==========\n";
            break;
        }
        std::wcout << L"\n";
    }
}

// ============ SFML 版战斗（新）============

static sf::Font g_font;
static CombatUIData g_state;

// 前向声明
static void ProcessPlayerAction(Player& p, Enemy& e, CombatUIData& s);
static void ProcessEnemyAction(Player& p, Enemy& e, CombatUIData& s);

static void DrawText(sf::RenderWindow& w, const std::wstring& s,
                     float x, float y, unsigned sz, const sf::Color& c) {
    sf::Text t(s, g_font, sz);
    t.setFillColor(c);
    t.setPosition(x, y);
    w.draw(t);
}

bool InitCombatUI() {
    return g_font.loadFromFile("C:/Windows/Fonts/simsun.ttc") ||
           g_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");
}

CombatResult RunCombat(Player& player, Enemy& enemy, sf::RenderWindow& window) {
    g_state = CombatUIData{};
    g_state.inCombat = true;

    if (!InitCombatUI()) return CombatResult::Defeat;

    while (g_state.inCombat && window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window.close();
                return CombatResult::Defeat;
            }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Num1 || ev.key.code == sf::Keyboard::Numpad1)
                    g_state.playerChoice = 0;
                else if (ev.key.code == sf::Keyboard::Num2 || ev.key.code == sf::Keyboard::Numpad2)
                    g_state.playerChoice = 1;
                else if (ev.key.code == sf::Keyboard::Num3 || ev.key.code == sf::Keyboard::Numpad3)
                    g_state.playerChoice = 2;
            }
        }

        // 处理玩家选择
        if (g_state.playerChoice >= 0 && g_state.enemyAction < 0) {
            ProcessPlayerAction(player, enemy, g_state);
            g_state.enemyAction = enemy.ChooseAction();
            ProcessEnemyAction(player, enemy, g_state);
            g_state.round++;
            g_state.playerChoice = -1;
            g_state.enemyAction  = -1;

            if (!enemy.IsAlive()) { g_state.inCombat = false; return CombatResult::Victory; }
            if (!player.IsAlive()) { g_state.inCombat = false; return CombatResult::Defeat;  }
        }

        window.clear(sf::Color(30, 30, 50));
        // HUD
        DrawText(window, L"回合: " + std::to_wstring(g_state.round), 20, 20, 20, sf::Color::White);
        DrawText(window, L"【" + player.GetName() + L"】", 20, 60, 24, sf::Color::Cyan);
        DrawText(window, L"HP: " + std::to_wstring(player.GetCurrentHp()) + L"/" + std::to_wstring(player.GetMaxHp()),
                 20, 90, 18, sf::Color::Green);
        DrawText(window, L"MP: " + std::to_wstring(player.GetCurrentMp()) + L"/" + std::to_wstring(player.GetMaxMp()),
                 20, 115, 18, sf::Color::Blue);
        DrawText(window, L"境界: " + player.GetRealmName(), 20, 140, 16, sf::Color(200,200,255));

        DrawText(window, L"【" + enemy.GetName() + L"】", 450, 60, 24, sf::Color::Red);
        DrawText(window, L"HP: " + std::to_wstring(enemy.GetHp()) + L"/" + std::to_wstring(enemy.GetMaxHp()),
                 450, 90, 18, sf::Color::Yellow);

        // 选项
        DrawText(window, L"1.攻击  2.疗伤(20MP)  3.重击(30MP)", 20, 500, 22, sf::Color::White);
        DrawText(window, g_state.logLine, 20, 540, 16, sf::Color(200,200,200));

        window.display();
    }
    return CombatResult::Ongoing;
}

static void ProcessPlayerAction(Player& p, Enemy& e, CombatUIData& s) {
    if (s.playerChoice == 0) {
        e.TakeDamage(p.GetAttack());
        s.logLine = p.GetName() + L" 发动攻击！造成 " + std::to_wstring(p.GetAttack()) + L" 伤害";
    } else if (s.playerChoice == 1) {
        if (p.GetCurrentMp() >= 20) {
            p.SetCurrentMp(p.GetCurrentMp() - 20);
            p.SetCurrentHp(std::min(p.GetCurrentHp() + 30, p.GetMaxHp()));
            s.logLine = p.GetName() + L" 使用疗伤，恢复30点生命";
        } else {
            s.logLine = L"法力不足！";
        }
    } else if (s.playerChoice == 2) {
        if (p.GetCurrentMp() >= 30) {
            p.SetCurrentMp(p.GetCurrentMp() - 30);
            e.TakeDamage(p.GetAttack() * 2);
            s.logLine = p.GetName() + L" 使出重击！造成 " + std::to_wstring(p.GetAttack()*2) + L" 伤害";
        } else {
            s.logLine = L"法力不足！";
        }
    }
}

static void ProcessEnemyAction(Player& p, Enemy& e, CombatUIData& s) {
    if (s.enemyAction == 0) {
        p.TakeDamage(e.GetAttack());
        s.logLine += L" | " + e.GetName() + L" 攻击，造成 " + std::to_wstring(e.GetAttack()) + L" 伤害";
    } else if (s.enemyAction == 1) {
        s.logLine += L" | " + e.GetName() + L" 防御！";
        p.TakeDamage(e.GetAttack() / 2);
    } else {
        p.TakeDamage(e.GetAttack() * 2);
        s.logLine += L" | " + e.GetName() + L" 使出绝招！";
    }
}
