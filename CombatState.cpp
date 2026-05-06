#pragma execution_character_set("utf-8")
#include "CombatState.h"
#include "GameState.h"
#include "GameSession.h"
#include "ConfigManager.h"
#include "QuestSystem.h"         // 任务系统：战斗胜利通知
#include <cmath>
#include <algorithm>
#include <cstdlib>
#define NOMINMAX
#include <Windows.h>

// 前向声明（函数定义在文件底部）
static std::wstring ItemUtf8ToWide(const std::string& s);
static std::wstring Utf8ToWide(const std::string& s);

// ============================================================
//  构造函数
// ============================================================
CombatState::CombatState(const std::wstring& enemyName, int hp, int mp,
                         int atk, int def, EnemyType type,
                         int expReward, int goldReward)
{
    m_type = GameStateType::Combat;
    m_enemy = std::make_unique<Enemy>(enemyName, hp, mp, atk, def, type);
    m_player = &GameSession::Instance().GetPlayer();
    m_expReward = expReward;
    m_goldReward = goldReward;

    // 加载字体（宋体 → 微软雅黑）
    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");
}

// ============================================================
//  进入战斗
// ============================================================
void CombatState::Enter() {
    m_phase = CombatPhase::ShowMessage;
    m_afterMsgPhase = CombatPhase::PlayerTurn;
    m_round = 0;
    m_mainCursor = 0;
    m_skillCursor = 0;
    m_itemCursor = 0;
    m_result = CombatResult::Ongoing;
    m_battleLog.clear();
    m_particles.clear();

    BuildSkillList();
    BuildItemList();

    // 进场消息
    m_msgQueue.clear();
    m_msgQueue.push_back(L"【" + m_enemy->GetName() + L"】出现了！");
    ShowNextMessage();
}

void CombatState::Exit() {}

// ============================================================
//  构建技能列表（从玩家已学功法 + 等级解锁技能）
// ============================================================
void CombatState::BuildSkillList() {
    m_skills.clear();

    // 基础攻击技能（所有玩家始终拥有）
    {
        CombatSkill sk;
        sk.id = "heavy_strike";
        sk.name = L"重击";
        sk.desc = L"全力一击，伤害×1.8";
        sk.mpCost = 20;
        sk.dmgMult = 1.8f;
        m_skills.push_back(sk);
    }

    // 根据学会的功法追加额外战斗技能（含等级解锁检测）
    const auto& learned = m_player->GetLearned();

    for (const auto& lt : learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (!t) continue;

        if (lt.techniqueId == "changchun") {
            // 长春功 → 木灵治愈（始终可用）
            CombatSkill sk;
            sk.id = "changchun_heal";
            sk.name = L"木灵疗愈";
            sk.desc = L"长春功·愈合术，恢复生命";
            sk.mpCost = 15;
            sk.healAmount = 30 + lt.level * 8;
            sk.isHeal = true;
            m_skills.push_back(sk);

            // Lv.5 解锁：春回大地
            for (const auto& us : t->unlocksSkill) {
                if (us.level == 5 && lt.level >= 5) {
                    CombatSkill sk2;
                    sk2.id = "changchun_spring";
                    sk2.name = Utf8ToWide(us.skillName);
                    sk2.desc = Utf8ToWide(us.skillDesc);
                    sk2.mpCost = 40;
                    sk2.healAmount = 80 + lt.level * 15;
                    sk2.isHeal = true;
                    m_skills.push_back(sk2);
                }
                // Lv.10 解锁：木灵护体
                if (us.level == 10 && lt.level >= 10) {
                    CombatSkill sk3;
                    sk3.id = "changchun_guard";
                    sk3.name = Utf8ToWide(us.skillName);
                    sk3.desc = Utf8ToWide(us.skillDesc);
                    sk3.mpCost = 30;
                    sk3.dmgMult = 0.f;   // 辅助技能
                    m_skills.push_back(sk3);
                }
            }
        }
        if (lt.techniqueId == "blink_sword") {
            CombatSkill sk;
            sk.id = "blink_sword_atk";
            sk.name = L"眨眼剑法";
            sk.desc = L"迅疾一击，难以闪避";
            sk.mpCost = 12 + lt.level * 2;
            sk.dmgMult = 1.4f + lt.level * 0.1f;
            m_skills.push_back(sk);
        }
        if (lt.techniqueId == "qinyuan") {
            // 基础：青元剑气
            CombatSkill sk;
            sk.id = "qinyuan_atk";
            sk.name = L"青元剑气";
            sk.desc = L"凝聚剑气，大幅伤害";
            sk.mpCost = 25 + lt.level * 3;
            sk.dmgMult = 1.8f + lt.level * 0.15f;
            m_skills.push_back(sk);

            // Lv.4 万剑归宗
            for (const auto& us : t->unlocksSkill) {
                if (us.level == 4 && lt.level >= 4) {
                    CombatSkill sk2;
                    sk2.id = "qinyuan_multi";
                    sk2.name = Utf8ToWide(us.skillName);
                    sk2.desc = Utf8ToWide(us.skillDesc);
                    sk2.mpCost = 50;
                    sk2.dmgMult = 1.5f + lt.level * 0.1f;
                    m_skills.push_back(sk2);
                }
                if (us.level == 7 && lt.level >= 7) {
                    CombatSkill sk3;
                    sk3.id = "qinyuan_domain";
                    sk3.name = Utf8ToWide(us.skillName);
                    sk3.desc = Utf8ToWide(us.skillDesc);
                    sk3.mpCost = 60;
                    sk3.dmgMult = 2.2f + lt.level * 0.15f;
                    m_skills.push_back(sk3);
                }
            }
        }
        if (lt.techniqueId == "yujian") {
            CombatSkill sk;
            sk.id = "yujian_atk";
            sk.name = L"御剑术";
            sk.desc = L"驭使飞剑攻击";
            sk.mpCost = 20 + lt.level * 2;
            sk.dmgMult = 1.6f + lt.level * 0.12f;
            m_skills.push_back(sk);
        }
        if (lt.techniqueId == "luoyan") {
            CombatSkill sk;
            sk.id = "luoyan_dodge";
            sk.name = L"罗烟步";
            sk.desc = L"步法闪避，下回合防御+50%";
            sk.mpCost = 10;
            sk.dmgMult = 0.0f;   // 不造成伤害
            m_skills.push_back(sk);

            // Lv.3 烟云身法 / Lv.5 瞬步
            for (const auto& us : t->unlocksSkill) {
                if (us.level == 3 && lt.level >= 3) {
                    CombatSkill sk2;
                    sk2.id = "luoyan_cloud";
                    sk2.name = Utf8ToWide(us.skillName);
                    sk2.desc = Utf8ToWide(us.skillDesc);
                    sk2.mpCost = 25;
                    sk2.dmgMult = 0.f;
                    m_skills.push_back(sk2);
                }
                if (us.level == 5 && lt.level >= 5) {
                    CombatSkill sk3;
                    sk3.id = "luoyan_teleport";
                    sk3.name = Utf8ToWide(us.skillName);
                    sk3.desc = Utf8ToWide(us.skillDesc);
                    sk3.mpCost = 35;
                    sk3.dmgMult = 1.3f + lt.level * 0.2f;
                    m_skills.push_back(sk3);
                }
            }
        }
        if (lt.techniqueId == "dayan") {
            // 大衍决技能
            CombatSkill sk;
            sk.id = "dayan_scan";
            sk.name = L"神识探查";
            sk.desc = L"查看敌人详细信息和弱点";
            sk.mpCost = 20;
            sk.dmgMult = 0.f;
            m_skills.push_back(sk);

            for (const auto& us : t->unlocksSkill) {
                if (us.level == 3 && lt.level >= 3) {
                    CombatSkill sk2;
                    sk2.id = "dayan_illusion";
                    sk2.name = Utf8ToWide(us.skillName);
                    sk2.desc = Utf8ToWide(us.skillDesc);
                    sk2.mpCost = 45;
                    sk2.dmgMult = 1.0f;
                    m_skills.push_back(sk2);
                }
                if (us.level == 5 && lt.level >= 5) {
                    CombatSkill sk3;
                    sk3.id = "dayan_shock";
                    sk3.name = Utf8ToWide(us.skillName);
                    sk3.desc = Utf8ToWide(us.skillDesc);
                    sk3.mpCost = 80;
                    sk3.dmgMult = 2.5f + lt.level * 0.2f;
                    m_skills.push_back(sk3);
                }
            }
        }
    }
}

// ============================================================
//  构建道具列表（从背包）
// ============================================================
// UTF-8 std::string → std::wstring 正确转换
static std::wstring ItemUtf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// 通用 UTF-8 转宽字符串（用于功法技能名等）
static std::wstring Utf8ToWide(const std::string& s) {
    return ItemUtf8ToWide(s);
}

void CombatState::BuildItemList() {
    m_items.clear();
    auto& inv = GameSession::Instance().GetInventory();

    for (const auto& slot : inv.GetItems()) {
        const auto* data = ConfigManager::Instance().GetItem(slot.itemId);
        if (!data) continue;
        // 只显示战斗可用的消耗品
        if (data->type == ItemType::Medicine) {
            UsableItem ui;
            ui.itemId = slot.itemId;
            ui.name = ItemUtf8ToWide(data->name);
            ui.count = slot.count;
            ui.effectType = data->effectType;
            ui.effectValue = data->effectValue;
            if (data->effectType == EffectType::RestoreHp)
                ui.effect = L"回复" + std::to_wstring(data->effectValue) + L"HP";
            else if (data->effectType == EffectType::RestoreMp)
                ui.effect = L"回复" + std::to_wstring(data->effectValue) + L"MP";
            else
                ui.effect = L"特殊效果";
            m_items.push_back(ui);
        }
    }
}

// ============================================================
//  消息系统
// ============================================================
void CombatState::PushMessage(const std::wstring& msg) {
    m_msgQueue.push_back(msg);
}

void CombatState::ShowNextMessage() {
    if (!m_msgQueue.empty()) {
        m_currentMsg = m_msgQueue.front();
        m_msgQueue.erase(m_msgQueue.begin());
        AddLog(m_currentMsg);
        m_phase = CombatPhase::ShowMessage;
    } else {
        m_phase = m_afterMsgPhase;
    }
}

void CombatState::AddLog(const std::wstring& line) {
    m_battleLog.push_back(line);
    if ((int)m_battleLog.size() > 6)
        m_battleLog.erase(m_battleLog.begin());
}

// ============================================================
//  粒子特效
// ============================================================
void CombatState::SpawnHitParticles(float cx, float cy, sf::Color col) {
    for (int i = 0; i < 12; ++i) {
        Particle p;
        p.x = cx + (rand() % 20 - 10);
        p.y = cy + (rand() % 20 - 10);
        float angle = (rand() % 360) * 3.14159f / 180.f;
        float speed = 40.f + rand() % 80;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.maxLife = p.life = 0.4f + (rand() % 30) * 0.01f;
        p.color = col;
        m_particles.push_back(p);
    }
}

void CombatState::UpdateParticles(float dt) {
    for (auto& p : m_particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vy += 120.f * dt;  // 重力
        p.life -= dt;
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle& p){ return p.life <= 0; }), m_particles.end());
}

// ============================================================
//  Update
// ============================================================
void CombatState::Update(float dt) {
    m_animTimer += dt;

    if (m_hitShakeTimer > 0) m_hitShakeTimer -= dt;

    UpdateParticles(dt);

    // 刷新道具列表（背包可能改变）
    if (m_phase == CombatPhase::SelectItem)
        BuildItemList();

    if (m_phase == CombatPhase::EnemyTurn) {
        if (m_animTimer > 0.8f) {
            ExecuteEnemyAction();
            m_animTimer = 0.f;
        }
    }
}

// ============================================================
//  键盘输入
// ============================================================
void CombatState::OnKeyPressed(sf::Keyboard::Key key) {
    // 战斗结束
    if (m_phase == CombatPhase::CombatEnd) {
        if (key == sf::Keyboard::Return || key == sf::Keyboard::Space)
            GameStateManager::Instance().PopState();
        return;
    }

    // 消息展示：任意键继续
    if (m_phase == CombatPhase::ShowMessage) {
        if (key == sf::Keyboard::Return || key == sf::Keyboard::Space ||
            key == sf::Keyboard::Z) {
            ShowNextMessage();
        }
        return;
    }

    // 主菜单（四方向选择：上下左右对应2×2网格）
    if (m_phase == CombatPhase::PlayerTurn) {
        int curRow = m_mainCursor / 2;    // 当前行 0=上排 1=下排
        int curCol = m_mainCursor % 2;    // 当前列 0=左列 1=右列
        if (key == sf::Keyboard::Up || key == sf::Keyboard::W)
            m_mainCursor = 0 * 2 + curCol;      // 跳到上排
        else if (key == sf::Keyboard::Down || key == sf::Keyboard::S)
            m_mainCursor = 1 * 2 + curCol;      // 跳到下排
        else if (key == sf::Keyboard::Left || key == sf::Keyboard::A)
            m_mainCursor = curRow * 2 + 0;      // 跳到左列
        else if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
            m_mainCursor = curRow * 2 + 1;      // 跳到右列
        else if (key == sf::Keyboard::Return || key == sf::Keyboard::Z
                 || key == sf::Keyboard::Space) {
            switch (m_mainCursor) {
                case 0: // 攻击
                    ExecutePlayerAttack();
                    break;
                case 1: // 技能
                    if (m_skills.empty()) {
                        PushMessage(L"还没有学会任何技能！");
                        m_afterMsgPhase = CombatPhase::PlayerTurn;
                        ShowNextMessage();
                    } else {
                        m_phase = CombatPhase::SelectSkill;
                        m_skillCursor = 0;
                    }
                    break;
                case 2: // 道具
                    BuildItemList();
                    if (m_items.empty()) {
                        PushMessage(L"背包中没有可用道具！");
                        m_afterMsgPhase = CombatPhase::PlayerTurn;
                        ShowNextMessage();
                    } else {
                        m_phase = CombatPhase::SelectItem;
                        m_itemCursor = 0;
                    }
                    break;
                case 3: // 逃跑
                    // 逃跑成功率 50% + 速度差影响
                    {
                        int runChance = 50;
                        // 玩家速度高则更容易跑
                        runChance += (m_player->GetSpeed() - 5) * 3;
                        runChance = std::clamp(runChance, 10, 90);
                        if (rand() % 100 < runChance) {
                            PushMessage(L"成功逃脱！");
                            m_afterMsgPhase = CombatPhase::CombatEnd;
                            m_result = CombatResult::Defeat;  // 逃跑算失败（不给奖励）
                            ShowNextMessage();
                        } else {
                            PushMessage(L"逃跑失败！");
                            m_afterMsgPhase = CombatPhase::EnemyTurn;
                            ShowNextMessage();
                            m_animTimer = 0.f;
                        }
                    }
                    break;
            }
        }
        else if (key == sf::Keyboard::Escape || key == sf::Keyboard::X) {
            // X/Esc 取消（在主菜单无效，只做提示）
        }
        return;
    }

    // 技能菜单
    if (m_phase == CombatPhase::SelectSkill) {
        if (key == sf::Keyboard::Up || key == sf::Keyboard::W)
            m_skillCursor = (m_skillCursor + (int)m_skills.size() - 1) % (int)m_skills.size();
        else if (key == sf::Keyboard::Down || key == sf::Keyboard::S)
            m_skillCursor = (m_skillCursor + 1) % (int)m_skills.size();
        else if (key == sf::Keyboard::Left || key == sf::Keyboard::A) {
            m_skillCursor = (m_skillCursor + (int)m_skills.size() - 3) % (int)m_skills.size();
            if (m_skillCursor < 0) m_skillCursor = 0;
        }
        else if (key == sf::Keyboard::Right || key == sf::Keyboard::D) {
            m_skillCursor = (m_skillCursor + 3) % (int)m_skills.size();
        }
        else if (key == sf::Keyboard::Return || key == sf::Keyboard::Z
                 || key == sf::Keyboard::Space) {
            ExecutePlayerSkill(m_skillCursor);
        }
        else if (key == sf::Keyboard::Escape || key == sf::Keyboard::X) {
            m_phase = CombatPhase::PlayerTurn;
        }
        return;
    }

    // 道具菜单
    if (m_phase == CombatPhase::SelectItem) {
        if (key == sf::Keyboard::Up || key == sf::Keyboard::W)
            m_itemCursor = (m_itemCursor + (int)m_items.size() - 1) % (int)m_items.size();
        else if (key == sf::Keyboard::Down || key == sf::Keyboard::S)
            m_itemCursor = (m_itemCursor + 1) % (int)m_items.size();
        else if (key == sf::Keyboard::Left || key == sf::Keyboard::A) {
            m_itemCursor = (m_itemCursor + (int)m_items.size() - 3) % (int)m_items.size();
            if (m_itemCursor < 0) m_itemCursor = 0;
        }
        else if (key == sf::Keyboard::Right || key == sf::Keyboard::D) {
            m_itemCursor = (m_itemCursor + 3) % (int)m_items.size();
        }
        else if (key == sf::Keyboard::Return || key == sf::Keyboard::Z
                 || key == sf::Keyboard::Space) {
            ExecutePlayerItem(m_itemCursor);
        }
        else if (key == sf::Keyboard::Escape || key == sf::Keyboard::X) {
            m_phase = CombatPhase::PlayerTurn;
        }
        return;
    }
}

// ============================================================
//  玩家行动
// ============================================================
void CombatState::ExecutePlayerAttack() {
    if (!m_player || !m_enemy) return;
    int dmg = std::max(1, m_player->GetAttack() - m_enemy->GetDefense() / 2);
    m_enemy->TakeDamage(dmg);
    m_enemyHit = true;
    m_hitShakeTimer = 0.3f;
    SpawnHitParticles(580.f, 200.f, sf::Color(255, 200, 50));

    m_round++;
    m_msgQueue.clear();
    PushMessage(m_player->GetName() + L" 发动攻击！");
    PushMessage(L"对【" + m_enemy->GetName() + L"】造成 " + std::to_wstring(dmg) + L" 点伤害！");

    CheckCombatEnd();
    if (m_result != CombatResult::Ongoing) {
        GiveRewards();
        m_afterMsgPhase = CombatPhase::CombatEnd;
    } else {
        m_afterMsgPhase = CombatPhase::EnemyTurn;
        m_animTimer = 0.f;
    }
    ShowNextMessage();
}

void CombatState::ExecutePlayerSkill(int idx) {
    if (idx < 0 || idx >= (int)m_skills.size()) return;
    const auto& sk = m_skills[idx];

    // MP检查
    if (m_player->GetCurrentMp() < sk.mpCost) {
        m_msgQueue.clear();
        PushMessage(L"灵力不足！无法施展【" + sk.name + L"】");
        m_afterMsgPhase = CombatPhase::PlayerTurn;
        ShowNextMessage();
        return;
    }

    m_player->SetCurrentMp(m_player->GetCurrentMp() - sk.mpCost);
    m_round++;
    m_msgQueue.clear();
    PushMessage(m_player->GetName() + L" 施展【" + sk.name + L"】！");

    if (sk.isHeal) {
        // 治愈技能
        int oldHp = m_player->GetCurrentHp();
        m_player->SetCurrentHp(oldHp + sk.healAmount);
        int actual = m_player->GetCurrentHp() - oldHp;
        SpawnHitParticles(200.f, 300.f, sf::Color(100, 255, 150));
        PushMessage(L"恢复了 " + std::to_wstring(actual) + L" 点生命！");
        m_afterMsgPhase = CombatPhase::EnemyTurn;
        m_animTimer = 0.f;
    } else if (sk.dmgMult > 0.f) {
        // 攻击技能
        int dmg = std::max(1, (int)(m_player->GetAttack() * sk.dmgMult) - m_enemy->GetDefense() / 2);
        m_enemy->TakeDamage(dmg);
        m_enemyHit = true;
        m_hitShakeTimer = 0.4f;
        SpawnHitParticles(580.f, 200.f, sf::Color(180, 100, 255));
        PushMessage(L"对【" + m_enemy->GetName() + L"】造成 " + std::to_wstring(dmg) + L" 点伤害！");

        CheckCombatEnd();
        if (m_result != CombatResult::Ongoing) {
            GiveRewards();
            m_afterMsgPhase = CombatPhase::CombatEnd;
        } else {
            m_afterMsgPhase = CombatPhase::EnemyTurn;
            m_animTimer = 0.f;
        }
    } else {
        // 特殊技能（如逃跑/防御等）
        PushMessage(L"使用了特殊技能！");
        m_afterMsgPhase = CombatPhase::EnemyTurn;
        m_animTimer = 0.f;
    }

    m_phase = CombatPhase::PlayerTurn; // 先切回，ShowNextMessage会覆盖
    ShowNextMessage();
}

void CombatState::ExecutePlayerItem(int idx) {
    if (idx < 0 || idx >= (int)m_items.size()) return;
    const auto& item = m_items[idx];

    auto& inv = GameSession::Instance().GetInventory();
    m_msgQueue.clear();
    PushMessage(m_player->GetName() + L" 使用了【" + item.name + L"】！");

    if (item.effectType == EffectType::RestoreHp) {
        int oldHp = m_player->GetCurrentHp();
        m_player->SetCurrentHp(oldHp + item.effectValue);
        int actual = m_player->GetCurrentHp() - oldHp;
        SpawnHitParticles(200.f, 300.f, sf::Color(100, 255, 100));
        PushMessage(L"恢复了 " + std::to_wstring(actual) + L" 点生命！");
    } else if (item.effectType == EffectType::RestoreMp) {
        int oldMp = m_player->GetCurrentMp();
        m_player->SetCurrentMp(oldMp + item.effectValue);
        int actual = m_player->GetCurrentMp() - oldMp;
        SpawnHitParticles(200.f, 340.f, sf::Color(100, 150, 255));
        PushMessage(L"恢复了 " + std::to_wstring(actual) + L" 点灵力！");
    } else {
        PushMessage(L"发生了奇妙的变化...");
    }

    inv.RemoveItem(item.itemId, 1);
    BuildItemList();
    m_round++;

    m_afterMsgPhase = CombatPhase::EnemyTurn;
    m_animTimer = 0.f;
    ShowNextMessage();
}

// ============================================================
//  敌人行动
// ============================================================
void CombatState::ExecuteEnemyAction() {
    if (!m_player || !m_enemy || !m_enemy->IsAlive()) {
        m_phase = CombatPhase::PlayerTurn;
        return;
    }

    int action = m_enemy->ChooseAction();
    m_msgQueue.clear();

    // Boss特殊AI：墨大夫等Boss有独特技能
    if (m_isBoss) {
        int bossHp = m_enemy->GetHp();
        int bossMaxHp = m_enemy->GetMaxHp();
        float hpRatio = (float)bossHp / (float)std::max(1, bossMaxHp);
        std::wstring bossName = m_enemy->GetName();

        // Boss阶段技能
        if (hpRatio > 0.6f) {
            // 阶段1：正常攻击 + 毒雾
            int roll = rand() % 10;
            if (roll < 5) {
                // 普通攻击
                int dmg = std::max(1, m_enemy->GetAttack() - m_player->GetDefense() / 2);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.4f;
                SpawnHitParticles(200.f, 300.f, sf::Color(255, 80, 80));
                PushMessage(L"【" + bossName + L"】发动攻击！");
                PushMessage(L"造成 " + std::to_wstring(dmg) + L" 点伤害！");
            } else if (roll < 8) {
                // 毒雾术 — 中等伤害+持续debuff描述
                int dmg = std::max(1, (int)(m_enemy->GetAttack() * 1.3f) - m_player->GetDefense() / 3);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.5f;
                SpawnHitParticles(200.f, 300.f, sf::Color(100, 200, 50));
                PushMessage(L"【" + bossName + L"】施展【毒雾术】！");
                PushMessage(L"绿色毒雾弥漫，造成 " + std::to_wstring(dmg) + L" 点伤害！");
            } else {
                // 夺舍秘术·预备 — 低伤害但蓄力
                int dmg = std::max(1, m_enemy->GetAttack() / 3);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.3f;
                SpawnHitParticles(200.f, 300.f, sf::Color(180, 50, 220));
                PushMessage(L"【" + bossName + L"】低声念咒...\n一股阴冷的力量向你袭来！");
                PushMessage(L"造成 " + std::to_wstring(dmg) + L" 点伤害。墨大夫似乎在积蓄力量...");
            }
        } else if (hpRatio > 0.3f) {
            // 阶段2：更激进，使用夺舍秘术
            int roll = rand() % 10;
            if (roll < 4) {
                // 强化攻击
                int dmg = std::max(1, (int)(m_enemy->GetAttack() * 1.5f) - m_player->GetDefense() / 2);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.5f;
                SpawnHitParticles(200.f, 300.f, sf::Color(255, 100, 50));
                PushMessage(L"【" + bossName + L"】狂暴攻击！");
                PushMessage(L"造成 " + std::to_wstring(dmg) + L" 点伤害！");
            } else if (roll < 7) {
                // 夺舍秘术 — 大伤害
                int dmg = std::max(1, (int)(m_enemy->GetAttack() * 2.0f) - m_player->GetDefense() / 3);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.6f;
                SpawnHitParticles(200.f, 300.f, sf::Color(200, 50, 255));
                PushMessage(L"【" + bossName + L"】施展【夺舍秘术】！");
                PushMessage(L"阴寒之力入侵体内，造成 " + std::to_wstring(dmg) + L" 点伤害！");
            } else {
                // 自愈 — 恢复少量HP
                int heal = bossMaxHp / 8;
                // Boss无法真正自愈，只是描述效果（保持战斗难度）
                SpawnHitParticles(580.f, 200.f, sf::Color(100, 255, 100));
                PushMessage(L"【" + bossName + L"】服用丹药，气息稍有恢复！");
                PushMessage(L"（但你的攻势令他无法安心疗伤）");
            }
        } else {
            // 阶段3：濒死狂暴，高伤害技能
            int roll = rand() % 10;
            if (roll < 5) {
                // 拼命一击
                int dmg = std::max(1, (int)(m_enemy->GetAttack() * 2.2f) - m_player->GetDefense() / 3);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.7f;
                SpawnHitParticles(200.f, 300.f, sf::Color(255, 50, 50));
                PushMessage(L"【" + bossName + L"】濒死反扑！");
                PushMessage(L"造成 " + std::to_wstring(dmg) + L" 点伤害！");
            } else {
                // 夺舍秘术·全力
                int dmg = std::max(1, (int)(m_enemy->GetAttack() * 2.5f) - m_player->GetDefense() / 4);
                m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
                m_playerHit = true;
                m_hitShakeTimer = 0.8f;
                SpawnHitParticles(200.f, 300.f, sf::Color(220, 30, 255));
                PushMessage(L"【" + bossName + L"】孤注一掷：【夺舍秘术·全力】！");
                PushMessage(L"巨大的灵压席卷而来，造成 " + std::to_wstring(dmg) + L" 点伤害！");
            }
        }
    } else {
        // 普通敌人AI
        static const wchar_t* actionNames[3] = {
            L"发动攻击", L"防御反击", L"使出绝招"
        };
        PushMessage(L"【" + m_enemy->GetName() + L"】" +
                    std::wstring(actionNames[std::min(action, 2)]) + L"！");

        int dmg = 0;
        switch (action) {
            case 0: dmg = std::max(1, m_enemy->GetAttack() - m_player->GetDefense() / 2); break;
            case 1: dmg = std::max(1, m_enemy->GetAttack() / 4); break;
            case 2: dmg = std::max(1, (int)(m_enemy->GetAttack() * 1.7f) - m_player->GetDefense() / 2); break;
            default: dmg = 1; break;
        }

        m_player->SetCurrentHp(m_player->GetCurrentHp() - dmg);
        m_playerHit = true;
        m_hitShakeTimer = 0.4f;
        SpawnHitParticles(200.f, 300.f, sf::Color(255, 80, 80));
        PushMessage(L"对【" + m_player->GetName() + L"】造成 " + std::to_wstring(dmg) + L" 点伤害！");
    }

    CheckCombatEnd();
    if (m_result != CombatResult::Ongoing) {
        if (m_result == CombatResult::Defeat)
            PushMessage(L"你被击败了...");
        m_afterMsgPhase = CombatPhase::CombatEnd;
    } else {
        m_afterMsgPhase = CombatPhase::PlayerTurn;
    }
    ShowNextMessage();
}

// ============================================================
//  战斗结束检查
// ============================================================
void CombatState::CheckCombatEnd() {
    if (!m_enemy->IsAlive()) {
        m_result = CombatResult::Victory;
    } else if (!m_player->IsAlive()) {
        m_result = CombatResult::Defeat;
    }
}

void CombatState::GiveRewards() {
    m_player->AddGold(m_goldReward);
    m_player->GetCultiMutable().cultivationExp += m_expReward;
    PushMessage(L"★ 获得灵石 " + std::to_wstring(m_goldReward) +
                L"，修炼经验 +" + std::to_wstring(m_expReward) + L" ★");

    // 通知任务系统：击败敌人+1
    if (m_result == CombatResult::Victory) {
        QuestSystem::Instance().UpdateProgress(QuestTargetType::DefeatEnemy, "any", 1);
        // Boss战特殊通知
        if (m_isBoss) {
            QuestSystem::Instance().UpdateProgress(QuestTargetType::DefeatEnemy, "mo_dafu_boss", 1);
            // 检测Boss击败事件（触发战败对话+旁白）
            QuestSystem::Instance().CheckBossDefeat("mo_dafu_boss");
        }
    }
}

// ============================================================
//  辅助绘图函数
// ============================================================
void CombatState::DrawBar(sf::RenderWindow& w, float x, float y, float bw, float bh,
                          float ratio, sf::Color fg, sf::Color bg) {
    sf::RectangleShape back(sf::Vector2f(bw, bh));
    back.setFillColor(bg);
    back.setPosition(x, y);
    w.draw(back);

    float fill = bw * std::max(0.f, std::min(1.f, ratio));
    if (fill > 0) {
        sf::RectangleShape bar(sf::Vector2f(fill, bh));
        bar.setFillColor(fg);
        bar.setPosition(x, y);
        w.draw(bar);
    }
}

void CombatState::DrawBox(sf::RenderWindow& w, float x, float y, float bw, float bh,
                          sf::Color fill, sf::Color border, float thickness) {
    sf::RectangleShape box(sf::Vector2f(bw, bh));
    box.setFillColor(fill);
    box.setOutlineThickness(thickness);
    box.setOutlineColor(border);
    box.setPosition(x, y);
    w.draw(box);
}

// ============================================================
//  渲染主函数
// ============================================================
void CombatState::Render(sf::RenderWindow& window) {
    RenderBackground(window);
    RenderEnemySprite(window);
    RenderPlayerSprite(window);
    RenderStatusBars(window);
    RenderParticles(window);

    if (m_phase == CombatPhase::CombatEnd) {
        RenderEndScreen(window);
        return;
    }
    if (m_phase == CombatPhase::ShowMessage) {
        RenderMessageBox(window);
        return;
    }
    if (m_phase == CombatPhase::SelectSkill) {
        RenderMessageBox(window);
        RenderSkillMenu(window);
        return;
    }
    if (m_phase == CombatPhase::SelectItem) {
        RenderMessageBox(window);
        RenderItemMenu(window);
        return;
    }

    // 主菜单 + 消息框
    RenderMessageBox(window);
    RenderMainMenu(window);
}

// ============================================================
//  背景
// ============================================================
void CombatState::RenderBackground(sf::RenderWindow& w) {
    // 天空渐变（上深蓝 → 中紫 → 下暗棕地面）
    // 天空区域
    sf::RectangleShape sky(sf::Vector2f(800.f, 320.f));
    sky.setFillColor(sf::Color(18, 10, 35));
    sky.setPosition(0, 0);
    w.draw(sky);

    // 地面区域
    sf::RectangleShape ground(sf::Vector2f(800.f, 130.f));
    ground.setFillColor(sf::Color(35, 25, 15));
    ground.setPosition(0, 310.f);
    w.draw(ground);

    // 地面线
    sf::RectangleShape groundLine(sf::Vector2f(800.f, 3.f));
    groundLine.setFillColor(sf::Color(80, 55, 30));
    groundLine.setPosition(0, 310.f);
    w.draw(groundLine);

    // 月亮装饰
    sf::CircleShape moon(30.f);
    moon.setFillColor(sf::Color(255, 250, 220));
    moon.setPosition(680.f, 20.f);
    w.draw(moon);

    // 几颗星星
    for (int i = 0; i < 8; ++i) {
        sf::CircleShape star(1.5f + (i % 2));
        star.setFillColor(sf::Color(220, 220, 255, 180));
        star.setPosition(50.f + i * 90.f + (i % 3) * 20.f, 30.f + (i % 4) * 15.f);
        w.draw(star);
    }

    // 回合数（右上角）
    sf::Text roundT;
    roundT.setFont(m_font);
    roundT.setCharacterSize(14);
    roundT.setFillColor(sf::Color(180, 180, 180));
    roundT.setString(L"第 " + std::to_wstring(m_round + 1) + L" 回合");
    roundT.setPosition(680.f, 5.f);
    w.draw(roundT);
}

// ============================================================
//  敌人立绘（右侧，较大）
// ============================================================
void CombatState::RenderEnemySprite(sf::RenderWindow& w) {
    float shake = 0.f;
    if (m_hitShakeTimer > 0 && m_enemyHit)
        shake = std::sin(m_hitShakeTimer * 40.f) * 6.f;

    float ex = 480.f + shake;
    float ey = 100.f;

    // 敌人身体（像素风，用小方块构成人形）
    sf::Color bodyC(140,60,60), bodyDk(100,40,40), bodyLt(180,80,80);
    sf::Color skin(255,200,160), hair(30,20,10), armor(90,80,70), eyeC(255,50,50);
    int t = 0;
    if (m_enemy) { auto& n = m_enemy->GetName(); if (!n.empty()) t = ((int)n[0])%4; }
    switch (t) {
        case 0: bodyC={160,55,55};bodyDk={110,35,35};bodyLt={200,70,70};armor={100,45,45};eyeC={255,30,30};break;
        case 1: bodyC={55,90,160};bodyDk={35,60,110};bodyLt={80,120,200};armor={60,70,90};eyeC={200,200,100};break;
        case 2: bodyC={80,150,55};bodyDk={50,100,35};bodyLt={120,200,80};armor={60,90,50};eyeC={255,200,50};break;
        case 3: bodyC={110,50,140};bodyDk={75,30,95};bodyLt={150,70,180};armor={80,40,100};eyeC={255,100,200};break;
    }
    auto P = [&](float x,float y,float ww,float h,sf::Color c){
        sf::RectangleShape r(sf::Vector2f(ww,h));r.setFillColor(c);r.setPosition(ex+x,ey+y);w.draw(r);
    };

    // 像素风敌人（约64x96像素人形）
    P(12,78,8,16,bodyDk);P(44,78,8,16,bodyDk);          // 腿
    P(10,90,12,5,sf::Color(40,35,30));P(42,90,12,5,sf::Color(40,35,30)); // 鞋
    P(10,44,44,38,bodyC);P(10,44,8,38,bodyDk);P(46,44,8,38,bodyDk); // 躯干
    P(14,62,36,5,armor);P(16,44,32,4,bodyLt);           // 腰带+衣领
    P(4,50,8,28,bodyC);P(52,50,8,28,bodyC);             // 双臂
    P(4,64,8,5,skin);P(52,64,8,5,skin);                 // 手
    P(14,16,36,30,skin);                                 // 头脸
    P(14,10,36,12,hair);P(10,16,8,12,hair);P(46,16,8,12,hair); // 头发
    P(14,8,10,6,hair);P(40,8,10,6,hair);                // 发顶
    P(22,28,8,6,eyeC);P(34,28,8,6,eyeC);                // 眼睛
    P(24,30,4,4,sf::Color(255,255,255));P(36,30,4,4,sf::Color(255,255,255)); // 瞳白
    if(m_hitShakeTimer>0&&m_enemyHit) P(26,40,12,4,sf::Color(255,50,50));
    else P(26,40,12,3,sf::Color(100,80,60));             // 嘴

    // 受击闪白

    // 受击闪白
    if (m_hitShakeTimer > 0 && m_enemyHit) {
        sf::RectangleShape flash(sf::Vector2f(110.f, 130.f));
        float alpha = (m_hitShakeTimer / 0.4f) * 200.f;
        flash.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)std::min(200.f, alpha)));
        flash.setPosition(ex, ey);
        w.draw(flash);
        if (m_hitShakeTimer <= 0) m_enemyHit = false;
    }

    // 敌人名称（头上方）
    if (m_enemy) {
        sf::Text nameT;
        nameT.setFont(m_font);
        nameT.setCharacterSize(14);
        nameT.setFillColor(sf::Color(255, 200, 200));
        nameT.setString(m_enemy->GetName());
        auto nb = nameT.getLocalBounds();
        nameT.setPosition(ex + 55 - nb.width/2, ey - 22);
        w.draw(nameT);
    }
}

// ============================================================
//  玩家立绘（左侧，使用像素风角色）
// ============================================================
void CombatState::RenderPlayerSprite(sf::RenderWindow& w) {
    float shake = 0.f;
    if (m_hitShakeTimer > 0 && m_playerHit)
        shake = std::sin(m_hitShakeTimer * 35.f) * 5.f;

    float px = 120.f + shake;
    float py = 160.f;

    // 韩立像素风（参考PlayerSprite的配色）
    auto P = [&](float x,float y,float ww,float h,sf::Color c){
        sf::RectangleShape r(sf::Vector2f(ww,h));r.setFillColor(c);r.setPosition(px+x,py+y);w.draw(r);
    };
    const sf::Color robe(38,128,95), robeDk(26,90,65), robeLt(52,160,120);
    const sf::Color skin(255,215,175), skinD(220,185,150);
    const sf::Color hair(28,22,18), belt(160,115,50), shoes(50,45,38);

    // 阴影
    P(10,98,32,5,sf::Color(0,0,0,40));

    // 腿+鞋
    P(12,84,8,16,robeDk);P(32,84,8,16,robeDk);
    P(10,96,12,5,shoes);P(30,96,12,5,shoes);

    // 身体
    P(10,44,32,44,robe);
    P(10,44,6,44,robeDk);P(36,44,6,44,robeDk);
    P(14,68,24,3,belt);
    P(16,44,20,4,robeLt); // 衣领

    // 手臂
    P(4,48,8,30,robe);P(40,48,8,30,robe);
    P(4,64,8,5,skin);P(40,64,8,5,skin);

    // 头
    P(16,18,20,28,skin);
    P(16,12,20,10,sf::Color(20,18,15)); // 头发顶
    P(12,16,8,12,sf::Color(20,18,15)); // 左鬓
    P(32,16,8,12,sf::Color(20,18,15)); // 右鬓

    // 眼睛（坚定眼神）
    P(18,27,6,5,sf::Color(255,255,255));
    P(28,27,6,5,sf::Color(255,255,255));
    P(20,28,3,4,sf::Color(25,20,18));
    P(30,28,3,4,sf::Color(25,20,18));

    // 嘴巴
    P(22,37,8,2,sf::Color(180,140,110));

    // 受击闪红
    if (m_hitShakeTimer > 0 && m_playerHit)
    if (m_hitShakeTimer > 0 && m_playerHit) {
        sf::RectangleShape flash(sf::Vector2f(60.f, 80.f));
        float alpha = (m_hitShakeTimer / 0.4f) * 180.f;
        flash.setFillColor(sf::Color(255, 80, 80, (sf::Uint8)std::min(180.f, alpha)));
        flash.setPosition(px, py);
        w.draw(flash);
        if (m_hitShakeTimer <= 0) m_playerHit = false;
    }
}

// ============================================================
//  状态栏（宝可梦HUD风格）
// ============================================================
void CombatState::RenderStatusBars(sf::RenderWindow& w) {
    // ===== 敌人状态栏（右上角，半透明框）=====
    DrawBox(w, 430.f, 20.f, 340.f, 85.f,
            sf::Color(15,15,15,200), sf::Color(100,80,80,200), 2.f);

    if (m_enemy) {
        sf::Text nameT;
        nameT.setFont(m_font);
        nameT.setCharacterSize(18);
        nameT.setFillColor(sf::Color(255, 180, 180));
        nameT.setString(m_enemy->GetName());
        nameT.setPosition(442.f, 25.f);
        w.draw(nameT);

        // 敌人HP条
        sf::Text hpLabel;
        hpLabel.setFont(m_font);
        hpLabel.setCharacterSize(13);
        hpLabel.setFillColor(sf::Color(180, 180, 180));
        hpLabel.setString(L"HP");
        hpLabel.setPosition(442.f, 52.f);
        w.draw(hpLabel);

        float ehpRatio = (float)m_enemy->GetHp() / (float)std::max(1, m_enemy->GetMaxHp());
        sf::Color hpCol = ehpRatio > 0.5f ? sf::Color(220,60,60) :
                          ehpRatio > 0.25f ? sf::Color(220,160,60) : sf::Color(255,60,60);
        DrawBar(w, 462.f, 55.f, 290.f, 14.f, ehpRatio, hpCol, sf::Color(50,20,20));

        sf::Text hpNum;
        hpNum.setFont(m_font);
        hpNum.setCharacterSize(12);
        hpNum.setFillColor(sf::Color(200,200,200));
        hpNum.setString(std::to_wstring(m_enemy->GetHp()) + L"/" +
                        std::to_wstring(m_enemy->GetMaxHp()));
        hpNum.setPosition(462.f, 72.f);
        w.draw(hpNum);
    }

    // ===== 玩家状态栏（左下角）=====
    DrawBox(w, 10.f, 310.f, 380.f, 100.f,
            sf::Color(15,15,15,210), sf::Color(80,80,120,200), 2.f);

    if (m_player) {
        // 玩家名 + 境界
        sf::Text nameT;
        nameT.setFont(m_font);
        nameT.setCharacterSize(18);
        nameT.setFillColor(sf::Color(150, 210, 255));
        nameT.setString(m_player->GetName() + L"  " + m_player->GetRealmName());
        nameT.setPosition(20.f, 315.f);
        w.draw(nameT);

        // HP条
        sf::Text hpL;
        hpL.setFont(m_font);
        hpL.setCharacterSize(13);
        hpL.setFillColor(sf::Color(180,180,180));
        hpL.setString(L"HP");
        hpL.setPosition(20.f, 340.f);
        w.draw(hpL);

        float phpRatio = (float)m_player->GetCurrentHp() / (float)std::max(1, m_player->GetMaxHp());
        sf::Color hpCol = phpRatio > 0.5f ? sf::Color(60,210,80) :
                          phpRatio > 0.25f ? sf::Color(210,180,60) : sf::Color(255,60,60);
        DrawBar(w, 44.f, 343.f, 320.f, 14.f, phpRatio, hpCol, sf::Color(20,40,20));

        sf::Text hpNum;
        hpNum.setFont(m_font);
        hpNum.setCharacterSize(12);
        hpNum.setFillColor(sf::Color(200,220,200));
        hpNum.setString(std::to_wstring(m_player->GetCurrentHp()) + L"/" +
                        std::to_wstring(m_player->GetMaxHp()));
        hpNum.setPosition(44.f, 360.f);
        w.draw(hpNum);

        // MP条
        sf::Text mpL;
        mpL.setFont(m_font);
        mpL.setCharacterSize(13);
        mpL.setFillColor(sf::Color(180,180,180));
        mpL.setString(L"MP");
        mpL.setPosition(200.f, 340.f);
        w.draw(mpL);

        float mpRatio = (float)m_player->GetCurrentMp() / (float)std::max(1, m_player->GetMaxMp());
        DrawBar(w, 224.f, 343.f, 150.f, 14.f, mpRatio,
                sf::Color(80, 130, 255), sf::Color(20,20,50));

        sf::Text mpNum;
        mpNum.setFont(m_font);
        mpNum.setCharacterSize(12);
        mpNum.setFillColor(sf::Color(180,200,255));
        mpNum.setString(std::to_wstring(m_player->GetCurrentMp()) + L"/" +
                        std::to_wstring(m_player->GetMaxMp()));
        mpNum.setPosition(224.f, 360.f);
        w.draw(mpNum);
    }
}

// ============================================================
//  消息框（下半屏，仿宝可梦文字框）
// ============================================================
void CombatState::RenderMessageBox(sf::RenderWindow& w) {
    DrawBox(w, 10.f, 418.f, 780.f, 80.f,
            sf::Color(15,15,25,230), sf::Color(100,100,160,220), 2.5f);

    sf::Text msg;
    msg.setFont(m_font);
    msg.setCharacterSize(20);
    msg.setFillColor(sf::Color(240, 240, 240));
    msg.setString(m_currentMsg);
    msg.setPosition(28.f, 432.f);
    w.draw(msg);

    // 如果还有消息或在展示阶段，显示"▼ 继续"提示
    if (m_phase == CombatPhase::ShowMessage) {
        float blink = std::fmod(m_animTimer, 0.8f);
        if (blink < 0.5f) {
            sf::Text cont;
            cont.setFont(m_font);
            cont.setCharacterSize(14);
            cont.setFillColor(sf::Color(180, 180, 220));
            cont.setString(L"▼ 按空格/Z继续");
            cont.setPosition(660.f, 468.f);
            w.draw(cont);
        }
    }
}

// ============================================================
//  主菜单（右下角4格，宝可梦风格）
// ============================================================
void CombatState::RenderMainMenu(sf::RenderWindow& w) {
    // 右下角菜单框
    DrawBox(w, 400.f, 418.f, 390.f, 170.f,
            sf::Color(20,20,40,240), sf::Color(120,120,200,220), 2.5f);

    static const wchar_t* labels[4] = { L"攻  击", L"技  能", L"道  具", L"逃  跑" };
    // 2×2 布局
    for (int i = 0; i < 4; ++i) {
        int col = i % 2;
        int row = i / 2;
        float bx = 420.f + col * 185.f;
        float by = 430.f + row * 70.f;

        bool sel = (i == m_mainCursor);
        sf::Color bgCol = sel ? sf::Color(60, 60, 120) : sf::Color(30, 30, 60);
        sf::Color fgCol = sel ? sf::Color(255, 220, 80) : sf::Color(200, 200, 220);

        DrawBox(w, bx, by, 160.f, 50.f, bgCol, sf::Color(100,100,180), 1.5f);

        sf::Text t;
        t.setFont(m_font);
        t.setCharacterSize(20);
        t.setFillColor(fgCol);
        t.setString(labels[i]);
        t.setPosition(bx + 30.f, by + 12.f);
        w.draw(t);

        if (sel) {
            // 四角边框高亮选中项
            sf::RectangleShape selBorder(sf::Vector2f(160.f, 50.f));
            selBorder.setFillColor(sf::Color::Transparent);
            selBorder.setOutlineThickness(2.f);
            selBorder.setOutlineColor(sf::Color(255, 220, 80));
            selBorder.setPosition(bx, by);
            w.draw(selBorder);

            // 闪烁光标
            sf::Text cursor;
            cursor.setFont(m_font);
            cursor.setCharacterSize(18);
            cursor.setFillColor(sf::Color(255, 220, 80));
            cursor.setString(L"▶");
            cursor.setPosition(bx + 8.f, by + 14.f);
            w.draw(cursor);
        }
    }

    //  方向键/ WASD选择 空格/Z确认  X取消
    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 140, 160));
    hint.setString(L"↑↓←→ / WASD方向选择  空格/Z确认  X取消");
    hint.setPosition(420.f, 555.f);
    w.draw(hint);
}

// ============================================================
//  技能菜单
// ============================================================
void CombatState::RenderSkillMenu(sf::RenderWindow& w) {
    DrawBox(w, 390.f, 418.f, 400.f, 170.f,
            sf::Color(20,10,40,245), sf::Color(140,100,200,220), 2.5f);

    sf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(16);
    title.setFillColor(sf::Color(200, 160, 255));
    title.setString(L"─ 技 能 ─");
    title.setPosition(560.f, 422.f);
    w.draw(title);

    int maxShow = 4;
    int start = std::max(0, m_skillCursor - maxShow + 1);

    for (int i = 0; i < maxShow && (start + i) < (int)m_skills.size(); ++i) {
        int idx = start + i;
        const auto& sk = m_skills[idx];
        bool sel = (idx == m_skillCursor);

        float by = 445.f + i * 34.f;
        sf::Color fgCol = sel ? sf::Color(255, 220, 80) : sf::Color(210, 210, 230);

        if (sel) {
            sf::RectangleShape highlight(sf::Vector2f(390.f, 30.f));
            highlight.setFillColor(sf::Color(60, 40, 90));
            highlight.setPosition(400.f, by - 2.f);
            w.draw(highlight);

            sf::Text cur;
            cur.setFont(m_font);
            cur.setCharacterSize(16);
            cur.setFillColor(sf::Color(255, 220, 80));
            cur.setString(L"▶");
            cur.setPosition(402.f, by);
            w.draw(cur);
        }

        sf::Text nameT;
        nameT.setFont(m_font);
        nameT.setCharacterSize(17);
        nameT.setFillColor(fgCol);
        nameT.setString(sk.name);
        nameT.setPosition(425.f, by);
        w.draw(nameT);

        // MP消耗
        sf::Text mpT;
        mpT.setFont(m_font);
        mpT.setCharacterSize(13);
        mpT.setFillColor(sf::Color(100, 140, 255));
        mpT.setString(L"MP:" + std::to_wstring(sk.mpCost));
        mpT.setPosition(590.f, by + 3.f);
        w.draw(mpT);

        // 效果描述
        sf::Text desc;
        desc.setFont(m_font);
        desc.setCharacterSize(12);
        desc.setFillColor(sf::Color(160, 160, 180));
        desc.setString(sk.desc);
        desc.setPosition(660.f, by + 4.f);
        w.draw(desc);
    }

    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 140, 160));
    hint.setString(L"↑↓←→选择  空格确认  X返回");
    hint.setPosition(410.f, 560.f);
    w.draw(hint);
}

// ============================================================
//  道具菜单
// ============================================================
void CombatState::RenderItemMenu(sf::RenderWindow& w) {
    DrawBox(w, 390.f, 418.f, 400.f, 170.f,
            sf::Color(10,25,15,245), sf::Color(80,160,100,220), 2.5f);

    sf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(16);
    title.setFillColor(sf::Color(150, 220, 160));
    title.setString(L"─ 道 具 ─");
    title.setPosition(560.f, 422.f);
    w.draw(title);

    if (m_items.empty()) {
        sf::Text empty;
        empty.setFont(m_font);
        empty.setCharacterSize(18);
        empty.setFillColor(sf::Color(180, 180, 180));
        empty.setString(L"背包中无可用道具");
        empty.setPosition(460.f, 480.f);
        w.draw(empty);
    } else {
        int maxShow = 4;
        int start = std::max(0, m_itemCursor - maxShow + 1);

        for (int i = 0; i < maxShow && (start + i) < (int)m_items.size(); ++i) {
            int idx = start + i;
            const auto& item = m_items[idx];
            bool sel = (idx == m_itemCursor);

            float by = 445.f + i * 34.f;
            sf::Color fgCol = sel ? sf::Color(255, 220, 80) : sf::Color(200, 230, 210);

            if (sel) {
                sf::RectangleShape highlight(sf::Vector2f(390.f, 30.f));
                highlight.setFillColor(sf::Color(20, 60, 30));
                highlight.setPosition(400.f, by - 2.f);
                w.draw(highlight);

                sf::Text cur;
                cur.setFont(m_font);
                cur.setCharacterSize(16);
                cur.setFillColor(sf::Color(255, 220, 80));
                cur.setString(L"▶");
                cur.setPosition(402.f, by);
                w.draw(cur);
            }

            sf::Text nameT;
            nameT.setFont(m_font);
            nameT.setCharacterSize(17);
            nameT.setFillColor(fgCol);
            nameT.setString(item.name + L" ×" + std::to_wstring(item.count));
            nameT.setPosition(425.f, by);
            w.draw(nameT);

            sf::Text effT;
            effT.setFont(m_font);
            effT.setCharacterSize(13);
            effT.setFillColor(sf::Color(130, 200, 130));
            effT.setString(item.effect);
            effT.setPosition(640.f, by + 3.f);
            w.draw(effT);
        }
    }

    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 160, 140));
    hint.setString(L"↑↓←→选择  空格使用  X返回");
    hint.setPosition(410.f, 560.f);
    w.draw(hint);
}

// ============================================================
//  粒子渲染
// ============================================================
void CombatState::RenderParticles(sf::RenderWindow& w) {
    for (const auto& p : m_particles) {
        float alpha = (p.life / p.maxLife) * 255.f;
        sf::CircleShape c(3.f);
        c.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, (sf::Uint8)alpha));
        c.setPosition(p.x, p.y);
        w.draw(c);
    }
}

// ============================================================
//  战斗结束画面
// ============================================================
void CombatState::RenderEndScreen(sf::RenderWindow& w) {
    // 半透明遮罩
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    w.draw(overlay);

    // 结果框
    DrawBox(w, 150.f, 170.f, 500.f, 220.f,
            m_result == CombatResult::Victory ? sf::Color(20,30,15,240) : sf::Color(30,10,10,240),
            m_result == CombatResult::Victory ? sf::Color(100,200,100) : sf::Color(200,60,60),
            3.f);

    sf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(40);
    if (m_result == CombatResult::Victory) {
        title.setString(L"✦ 战 斗 胜 利 ✦");
        title.setFillColor(sf::Color(255, 215, 80));
    } else {
        title.setString(L"✦ 战 斗 失 败 ✦");
        title.setFillColor(sf::Color(220, 80, 80));
    }
    title.setPosition(175.f, 185.f);
    w.draw(title);

    // 奖励信息（胜利时）
    if (m_result == CombatResult::Victory) {
        sf::Text reward;
        reward.setFont(m_font);
        reward.setCharacterSize(18);
        reward.setFillColor(sf::Color(200, 230, 200));
        reward.setString(L"  获得灵石: " + std::to_wstring(m_goldReward) +
                         L"\n  修炼经验: +" + std::to_wstring(m_expReward));
        reward.setPosition(240.f, 250.f);
        w.draw(reward);
    }

    // 闪烁提示
    float blink = std::fmod(m_animTimer, 1.0f);
    if (blink < 0.6f) {
        sf::Text hint;
        hint.setFont(m_font);
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(160, 160, 180));
        hint.setString(L"按 Enter / 空格 返回地图");
        hint.setPosition(290.f, 340.f);
        w.draw(hint);
    }
}
