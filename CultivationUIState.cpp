#pragma execution_character_set("utf-8")
#define NOMINMAX
#include "CultivationUIState.h"
#include "GameState.h"
#include "Player.h"
#include "CultivationSystem.h"
#include "ConfigManager.h"
#include "GameSession.h"
#include "QuestSystem.h"          // 任务系统
#include "GameCallbacks.h"        // 游戏回调
#include <cmath>
#include <Windows.h>

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

static CultivationSystem g_cultiSys;

// ============================================================
//  属性名映射
// ============================================================
std::wstring CultivationUIState::GetAttrName(const std::string& attr) {
    if (attr == "hp")      return L"生命";
    if (attr == "mp")      return L"法力";
    if (attr == "atk")     return L"攻击";
    if (attr == "def")     return L"防御";
    if (attr == "speed" || attr == "spd") return L"速度";
    if (attr == "spirit")  return L"神识";
    return L"未知";
}

int CultivationUIState::GetAttrBonus(const std::string& attr, const std::string& techId, int level) {
    const auto* t = ConfigManager::Instance().GetTechnique(techId);
    if (!t) return 0;
    if (attr == "hp")      return t->hpPerLevel * level;
    if (attr == "mp")      return t->mpPerLevel * level;
    if (attr == "atk")     return t->atkPerLevel * level;
    if (attr == "def")     return t->defPerLevel * level;
    if (attr == "spd")     return t->speedPerLevel * level;
    if (attr == "spirit")  return t->spiritPerLevel * level;
    return 0;
}

// ============================================================
//  功法经验系统（内存存储）
// ============================================================
// 功法经验系统已替换为功法点数系统，所有升级消耗对应境界的功法点数
bool CultivationUIState::CanUpgradeTech(const std::string& techId) {
    const auto* lt = GameSession::Instance().GetPlayer().GetLearned(techId);
    if (!lt) return false;
    const auto* t = ConfigManager::Instance().GetTechnique(techId);
    if (!t || lt->level >= t->maxLevel) return false;
    // 检查功法所属境界是否有可用点数
    std::string realm = Player::GetRealmFromTechnique(t->cultivationReq);
    return GameSession::Instance().GetPlayer().GetTechniquePoints(realm) >= 1;
}

// ============================================================
//  构造函数 —— 重新布局所有 UI 元素位置
//
//  布局规划（800x600）:
//  ┌──────────────────────────────────────────────┐
//  │           标题: — 修 炼 —                      │ y12-40
//  ├─────────────────┬─────────────────────────────┤
//  │  境界面板       │  功法列表面板                │
//  │  (左上)         │  (右上)                      │
//  │  20,50~380,260  │  410,50~780,280             │
//  ├─────────────────┤                             │
//  │  属性面板       │  功法详情面板                │
//  │  (左下)         │  (右下)                      │
//  │  20,268~380,430 │  410,290~780,440            │
//  ├─────────────────┴─────────────────────────────┤
//  │  战斗技能预览面板 (横跨底部上方)               │ y438~520
//  ├───────────────────────────────────────────────┤
//  │  消息区域                                     │ y522~556
//  │  操作提示 C/U/B/ESC                           │ y560~580
//  └───────────────────────────────────────────────┘
// ============================================================
CultivationUIState::CultivationUIState() {
    m_type = GameStateType::CultivationUI;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    // ===== 标题 =====
    m_titleText.setFont(m_font);
    m_titleText.setString(L"— 修 炼 —");
    m_titleText.setCharacterSize(26);
    m_titleText.setFillColor(sf::Color(200, 170, 100));
    m_titleText.setPosition(330.f, 10.f);

    // ===== 境界面板元素 =====
    m_realmName.setFont(m_font);
    m_realmName.setCharacterSize(22);

    m_expBarLabel.setFont(m_font);
    m_expBarLabel.setCharacterSize(13);
    m_expBarLabel.setFillColor(sf::Color(150, 150, 150));

    m_expText.setFont(m_font);
    m_expText.setCharacterSize(13);
    m_expText.setFillColor(sf::Color(180, 220, 255));

    // ===== 属性面板元素（位置在 attrPanel 内部：y 从 292 开始）=====
    const wchar_t* attrNames[6] = { L"生命上限", L"法力上限", L"攻击力", L"防御力", L"速度", L"神识" };
    for (int i = 0; i < 6; ++i) {
        m_attrLabels[i].setFont(m_font);
        m_attrLabels[i].setString(attrNames[i]);
        m_attrLabels[i].setCharacterSize(14);
        m_attrLabels[i].setFillColor(sf::Color(160, 160, 160));
        // 属性面板标题下方开始：panel_y(268) + title_h(18) + padding(6) = 292
        m_attrLabels[i].setPosition(35.f, 292.f + i * 22.f);

        m_attrValues[i].setFont(m_font);
        m_attrValues[i].setCharacterSize(14);
        m_attrValues[i].setFillColor(sf::Color(230, 230, 200));
        m_attrValues[i].setPosition(155.f, 292.f + i * 22.f);
    }

    // ===== 功法列表面板（右侧上半部分）=====
    m_techTitle.setFont(m_font);
    m_techTitle.setString(L"— 已学功法 —");
    m_techTitle.setCharacterSize(16);
    m_techTitle.setFillColor(sf::Color(180, 150, 100));
    m_techTitle.setPosition(570.f, 58.f);

    for (int i = 0; i < 8; ++i) {
        m_techItems[i].setFont(m_font);
        m_techItems[i].setCharacterSize(14);
        m_techItems[i].setPosition(420.f, 82.f + i * 24.f);
    }

    // ===== 功法详情区域（右侧下半部分）=====
    m_techDetailTitle.setFont(m_font);
    m_techDetailTitle.setCharacterSize(14);

    for (int i = 0; i < 8; ++i) {
        m_techDetailLines[i].setFont(m_font);
        m_techDetailLines[i].setCharacterSize(11);
        m_techDetailLines[i].setPosition(418.f, 316.f + i * 17.f);
    }

    // ===== 战斗技能预览面板 =====
    m_skillTitle.setFont(m_font);
    m_skillTitle.setString(L"◇ 战斗技能 ◇");
    m_skillTitle.setCharacterSize(15);
    m_skillTitle.setFillColor(sf::Color(180, 130, 220));
    m_skillTitle.setPosition(30.f, 442.f);

    for (int i = 0; i < 6; ++i) {
        m_skillItems[i].setFont(m_font);
        m_skillItems[i].setCharacterSize(12);
        m_skillItems[i].setPosition(35.f, 464.f + i * 18.f);
    }

    // ===== 操作提示（底部，确保不超出屏幕）=====
    const wchar_t* hints[4] = {
        L"C 修炼",
        L"U 升级功法",
        L"B 突破境界",
        L"ESC 返回"
    };
    for (int i = 0; i < 4; ++i) {
        m_actionHints[i].setFont(m_font);
        m_actionHints[i].setString(hints[i]);
        m_actionHints[i].setCharacterSize(12);
        m_actionHints[i].setFillColor(sf::Color(140, 140, 150));
        // 均匀分布在底部: 800/4=200 宽度间隔
        m_actionHints[i].setPosition(50.f + i * 195.f, 562.f);
    }

    // ===== 消息文本 =====
    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(15);
}

void CultivationUIState::Enter() {
    m_msgTimer = 0.f;
    m_animTimer = 0.f;
    m_selectedTech = 0;
    // 功法经验系统已移除，功法升级消耗突破获得的功法点数
}

void CultivationUIState::Update(float dt) {
    m_animTimer += dt;
    if (m_msgTimer > 0.f) m_msgTimer -= dt;
}

void CultivationUIState::HandleInput() {}

void CultivationUIState::OnKeyPressed(sf::Keyboard::Key key) {
    const auto& learned = GameSession::Instance().GetPlayer().GetLearned();
    int techCount = (int)learned.size();

    switch (key) {
        case sf::Keyboard::Up:
            if (techCount > 0) m_selectedTech = (m_selectedTech + techCount - 1) % techCount;
            break;
        case sf::Keyboard::Down:
            if (techCount > 0) m_selectedTech = (m_selectedTech + 1) % techCount;
            break;
        case sf::Keyboard::C:
            DoCultivate();
            break;
        case sf::Keyboard::U:
            TryUpgradeTechnique();
            break;
        case sf::Keyboard::B:
            TryBreakthrough();
            break;
        case sf::Keyboard::Escape:
            GameStateManager::Instance().PopState();
            break;
        default: break;
    }
}

// ============================================================
//  修炼：给选中的功法增加经验 + 境界经验
// ============================================================
void CultivationUIState::DoCultivate() {
    auto* player = &GameSession::Instance().GetPlayer();
    if (!player) return;

    const auto& learned = player->GetLearned();
    if (learned.empty()) {
        m_messageText.setString(L"尚未学会任何功法，无法修炼！");
        m_messageText.setFillColor(sf::Color(255, 180, 100));
        m_msgTimer = 2.5f;
        return;
    }

    if (m_selectedTech >= (int)learned.size()) m_selectedTech = 0;

    const auto& lt = learned[m_selectedTech];
    const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);

    auto& culti = player->GetCultiMutable();
    int layerBonus = culti.qiLayer / 2;
    int majorBonus = (int)culti.major * 2;

    // 下调按C收益，主要靠打怪和任务
    int realmGain = 1 + (layerBonus + majorBonus) / 2 + rand() % 2;
    culti.cultivationExp += realmGain;

    std::wstring techName = t ? Utf8ToWide(t->name) : L"未知功法";
    std::wstring msg = L"修炼【" + techName + L"】，境界+" + std::to_wstring(realmGain);

    if (CanUpgradeTech(lt.techniqueId)) {
        msg += L"（按U消耗功法点数升级！）";
        m_messageText.setFillColor(sf::Color(100, 255, 150));
    } else if (culti.cultivationExp >= culti.breakthroughReq) {
        msg += L"（境界经验已满，可突破！）";
        m_messageText.setFillColor(sf::Color(100, 255, 150));
    } else {
        m_messageText.setFillColor(sf::Color(200, 220, 255));
    }

    m_messageText.setString(msg);
    m_msgTimer = 3.f;

    // 通知任务系统：修炼次数+1
    QuestSystem::Instance().UpdateProgress(QuestTargetType::Custom, "cultivate_3", 1);
}

// ============================================================
//  升级选中功法
// ============================================================
void CultivationUIState::TryUpgradeTechnique() {
    auto* player = &GameSession::Instance().GetPlayer();
    if (!player) return;

    const auto& learned = player->GetLearned();
    if (learned.empty() || m_selectedTech >= (int)learned.size()) {
        m_messageText.setString(L"没有可升级的功法！");
        m_messageText.setFillColor(sf::Color(255, 180, 100));
        m_msgTimer = 2.f;
        return;
    }

    const auto& lt = learned[m_selectedTech];
    const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
    if (!t) return;

    if (lt.level >= t->maxLevel) {
        m_messageText.setString(Utf8ToWide(t->name) + L" 已达最高等级！");
        m_messageText.setFillColor(sf::Color(255, 215, 80));
        m_msgTimer = 2.5f;
        return;
    }

    if (!CanUpgradeTech(lt.techniqueId)) {
        std::string realm = Player::GetRealmFromTechnique(t->cultivationReq);
        std::wstring realmW;
        if (realm == "qi") realmW = L"练气";
        else if (realm == "zhuji") realmW = L"筑基";
        else realmW = L"未知";
        int pts = player->GetTechniquePoints(realm);
        m_messageText.setString(L"功法点数不足！需要1点" + realmW + L"点数（当前" + std::to_wstring(pts) + L"点），突破境界可获得2点。");
        m_messageText.setFillColor(sf::Color(255, 180, 100));
        m_msgTimer = 2.5f;
        return;
    }

    // 消耗功法点数升级
    bool ok = player->UpgradeTechnique(lt.techniqueId);
    if (!ok) {
        m_messageText.setString(L"升级失败！");
        m_messageText.setFillColor(sf::Color(255, 120, 100));
        m_msgTimer = 2.f;
        return;
    }

    const auto* updatedLt = player->GetLearned(lt.techniqueId);
    int newLevel = updatedLt ? updatedLt->level : lt.level;

    std::wstring techNameW = Utf8ToWide(t->name);
    m_messageText.setString(L"★ " + techNameW + L" 升级成功！→ Lv." + std::to_wstring(newLevel));

    // 通知任务系统：功法升级
    QuestSystem::Instance().UpdateProgress(QuestTargetType::UpgradeTechnique, lt.techniqueId, 1);

    if (!t->unlocksSkill.empty()) {
        for (const auto& us : t->unlocksSkill) {
            if (us.level == newLevel) {
                m_msgTimer = 5.f;
                m_messageText.setFillColor(sf::Color(100, 255, 130));
                break;
            }
        }
    } else {
        m_messageText.setFillColor(sf::Color(100, 255, 130));
        m_msgTimer = 3.f;
    }
}

void CultivationUIState::TryBreakthrough() {
    auto* player = &GameSession::Instance().GetPlayer();
    if (!player) return;

    auto& culti = player->GetCultiMutable();

    if (culti.cultivationExp < culti.breakthroughReq) {
        int need = culti.breakthroughReq - culti.cultivationExp;
        m_messageText.setString(L"境界经验不足！还需 " + std::to_wstring(need) + L" 点经验。继续修炼或打怪积累吧。");
        m_messageText.setFillColor(sf::Color(255, 180, 100));
        m_msgTimer = 2.5f;
        return;
    }

    // 检查突破丹
    std::string pillId = player->GetRequiredBreakthroughPill();
    std::wstring pillName;
    if (pillId == "qi_breakthrough_pill") pillName = L"练气丹";
    else if (pillId == "zhuji_breakthrough_pill") pillName = L"筑基丹";
    else if (pillId == "jiedan_breakthrough_pill") pillName = L"结金丹";
    else pillName = L"未知丹药";

    auto& inv = GameSession::Instance().GetInventory();
    if (!inv.HasItem(pillId, 1)) {
        m_messageText.setString(L"需要【" + pillName + L"】×1 才能突破！丹药师有售，部分任务也会奖励。");
        m_messageText.setFillColor(sf::Color(255, 200, 100));
        m_msgTimer = 3.5f;
        return;
    }

    // 尝试突破（消耗丹药+经验）
    bool ok = player->BreakThrough();
    if (ok) {
        m_messageText.setString(L"★ 突破成功！当前境界：" + player->GetRealmName());
        m_messageText.setFillColor(sf::Color(100, 255, 130));
        // 通知任务系统：突破成功
        QuestSystem::Instance().UpdateProgress(QuestTargetType::Custom, "breakthrough_success", 1);

        // 通知任务系统：检查等级条件
        std::string levelStr = CultivationSystem::GetLevelString(player->GetCulti());
        OnLevelUp(levelStr);
    } else {
        m_messageText.setString(L"已达最高境界，无法再突破！");
        m_messageText.setFillColor(sf::Color(255, 215, 80));
    }
    m_msgTimer = 4.f;
}

// ============================================================
//  根据功法ID和等级返回对应的战斗技能描述
// ============================================================
std::wstring CultivationUIState::GetCombatSkillDesc(const std::string& techId, int level) {
    auto fmt1 = [](float v) -> std::wstring {
        int i = (int)(v * 10);
        return std::to_wstring(i / 10) + L"." + std::to_wstring(i % 10);
    };
    if (techId == "changchun") {
        return L"木灵疗愈 — 消耗" + std::to_wstring(15) + L"MP，回复"
               + std::to_wstring(30 + level * 8) + L"点生命（治疗技能）";
    } else if (techId == "blink_sword") {
        int cost = 12 + level * 2;
        float dmg = 1.4f + level * 0.1f;
        return L"眨眼剑法 — 消耗" + std::to_wstring(cost)
               + L"MP，伤害×" + fmt1(dmg);
    } else if (techId == "qinyuan") {
        int cost = 25 + level * 3;
        float dmg = 1.8f + level * 0.15f;
        return L"青元剑气 — 消耗" + std::to_wstring(cost)
               + L"MP，伤害×" + fmt1(dmg);
    } else if (techId == "yujian") {
        int cost = 20 + level * 2;
        float dmg = 1.6f + level * 0.12f;
        return L"御剑术 — 消耗" + std::to_wstring(cost)
               + L"MP，伤害×" + fmt1(dmg);
    } else if (techId == "luoyan") {
        return L"罗烟步 — 消耗10MP，特殊闪避（下回合防御+50%）";
    }
    return L"—";
}

// ============================================================
//  渲染主函数 —— 使用新布局
// ============================================================
void CultivationUIState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(18, 22, 35));

    window.draw(m_titleText);

    auto* player = &GameSession::Instance().GetPlayer();
    if (!player) return;

    const auto& culti = player->GetCulti();

    // ===== 境界面板（左上: 20,50 ~ 380,260）=====
    {
        sf::RectangleShape realmPanel(sf::Vector2f(360.f, 208.f));
        realmPanel.setFillColor(sf::Color(30, 38, 55, 240));
        realmPanel.setPosition(20.f, 50.f);
        realmPanel.setOutlineThickness(1.f);
        realmPanel.setOutlineColor(sf::Color(70, 90, 120));
        window.draw(realmPanel);

        // 境界名
        m_realmName.setString(L"【" + player->GetRealmName() + L"】  "
                             + L"灵根: " + CultivationSystem::SpiritRootNameW(culti.spiritRoot));
        m_realmName.setCharacterSize(20);
        m_realmName.setFillColor(sf::Color(120, 200, 255));
        m_realmName.setPosition(32.f, 60.f);
        window.draw(m_realmName);

        // 经验条背景
        sf::RectangleShape expBg(sf::Vector2f(320.f, 22.f));
        expBg.setFillColor(sf::Color(40, 30, 30));
        expBg.setPosition(32.f, 95.f);
        window.draw(expBg);

        float expRatio = (float)culti.cultivationExp / (float)culti.breakthroughReq;
        expRatio = std::max(0.f, std::min(1.f, expRatio));

        float pulse = 1.f + 0.03f * std::sin(m_animTimer * 3.f);
        sf::RectangleShape expFill(sf::Vector2f(316.f * expRatio * pulse, 18.f));
        expFill.setFillColor(expRatio >= 1.f ? sf::Color(80, 200, 100) : sf::Color(60, 120, 200));
        expFill.setPosition(34.f, 97.f);
        window.draw(expFill);

        m_expBarLabel.setPosition(32.f, 122.f);
        m_expBarLabel.setString(L"修炼进度");
        window.draw(m_expBarLabel);

        m_expText.setPosition(275.f, 122.f);
        m_expText.setString(std::to_wstring(culti.cultivationExp) + L" / " + std::to_wstring(culti.breakthroughReq));
        window.draw(m_expText);

        // 突破按钮状态
        sf::Text btStatus;
        btStatus.setFont(m_font);
        btStatus.setCharacterSize(12);

        // 显示当前需要的突破丹
        std::string pillId = player->GetRequiredBreakthroughPill();
        std::wstring pillName;
        if (pillId == "qi_breakthrough_pill") pillName = L"练气丹";
        else if (pillId == "zhuji_breakthrough_pill") pillName = L"筑基丹";
        else if (pillId == "jiedan_breakthrough_pill") pillName = L"结金丹";
        else pillName = Utf8ToWide(pillId);
        bool hasPill = GameSession::Instance().GetInventory().HasItem(pillId, 1);

        if (culti.cultivationExp >= culti.breakthroughReq) {
            if (hasPill) {
                btStatus.setString(L"★ 可突破！按 B 键突破 ★");
                btStatus.setFillColor(sf::Color(100, 255, 130));
            } else {
                btStatus.setString(L"需要【" + pillName + L"】×1 才能突破！");
                btStatus.setFillColor(sf::Color(255, 200, 100));
            }
        } else {
            btStatus.setString(L"需要更多修炼经验 + " + pillName + L"×1");
            btStatus.setFillColor(sf::Color(120, 120, 130));
        }
        btStatus.setPosition(32.f, 145.f);
        window.draw(btStatus);

        // 寿元信息
        sf::Text lifespanText(L"", m_font, 12);
        const auto* node = g_cultiSys.GetNode(culti.major, culti.qiLayer, culti.subStage);
        if (node && node->lifespan > 0) {
            lifespanText.setString(L"寿元: +" + std::to_wstring(node->lifespan) + L"年");
            lifespanText.setFillColor(sf::Color(180, 150, 100));
        } else {
            lifespanText.setString(L"寿元: 同凡人");
            lifespanText.setFillColor(sf::Color(130, 130, 130));
        }
        lifespanText.setPosition(32.f, 170.f);
        window.draw(lifespanText);
    }

    // ===== 属性面板（左下: 20,268 ~ 380,434）=====
    {
        sf::RectangleShape attrPanel(sf::Vector2f(360.f, 164.f));
        attrPanel.setFillColor(sf::Color(30, 38, 55, 230));
        attrPanel.setPosition(20.f, 268.f);
        attrPanel.setOutlineThickness(1.f);
        attrPanel.setOutlineColor(sf::Color(60, 75, 95));
        window.draw(attrPanel);

        sf::Text attrTitle(L"— 基础属性 —", m_font, 15);
        attrTitle.setFillColor(sf::Color(160, 140, 100));
        attrTitle.setPosition(135.f, 274.f);
        window.draw(attrTitle);

        int attrs[6] = {
            player->GetMaxHp(),
            player->GetMaxMp(),
            player->GetAttack(),
            player->GetDefense(),
            player->GetSpeed(),
            player->GetSpirit()
        };

        // 属性标签和值（位置已在构造函数中设置好: y=292 起始，间距22）
        for (int i = 0; i < 6; ++i) {
            window.draw(m_attrLabels[i]);
            m_attrValues[i].setString(std::to_wstring(attrs[i]));
            window.draw(m_attrValues[i]);
        }
    }

    // ===== 功法列表面板（右上: 410,50 ~ 780,280）=====
    {
        sf::RectangleShape techPanel(sf::Vector2f(370.f, 228.f));
        techPanel.setFillColor(sf::Color(30, 38, 55, 230));
        techPanel.setPosition(410.f, 50.f);
        techPanel.setOutlineThickness(1.f);
        techPanel.setOutlineColor(sf::Color(60, 75, 95));
        window.draw(techPanel);

        window.draw(m_techTitle);

        const auto& learned = player->GetLearned();
        if (learned.empty()) {
            m_techItems[0].setString(L"  （尚未学会任何功法）");
            m_techItems[0].setFillColor(sf::Color(110, 110, 120));
            window.draw(m_techItems[0]);
        } else {
            for (size_t i = 0; i < learned.size() && i < 8; ++i) {
                const auto* t = ConfigManager::Instance().GetTechnique(learned[i].techniqueId);
                bool selected = ((int)i == m_selectedTech);
                if (t) {
                    std::wstring name = Utf8ToWide(t->name);
                    // 功法境界标签与颜色
                    std::string realm = Player::GetRealmFromTechnique(t->cultivationReq);
                    std::wstring realmTag;
                    sf::Color realmColor;
                    if (realm == "qi")      { realmTag = L"【练气】"; realmColor = sf::Color(102, 187, 106); }
                    else if (realm == "zhuji")  { realmTag = L"【筑基】"; realmColor = sf::Color(66, 165, 245); }
                    else if (realm == "jindan") { realmTag = L"【结丹】"; realmColor = sf::Color(255, 213, 79); }
                    else                    { realmTag = L"";       realmColor = sf::Color(170, 200, 170); }
                    if (selected) {
                        m_techItems[i].setString(realmTag + name + L"  Lv." + std::to_wstring(learned[i].level)
                                                  + L"/" + std::to_wstring(t->maxLevel));
                        m_techItems[i].setFillColor(realmColor);
                        m_techItems[i].setStyle(sf::Text::Bold);
                    } else {
                        m_techItems[i].setString(realmTag + name + L"  Lv." + std::to_wstring(learned[i].level)
                                                  + L"/" + std::to_wstring(t->maxLevel));
                        m_techItems[i].setFillColor(realmColor);
                        m_techItems[i].setStyle(sf::Text::Regular);
                    }
                } else {
                    m_techItems[i].setString(selected ? L"▶ ??? Lv." + std::to_wstring(learned[i].level)
                                                    : L"  ??? Lv." + std::to_wstring(learned[i].level));
                    m_techItems[i].setFillColor(selected ? sf::Color(255, 220, 80) : sf::Color(170, 170, 170));
                }
                window.draw(m_techItems[i]);
            }
        }
    }

    // ===== 选中功法详情（右下: 410,290 ~ 780,458）=====
    {
        sf::RectangleShape detailPanel(sf::Vector2f(370.f, 166.f));
        detailPanel.setFillColor(sf::Color(25, 30, 48, 240));
        detailPanel.setPosition(410.f, 290.f);
        detailPanel.setOutlineThickness(1.f);
        detailPanel.setOutlineColor(sf::Color(70, 60, 100));
        window.draw(detailPanel);

        const auto& learned = player->GetLearned();
        if (!learned.empty() && m_selectedTech < (int)learned.size()) {
            const auto& lt = learned[m_selectedTech];
            const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);

            if (t) {
                std::wstring nameW = Utf8ToWide(t->name);
                m_techDetailTitle.setString(L"◆ " + nameW + L" 详情");
                m_techDetailTitle.setFillColor(sf::Color(200, 170, 255));
                m_techDetailTitle.setPosition(418.f, 298.f);
                window.draw(m_techDetailTitle);

                int line = 0;

                // 行1: 类型 + 属性
                m_techDetailLines[line].setString(
                    L"类型: " + Utf8ToWide(t->type) + L"  |  属性: " + Utf8ToWide(t->element));
                m_techDetailLines[line].setFillColor(sf::Color(170, 170, 190));
                window.draw(m_techDetailLines[line]); line++;

                // 行2: 每级加成（紧凑显示）
                std::wstring effStr = L"加成: ";
                if (t->hpPerLevel) effStr += L"HP+" + std::to_wstring(t->hpPerLevel) + L" ";
                if (t->mpPerLevel) effStr += L"MP+" + std::to_wstring(t->mpPerLevel) + L" ";
                if (t->atkPerLevel) effStr += L"攻+" + std::to_wstring(t->atkPerLevel) + L" ";
                if (t->defPerLevel) effStr += L"防+" + std::to_wstring(t->defPerLevel) + L" ";
                if (t->speedPerLevel) effStr += L"速+" + std::to_wstring(t->speedPerLevel) + L" ";
                if (t->spiritPerLevel) effStr += L"神+" + std::to_wstring(t->spiritPerLevel) + L" ";
                if (effStr == L"加成: ") effStr += L"无";
                m_techDetailLines[line].setString(effStr);
                m_techDetailLines[line].setFillColor(sf::Color(150, 200, 150));
                window.draw(m_techDetailLines[line]); line++;

                // 行3: 描述
                m_techDetailLines[line].setString(
                    L"描述: " + Utf8ToWide(t->description));
                m_techDetailLines[line].setFillColor(sf::Color(180, 180, 180));
                window.draw(m_techDetailLines[line]); line++;

                // 行4: 偏向
                m_techDetailLines[line].setString(
                    L"偏向: " + GetAttrName(t->cultBonusPrimary) + L" / "
                    + GetAttrName(t->cultBonusSecondary)
                    + L"  |  Lv." + std::to_wstring(lt.level) + L"/" + std::to_wstring(t->maxLevel));
                m_techDetailLines[line].setFillColor(sf::Color(180, 180, 120));
                window.draw(m_techDetailLines[line]); line++;

                // 行5: 功法点数信息
                std::string realm = Player::GetRealmFromTechnique(t->cultivationReq);
                int pts = GameSession::Instance().GetPlayer().GetTechniquePoints(realm);
                bool canUp = CanUpgradeTech(lt.techniqueId);

                m_techDetailLines[line].setString(
                    L"点数: " + std::to_wstring(pts) + L"点可用"
                    + (canUp ? L" [按U升级]" : L" (突破得2点)"));
                m_techDetailLines[line].setFillColor(
                    canUp ? sf::Color(100, 255, 130) : sf::Color(140, 170, 200));
                window.draw(m_techDetailLines[line]);

                // 代替旧经验条的简化升级提示
                float barX = 620.f;
                float barY = 316.f + line * 17.f + 3.f;
                sf::RectangleShape ptsBg(sf::Vector2f(90.f, 7.f));
                ptsBg.setFillColor(sf::Color(40, 40, 50));
                ptsBg.setPosition(barX, barY);
                window.draw(ptsBg);

                line++;

                // 行6-7: 解锁技能列表
                if (t && !t->unlocksSkill.empty()) {
                    for (const auto& us : t->unlocksSkill) {
                        if (line >= 7) break;
                        if (us.level <= lt.level) {
                            m_techDetailLines[line].setString(
                                L"✓ Lv." + std::to_wstring(us.level) + L": " + Utf8ToWide(us.skillName));
                            m_techDetailLines[line].setFillColor(sf::Color(100, 255, 150));
                        } else {
                            m_techDetailLines[line].setString(
                                L"  Lv." + std::to_wstring(us.level) + L": " + Utf8ToWide(us.skillName) + L"(未解锁)");
                            m_techDetailLines[line].setFillColor(sf::Color(120, 120, 140));
                        }
                        window.draw(m_techDetailLines[line]); line++;
                    }
                }

                // 清空剩余行
                for (; line < 8; ++line) {
                    m_techDetailLines[line].setString(L"");
                    window.draw(m_techDetailLines[line]);
                }
            }
        } else {
            m_techDetailTitle.setString(L"↑↓ 选择一个功法查看详情");
            m_techDetailTitle.setFillColor(sf::Color(120, 120, 140));
            m_techDetailTitle.setPosition(500.f, 360.f);
            window.draw(m_techDetailTitle);
            for (int i = 0; i < 8; ++i) { m_techDetailLines[i].setString(L""); window.draw(m_techDetailLines[i]); }
        }
    }

    // ===== 战斗技能预览面板（底部横跨: 18,438 ~ 780,518）=====
    {
        sf::RectangleShape skillPanel(sf::Vector2f(764.f, 76.f));
        skillPanel.setFillColor(sf::Color(25, 20, 40, 230));
        skillPanel.setPosition(18.f, 438.f);
        skillPanel.setOutlineThickness(1.f);
        skillPanel.setOutlineColor(sf::Color(80, 60, 110));
        window.draw(skillPanel);

        window.draw(m_skillTitle);

        const auto& learned = player->GetLearned();
        if (learned.empty()) {
            m_skillItems[0].setString(L"  （学会功法后显示对应战斗技能）");
            m_skillItems[0].setFillColor(sf::Color(120, 115, 135));
            window.draw(m_skillItems[0]);
            for (int i = 1; i < 6; ++i) { m_skillItems[i].setString(L""); window.draw(m_skillItems[i]); }
        } else if (m_selectedTech < (int)learned.size()) {
            const auto& lt = learned[m_selectedTech];
            const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
            int line = 0;

            std::wstring currentSkill = GetCombatSkillDesc(lt.techniqueId, lt.level);
            m_skillItems[line].setString(L"▶ 当前战斗技能:");
            m_skillItems[line].setFillColor(sf::Color(200, 170, 255));
            window.draw(m_skillItems[line]); line++;

            m_skillItems[line].setString(L"   " + currentSkill);
            m_skillItems[line].setFillColor(sf::Color(160, 210, 180));
            window.draw(m_skillItems[line]); line++;

            if (t && !t->unlocksSkill.empty()) {
                for (const auto& us : t->unlocksSkill) {
                    if (line >= 5) break;
                    std::wstring prefix;
                    if (us.level <= lt.level) {
                        prefix = L"✓ Lv." + std::to_wstring(us.level) + L": ";
                        m_skillItems[line].setFillColor(sf::Color(100, 255, 150));
                    } else {
                        prefix = L"  Lv." + std::to_wstring(us.level) + L": ";
                        m_skillItems[line].setFillColor(sf::Color(120, 120, 140));
                    }
                    m_skillItems[line].setString(prefix + Utf8ToWide(us.skillName)
                                                   + L" - " + Utf8ToWide(us.skillDesc));
                    window.draw(m_skillItems[line]); line++;
                }
            }
            for (; line < 6; ++line) { m_skillItems[line].setString(L""); window.draw(m_skillItems[line]); }
        }
    }

    // ===== 操作提示（底部 y=562，确保不越界）=====
    for (int i = 0; i < 4; ++i)
        window.draw(m_actionHints[i]);

    // ===== 消息区域（操作提示上方: y=522 ~ 556）=====
    if (m_msgTimer > 0.f) {
        sf::RectangleShape msgBox(sf::Vector2f(760.f, 32.f));
        msgBox.setFillColor(sf::Color(20, 25, 40, 230));
        msgBox.setPosition(20.f, 522.f);
        msgBox.setOutlineThickness(1.f);
        msgBox.setOutlineColor(sf::Color(80, 100, 80));
        window.draw(msgBox);

        m_messageText.setPosition(35.f, 529.f);
        window.draw(m_messageText);
    }
}
