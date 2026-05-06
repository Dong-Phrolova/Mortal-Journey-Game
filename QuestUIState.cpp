#pragma execution_character_set("utf-8")
#define NOMINMAX
#include "QuestUIState.h"
#include "QuestSystem.h"
#include "GameSession.h"
#include "ConfigManager.h"
#include <Windows.h>

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// 文本自动换行：将长文本按指定字符数拆分为多行
static std::vector<std::wstring> WrapText(const std::wstring& text, int maxCharsPerLine) {
    std::vector<std::wstring> lines;
    std::wstring currentLine;
    for (wchar_t ch : text) {
        if (ch == L'\n') {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }
        currentLine += ch;
        if (currentLine.size() >= (size_t)maxCharsPerLine) {
            size_t breakPos = currentLine.find_last_of(L" ，。、）)");
            if (breakPos != std::wstring::npos && breakPos > 4) {
                lines.push_back(currentLine.substr(0, breakPos));
                currentLine = currentLine.substr(breakPos);
            } else {
                lines.push_back(currentLine.substr(0, currentLine.size() - 1));
                currentLine = currentLine.substr(currentLine.size() - 1);
            }
        }
    }
    if (!currentLine.empty())
        lines.push_back(currentLine);
    return lines;
}

// ============================================================
//  构造函数
// ============================================================
QuestUIState::QuestUIState() {
    m_type = GameStateType::QuestUI;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    // 标题
    m_titleText.setFont(m_font);
    m_titleText.setString(L"◇ 主线任务 ◇");
    m_titleText.setCharacterSize(24);
    m_titleText.setFillColor(sf::Color(220, 180, 100));
    m_titleText.setPosition(300.f, 12.f);

    // 任务列表
    for (int i = 0; i < 10; ++i) {
        m_questListText[i].setFont(m_font);
        m_questListText[i].setCharacterSize(14);
        m_questListText[i].setPosition(30.f, 55.f + i * 28.f);
    }

    // 详情标题
    m_detailTitle.setFont(m_font);
    m_detailTitle.setCharacterSize(18);

    // 详情行
    for (int i = 0; i < 8; ++i) {
        m_detailLines[i].setFont(m_font);
        m_detailLines[i].setCharacterSize(13);
        m_detailLines[i].setPosition(410.f, 90.f + i * 22.f);
    }

    // 奖励标签
    m_rewardLabel.setFont(m_font);
    m_rewardLabel.setCharacterSize(15);
    m_rewardLabel.setFillColor(sf::Color(255, 215, 80));

    for (int i = 0; i < 4; ++i) {
        m_rewardLines[i].setFont(m_font);
        m_rewardLines[i].setCharacterSize(13);
        m_rewardLines[i].setFillColor(sf::Color(200, 200, 180));
        m_rewardLines[i].setPosition(420.f, 290.f + i * 20.f);
    }

    // 操作提示
    m_hintText.setFont(m_font);
    m_hintText.setString(L"↑↓ 选择   Enter 领取奖励   ESC 返回");
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(140, 140, 150));
    m_hintText.setPosition(260.f, 565.f);

    // 无任务提示
    m_emptyHint.setFont(m_font);
    m_emptyHint.setFillColor(sf::Color(120, 120, 140));
}

void QuestUIState::Enter() {
    m_selectedIndex = 0;
    m_animTimer = 0.f;
    // 确保任务系统已初始化（不会重复初始化）
    QuestSystem::Instance().Initialize();
}

void QuestUIState::Update(float dt) {
    m_animTimer += dt;
    QuestSystem::Instance().UpdateNarration(dt);
}

void QuestUIState::OnKeyPressed(sf::Keyboard::Key key) {
    auto& qs = QuestSystem::Instance();
    const auto& allQuests = qs.GetAllQuests();

    // 收集有效任务索引
    std::vector<int> validIndices;
    for (int i = 0; i < (int)allQuests.size(); ++i) {
        if (allQuests[i].status == QuestStatus::Active ||
            allQuests[i].status == QuestStatus::Completed) {
            validIndices.push_back(i);
        }
    }
    int validCount = (int)validIndices.size();

    switch (key) {
        case sf::Keyboard::Up:
            if (validCount > 0)
                m_selectedIndex = (m_selectedIndex - 1 + validCount) % validCount;
            break;
        case sf::Keyboard::Down:
            if (validCount > 0)
                m_selectedIndex = (m_selectedIndex + 1) % validCount;
            break;
        case sf::Keyboard::Return:
        case sf::Keyboard::Space: {
            // 领取已完成的任务奖励
            if (!validIndices.empty() && m_selectedIndex < (int)validIndices.size()) {
                int qIdx = validIndices[m_selectedIndex];
                const auto& q = allQuests[qIdx];
                if (q.status == QuestStatus::Completed) {
                    qs.ClaimReward(q.id);
                }
            }
            break;
        }
        case sf::Keyboard::Escape:
        case sf::Keyboard::Q:
            GameStateManager::Instance().PopState();
            break;
        default: break;
    }
}

// ============================================================
//  状态字符串
// ============================================================
std::wstring QuestUIState::StatusString(QuestStatus status) const {
    switch (status) {
        case QuestStatus::Locked:    return L"[未解锁]";
        case QuestStatus::Active:    return L"【进行中】";
        case QuestStatus::Completed: return L"★ 可领奖 ★";
        case QuestStatus::Rewarded:  return L"[已完成]";
        default: return L"[?]";
    }
}

// ============================================================
//  渲染
// ============================================================
void QuestUIState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(16, 18, 30, 240));

    // 半透明背景覆盖
    sf::RectangleShape bgOverlay(sf::Vector2f(800.f, 600.f));
    bgOverlay.setFillColor(sf::Color(16, 18, 30, 220));
    window.draw(bgOverlay);

    window.draw(m_titleText);

    auto& qs = QuestSystem::Instance();
    const auto& allQuests = qs.GetAllQuests();

    // 左侧：任务列表面板
    sf::RectangleShape listPanel(sf::Vector2f(370.f, 490.f));
    listPanel.setFillColor(sf::Color(25, 30, 48, 250));
    listPanel.setPosition(15.f, 45.f);
    listPanel.setOutlineThickness(1.f);
    listPanel.setOutlineColor(sf::Color(60, 70, 100));
    window.draw(listPanel);

    int activeCount = 0;
    RenderQuestList(window, activeCount);

    // 右侧：详情面板
    sf::RectangleShape detailPanel(sf::Vector2f(395.f, 490.f));
    detailPanel.setFillColor(sf::Color(25, 30, 48, 250));
    detailPanel.setPosition(395.f, 45.f);
    detailPanel.setOutlineThickness(1.f);
    detailPanel.setOutlineColor(sf::Color(60, 70, 100));
    window.draw(detailPanel);

    // 找到选中任务并渲染详情
    std::vector<int> validIndices;
    for (int i = 0; i < (int)allQuests.size(); ++i) {
        if (allQuests[i].status == QuestStatus::Active ||
            allQuests[i].status == QuestStatus::Completed) {
            validIndices.push_back(i);
        }
    }

    if (!validIndices.empty() && m_selectedIndex < (int)validIndices.size()) {
        const auto* quest = &allQuests[validIndices[m_selectedIndex]];
        RenderQuestDetail(window, quest);
    } else {
        m_emptyHint.setString(L"选择一个任务查看详情");
        m_emptyHint.setCharacterSize(16);
        m_emptyHint.setPosition(530.f, 280.f);
        window.draw(m_emptyHint);
    }

    // 操作提示
    window.draw(m_hintText);

    // 旁白显示（在任务界面中如果触发了旁白，直接确认，因为会在WorldMapState中显示）
    auto* nar = qs.GetCurrentNarration();
    if (nar && qs.IsNarrationWaitingConfirm()) {
        // 任务界面中不显示旁白，等退出后会自动在地图界面显示
    } else if (nar) {
        // 非等待确认模式（自动消失型）- 保持原逻辑
        float elapsed = qs.GetNarrationTimer();
        float dur = nar->duration > 0.f ? nar->duration : 4.f;
        float alphaRatio = std::max(0.f, std::min(1.f, elapsed / dur));

        sf::RectangleShape narBox(sf::Vector2f(760.f, 70.f));
        narBox.setFillColor(sf::Color(10, 10, 20,
            static_cast<sf::Uint8>(200 * alphaRatio)));
        narBox.setPosition(20.f, 520.f);
        narBox.setOutlineThickness(1.f);
        narBox.setOutlineColor(sf::Color(100, 80, 140,
            static_cast<sf::Uint8>(200 * alphaRatio)));
        window.draw(narBox);

        std::wstring narText = qs.GetCurrentNarrationText();
        if (!narText.empty()) {
            sf::Text narDisplay(narText, m_font, 14);
            sf::Color col = (nar->type == QuestSystem::NarrationEvent::Type::InnerThought)
                ? sf::Color(180, 160, 220) : sf::Color(230, 220, 190);
            narDisplay.setFillColor(sf::Color(col.r, col.g, col.b,
                static_cast<sf::Uint8>(255 * alphaRatio)));
            narDisplay.setPosition(35.f, 532.f);
            window.draw(narDisplay);
        }
    }
}

void QuestUIState::RenderQuestList(sf::RenderWindow& window, int& activeCount) {
    auto& qs = QuestSystem::Instance();
    const auto& allQuests = qs.GetAllQuests();

    activeCount = 0;
    int displayIdx = 0;

    for (size_t i = 0; i < allQuests.size() && displayIdx < 10; ++i) {
        const auto& q = allQuests[i];
        if (q.status != QuestStatus::Active && q.status != QuestStatus::Completed)
            continue;

        bool selected = (activeCount == m_selectedIndex);
        std::wstring nameW = Utf8ToWide(q.name);
        std::wstring statusW = StatusString(q.status);

        int done = 0, total = (int)q.objectives.size();
        for (const auto& obj : q.objectives) {
            if (obj.completed) done++;
        }

        std::wstring prefix = selected ? L"▶ " : L"  ";
        m_questListText[displayIdx].setString(
            prefix + nameW + L" " + std::to_wstring(done) + L"/" + std::to_wstring(total));

        if (selected) {
            m_questListText[displayIdx].setFillColor(
                q.status == QuestStatus::Completed ?
                    sf::Color(100, 255, 130) : sf::Color(255, 220, 80));
            m_questListText[displayIdx].setStyle(sf::Text::Bold);

            sf::RectangleShape hl(sf::Vector2f(360.f, 26.f));
            hl.setFillColor(sf::Color(45, 55, 35, 200));
            hl.setPosition(17.f, 53.f + displayIdx * 28.f);
            window.draw(hl);
        } else {
            m_questListText[displayIdx].setFillColor(
                q.status == QuestStatus::Completed ?
                    sf::Color(100, 200, 130) : sf::Color(170, 175, 180));
            m_questListText[displayIdx].setStyle(sf::Text::Regular);
        }

        window.draw(m_questListText[displayIdx]);
        displayIdx++;
        activeCount++;
    }

    if (activeCount == 0) {
        m_questListText[0].setString(L"  暂无进行中的任务...");
        m_questListText[0].setFillColor(sf::Color(110, 110, 120));
        window.draw(m_questListText[0]);
    }
}

void QuestUIState::RenderQuestDetail(sf::RenderWindow& window, const QuestData* quest) {
    if (!quest) return;

    std::wstring nameW = Utf8ToWide(quest->name);
    m_detailTitle.setString(L"◆ " + nameW);
    m_detailTitle.setFillColor(sf::Color(200, 170, 255));
    m_detailTitle.setPosition(410.f, 55.f);
    window.draw(m_detailTitle);

    // 状态
    sf::Text statusText(StatusString(quest->status), m_font, 14);
    statusText.setFillColor(quest->status == QuestStatus::Completed ?
        sf::Color(100, 255, 130) : sf::Color(150, 200, 255));
    statusText.setPosition(680.f, 58.f);
    window.draw(statusText);

    // 描述（自动换行，每行最多28个中文字符）
    int line = 0;
    int descLinesMax = 28;
    auto descLines = WrapText(Utf8ToWide(quest->description), descLinesMax);
    for (size_t i = 0; i < descLines.size() && line < 7; ++i, ++line) {
        m_detailLines[line].setString(L"  " + descLines[i]);
        m_detailLines[line].setFillColor(sf::Color(170, 170, 160));
        m_detailLines[line].setPosition(410.f, 90.f + line * 22.f);
        window.draw(m_detailLines[line]);
    }
    // 如果描述为空，显示原文本
    if (descLines.empty()) {
        m_detailLines[line].setString(L"  " + Utf8ToWide(quest->description));
        m_detailLines[line].setFillColor(sf::Color(170, 170, 160));
        m_detailLines[line].setPosition(410.f, 90.f + line * 22.f);
        window.draw(m_detailLines[line]); line++;
    }

    line++;  // 空行

    // 目标
    m_detailLines[line].setString(L"— 目标 —");
    m_detailLines[line].setFillColor(sf::Color(180, 150, 100));
    window.draw(m_detailLines[line]); line++;

    for (const auto& obj : quest->objectives) {
        if (line >= 7) break;
        std::wstring checkMark = obj.completed ? L"✓" : L"○";
        std::wstring objStr = checkMark + L" " + Utf8ToWide(obj.description);
        if (!obj.completed && obj.requiredCount > 1) {
            objStr += L" (" + std::to_wstring(obj.currentCount) + L"/"
                     + std::to_wstring(obj.requiredCount) + L")";
        }
        m_detailLines[line].setString(objStr);
        m_detailLines[line].setFillColor(obj.completed ?
            sf::Color(100, 220, 130) : sf::Color(150, 150, 155));
        window.draw(m_detailLines[line]); line++;
    }

    for (; line < 7; ++line) {
        m_detailLines[line].setString(L"");
        window.draw(m_detailLines[line]);
    }

    // 奖励预览
    m_rewardLabel.setPosition(410.f, 280.f);
    window.draw(m_rewardLabel);

    int rLine = 0;
    if (quest->reward.gold > 0) {
        m_rewardLines[rLine].setString(L"  灵石 x" + std::to_wstring(quest->reward.gold));
        window.draw(m_rewardLines[rLine]); rLine++;
    }
    if (quest->reward.exp > 0) {
        m_rewardLines[rLine].setString(L"  修炼经验 x" + std::to_wstring(quest->reward.exp));
        window.draw(m_rewardLines[rLine]); rLine++;
    }
    for (const auto& item : quest->reward.items) {
        if (rLine >= 4) break;
        const auto* itemData = ConfigManager::Instance().GetItem(item.itemId);
        std::wstring itemName = itemData ? Utf8ToWide(itemData->name) : Utf8ToWide(item.itemId);
        m_rewardLines[rLine].setString(L"  " + itemName + L" x" + std::to_wstring(item.count));
        window.draw(m_rewardLines[rLine]); rLine++;
    }
    for (; rLine < 4; ++rLine) {
        m_rewardLines[rLine].setString(L"");
        window.draw(m_rewardLines[rLine]);
    }
}
