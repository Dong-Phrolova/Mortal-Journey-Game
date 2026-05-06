#pragma execution_character_set("utf-8")
#define NOMINMAX
#include "InventoryState.h"
#include "GameState.h"
#include "Player.h"
#include "ConfigManager.h"
#include "GameSession.h"
#include "ItemSystem.h"
#include "EquipmentSystem.h"
#include <Windows.h>

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// 辅助函数：枚举转字符串
static std::wstring ItemTypeToWString(ItemType t) {
    switch (t) {
        case ItemType::Material: return L"材料";
        case ItemType::Medicine: return L"丹药";
        case ItemType::Treasure: return L"宝物";
        case ItemType::Weapon: return L"武器";
        case ItemType::Armor: return L"防具";
        case ItemType::Accessory: return L"饰品";
        case ItemType::TechniqueBook: return L"功法秘籍";
        case ItemType::Misc: return L"杂物";
        default: return L"未知";
    }
}

static std::wstring EffectTypeToWString(EffectType t) {
    switch (t) {
        case EffectType::None: return L"";
        case EffectType::RestoreHp: return L"恢复生命";
        case EffectType::RestoreMp: return L"恢复法力";
        case EffectType::BuffAtk: return L"攻击加成";
        case EffectType::BuffDef: return L"防御加成";
        case EffectType::ExpBoost: return L"经验加成";
        case EffectType::BreakthroughAid: return L"突破辅助";
        case EffectType::PermanentHp: return L"永久生命";
        case EffectType::PermanentMp: return L"永久法力";
        default: return L"未知";
    }
}

InventoryState::InventoryState() {
    m_type = GameStateType::Inventory;
    m_inventory = &GameSession::Instance().GetInventory();

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    m_titleText.setFont(m_font);
    m_titleText.setString(L"— 背 包 —");
    m_titleText.setCharacterSize(26);
    m_titleText.setFillColor(sf::Color(200, 170, 100));
    m_titleText.setPosition(340.f, 12.f);

    for (int i = 0; i < 12; ++i) {
        m_itemList[i].setFont(m_font);
        m_itemList[i].setCharacterSize(15);
        m_itemList[i].setPosition(50.f, 60.f + i * 32.f);
    }

    m_itemDetail.setFont(m_font);
    m_itemDetail.setCharacterSize(14);

    m_goldText.setFont(m_font);
    m_goldText.setCharacterSize(16);
    m_goldText.setFillColor(sf::Color(255, 215, 80));

    m_hintText.setFont(m_font);
    m_hintText.setString(L"↑↓ 选择   Enter 使用   E 装备/卸下   S 出售   ESC 返回");
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(130, 130, 130));

    m_msgText.setFont(m_font);
    m_msgText.setCharacterSize(15);
}

void InventoryState::Enter() {
    m_selected = 0;
    m_scrollOffset = 0;
    m_msgTimer = 0.f;
}

void InventoryState::Update(float dt) {
    if (m_msgTimer > 0.f) m_msgTimer -= dt;
}

void InventoryState::HandleInput() {}

void InventoryState::OnKeyPressed(sf::Keyboard::Key key) {
    if (!m_inventory) return;

    const auto& items = m_inventory->GetItems();
    int count = static_cast<int>(items.size());

    switch (key) {
        case sf::Keyboard::Up:
            m_selected = std::max(0, m_selected - 1);
            if (m_selected < m_scrollOffset) m_scrollOffset = m_selected;
            break;
        case sf::Keyboard::Down:
            m_selected = std::min(count - 1, m_selected + 1);
            if (m_selected >= m_scrollOffset + 11)
                m_scrollOffset = m_selected - 10;
            break;
        case sf::Keyboard::Return:
            UseSelectedItem();
            break;
        case sf::Keyboard::E:
            EquipSelectedItem();
            break;
        case sf::Keyboard::S:
            SellSelectedItem();
            break;
        case sf::Keyboard::D:
            DiscardSelectedItem();
            break;
        case sf::Keyboard::Escape:
            GameStateManager::Instance().PopState();
            break;
        default: break;
    }
}

void InventoryState::UseSelectedItem() {
    if (!m_inventory) return;

    auto* player = &GameSession::Instance().GetPlayer();

    const auto& items = m_inventory->GetItems();
    if (m_selected < 0 || m_selected >= (int)items.size()) return;

    const auto& item = items[m_selected];
    const auto* data = ConfigManager::Instance().GetItem(item.itemId);
    if (!data) return;

    std::wstring name = Utf8ToWide(data->name);

    if (data->type == ItemType::TechniqueBook) {
        m_msgText.setString(name + L" 是功法秘籍，需要在安全处研读！");
        m_msgText.setFillColor(sf::Color(255, 200, 100));
        m_msgTimer = 2.5f;
        return;
    }

    bool ok = m_inventory->UseItem(item.itemId, *player);
    if (ok) {
        m_msgText.setString(L"使用了 " + name);
        m_msgText.setFillColor(sf::Color(100, 230, 150));
        if (m_selected >= (int)m_inventory->GetSize())
            m_selected = std::max(0, (int)m_inventory->GetSize() - 1);
    } else {
        m_msgText.setString(L"无法使用 " + name);
        m_msgText.setFillColor(sf::Color(255, 120, 100));
    }
    m_msgTimer = 2.5f;
}

void InventoryState::DiscardSelectedItem() {
    if (!m_inventory) return;

    const auto& items = m_inventory->GetItems();
    if (m_selected < 0 || m_selected >= (int)items.size()) return;

    const auto& item = items[m_selected];
    const auto* data = ConfigManager::Instance().GetItem(item.itemId);
    std::wstring name = data ? Utf8ToWide(data->name) : L"???";

    m_inventory->RemoveItem(item.itemId, 1);
    m_msgText.setString(L"丢弃了 x1 " + name);
    m_msgText.setFillColor(sf::Color(200, 160, 140));
    m_msgTimer = 2.f;

    if (m_selected >= (int)m_inventory->GetSize())
        m_selected = std::max(0, (int)m_inventory->GetSize() - 1);
}

void InventoryState::EquipSelectedItem() {
    if (!m_inventory) return;
    const auto& items = m_inventory->GetItems();
    if (m_selected < 0 || m_selected >= (int)items.size()) return;

    const auto& invItem = items[m_selected];
    const auto* data = ConfigManager::Instance().GetItem(invItem.itemId);
    if (!data) return;

    std::wstring name = Utf8ToWide(data->name);

    // 判断是否为装备类型
    if (data->type != ItemType::Weapon && data->type != ItemType::Armor && data->type != ItemType::Accessory) {
        // 检查是否有对应ID的装备数据
        const auto* equipData = EquipmentSystem::Instance().GetEquipment(invItem.itemId);
        if (!equipData) {
            m_msgText.setString(name + L" 不是可装备物品");
            m_msgText.setFillColor(sf::Color(255, 200, 100));
            m_msgTimer = 2.5f;
            return;
        }
    }

    // 尝试装备
    std::string replaced = EquipmentSystem::Instance().Equip(invItem.itemId);
    if (replaced.empty()) {
        // 新装备，从背包移除
        m_inventory->RemoveItem(invItem.itemId, 1);
        m_msgText.setString(L"装备了 " + name);
        m_msgText.setFillColor(sf::Color(100, 230, 150));
    } else {
        // 替换装备：旧装备回背包
        m_inventory->RemoveItem(invItem.itemId, 1);
        if (!replaced.empty()) {
            m_inventory->AddItem(replaced, 1);
            const auto* oldData = ConfigManager::Instance().GetItem(replaced);
            std::wstring oldName = oldData ? Utf8ToWide(oldData->name) : L"???";
            m_msgText.setString(L"装备了 " + name + L"，替换了 " + oldName);
        } else {
            m_msgText.setString(L"装备了 " + name);
        }
        m_msgText.setFillColor(sf::Color(100, 230, 150));
    }
    m_msgTimer = 2.5f;

    if (m_selected >= (int)m_inventory->GetSize())
        m_selected = std::max(0, (int)m_inventory->GetSize() - 1);
}

void InventoryState::SellSelectedItem() {
    if (!m_inventory) return;
    const auto& items = m_inventory->GetItems();
    if (m_selected < 0 || m_selected >= (int)items.size()) return;

    const auto& invItem = items[m_selected];
    const auto* data = ConfigManager::Instance().GetItem(invItem.itemId);
    if (!data) return;

    std::wstring name = Utf8ToWide(data->name);
    int value = data->value / 2;  // 半价回收
    if (value <= 0) value = 1;

    auto& player = GameSession::Instance().GetPlayer();
    player.AddGold(value);

    m_inventory->RemoveItem(invItem.itemId, 1);
    m_msgText.setString(L"出售了 " + name + L" 获得 " + std::to_wstring(value) + L" 灵石");
    m_msgText.setFillColor(sf::Color(255, 215, 80));
    m_msgTimer = 2.5f;

    if (m_selected >= (int)m_inventory->GetSize())
        m_selected = std::max(0, (int)m_inventory->GetSize() - 1);
}

void InventoryState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(18, 20, 30));

    window.draw(m_titleText);

    // 物品列表面板
    sf::RectangleShape listPanel(sf::Vector2f(380.f, 420.f));
    listPanel.setFillColor(sf::Color(28, 33, 48, 240));
    listPanel.setPosition(25.f, 52.f);
    listPanel.setOutlineThickness(1.f);
    listPanel.setOutlineColor(sf::Color(55, 70, 90));
    window.draw(listPanel);

    // 物品详情面板
    sf::RectangleShape detailPanel(sf::Vector2f(370.f, 420.f));
    detailPanel.setFillColor(sf::Color(28, 33, 48, 235));
    detailPanel.setPosition(418.f, 52.f);
    detailPanel.setOutlineThickness(1.f);
    detailPanel.setOutlineColor(sf::Color(55, 70, 90));
    window.draw(detailPanel);

    // 渲染物品列表
    const auto& items = m_inventory ? m_inventory->GetItems() : std::vector<InventoryItem>();

    for (int i = 0; i < 12; ++i) {
        int idx = m_scrollOffset + i;
        if (idx < 0 || idx >= (int)items.size()) {
            m_itemList[i].setString(L"");
            window.draw(m_itemList[i]);
            continue;
        }

        const auto& invItem = items[idx];
        const auto* data = ConfigManager::Instance().GetItem(invItem.itemId);
        bool selected = (idx == m_selected);

        if (data) {
            std::wstring name = Utf8ToWide(data->name);
            std::wstring prefix = selected ? L"▶ " : L"  ";
            std::wstring suffix = invItem.count > 1 ? L" x" + std::to_wstring(invItem.count) : L"";

            m_itemList[i].setString(prefix + name + suffix);
        } else {
            m_itemList[i].setString(selected ? L"▶ ???" : L"  ???");
        }

        m_itemList[i].setFillColor(selected ? sf::Color(255, 220, 100) : sf::Color(175, 180, 190));
        m_itemList[i].setStyle(selected ? sf::Text::Bold : sf::Text::Regular);
        window.draw(m_itemList[i]);
    }

    // 物品详情
    if (m_selected >= 0 && m_selected < (int)items.size()) {
        const auto* data = ConfigManager::Instance().GetItem(items[m_selected].itemId);
        if (data) {
            sf::Text detailTitle(L"— 物品详情 —", m_font, 17);
            detailTitle.setFillColor(sf::Color(170, 145, 95));
            detailTitle.setPosition(550.f, 62.f);
            window.draw(detailTitle);

            std::wstring name = Utf8ToWide(data->name);
            std::wstring desc = Utf8ToWide(data->description);
            std::wstring type = ItemTypeToWString(data->type);

            sf::Text n(name, m_font, 22);
            n.setFillColor(sf::Color(230, 210, 150));
            n.setPosition(435.f, 92.f);
            window.draw(n);

            sf::Text t(L"类型: " + ItemTypeToWString(data->type), m_font, 15);
            t.setFillColor(sf::Color(150, 150, 170));
            t.setPosition(435.f, 125.f);
            window.draw(t);

            // 描述文字自动换行（每行约25个中文字符，防止溢出面板）
            std::wstring wrappedDesc;
            int maxLineLen = 25;
            int lineLen = 0;
            for (size_t ci = 0; ci < desc.size(); ++ci) {
                wrappedDesc += desc[ci];
                ++lineLen;
                // 在标点或空格处断行
                if (lineLen >= maxLineLen && (ci + 1 < desc.size())) {
                    wchar_t c = desc[ci];
                    if (c == L'。' || c == L'，' || c == L'！' || c == L'？' || c == L'；' || c == L'）' || c == L'」' || c == L' ' || c == L'）' || c == L'\n') {
                        wrappedDesc += L'\n';
                        lineLen = 0;
                    }
                }
            }

            sf::Text d(wrappedDesc, m_font, 14);
            d.setFillColor(sf::Color(170, 170, 165));
            d.setPosition(435.f, 155.f);
            d.setLineSpacing(3.f);
            window.draw(d);

            sf::Text val(L"售价: " + std::to_wstring(data->value) + L" 灵石", m_font, 14);
            val.setFillColor(sf::Color(180, 160, 80));
            val.setPosition(435.f, 250.f);
            window.draw(val);

            // 效果信息
            if (data->effectType != EffectType::None) {
                sf::Text eff(L"效果: " + EffectTypeToWString(data->effectType) +
                    (data->effectValue > 0 ? L" +" + std::to_wstring(data->effectValue) : L""), m_font, 14);
                eff.setFillColor(sf::Color(100, 200, 150));
                eff.setPosition(435.f, 278.f);
                window.draw(eff);
            }

            // 装备属性显示（如果该物品是装备）
            const auto* equipData = EquipmentSystem::Instance().GetEquipment(items[m_selected].itemId);
            if (equipData) {
                float ey = 305.f;
                auto drawStat = [&](const std::wstring& label, int val, float y) {
                    if (val > 0) {
                        sf::Text stat(label + L" +" + std::to_wstring(val), m_font, 13);
                        stat.setFillColor(sf::Color(140, 200, 255));
                        stat.setPosition(435.f, y);
                        window.draw(stat);
                    }
                };
                drawStat(L"攻击", equipData->atkBonus, ey); ey += 18.f;
                drawStat(L"防御", equipData->defBonus, ey); ey += 18.f;
                drawStat(L"生命", equipData->hpBonus, ey); ey += 18.f;
                drawStat(L"法力", equipData->mpBonus, ey); ey += 18.f;
                drawStat(L"速度", equipData->spdBonus, ey); ey += 18.f;
                drawStat(L"神识", equipData->spiritBonus, ey);
            }
        }
    }

    // 当前装备显示
    {
        float ex = 435.f, ey = 405.f;  // 上移到405避免溢出面板
        sf::Text equipTitle(L"— 当前装备 —", m_font, 15);
        equipTitle.setFillColor(sf::Color(170, 145, 95));
        equipTitle.setPosition(ex, ey); ey += 22.f;
        window.draw(equipTitle);

        auto drawSlot = [&](const std::wstring& slotName, EquipSlot slot, float y) {
            sf::Text label(slotName, m_font, 13);
            label.setFillColor(sf::Color(150, 150, 170));
            label.setPosition(ex, y);
            window.draw(label);
            const auto* e = EquipmentSystem::Instance().GetEquipped(slot);
            std::wstring equipName = e ? Utf8ToWide(e->name) : L"(空)";
            sf::Text ename(equipName, m_font, 13);
            ename.setFillColor(e ? sf::Color(230, 210, 150) : sf::Color(120, 120, 130));
            ename.setPosition(ex + 80.f, y);
            window.draw(ename);
        };
        drawSlot(L"武器", EquipSlot::Weapon, ey); ey += 18.f;
        drawSlot(L"防具", EquipSlot::Armor, ey); ey += 18.f;
        drawSlot(L"饰品", EquipSlot::Accessory, ey); ey += 18.f;
    }

    // 底部状态栏
    auto* player = &GameSession::Instance().GetPlayer();
    if (player) {
        std::wstring goldStr = L"灵石: " + std::to_wstring(player->GetGold())
                               + L"    |    背包: "
                               + std::to_wstring(items.size()) + L"/"
                               + std::to_wstring(InventorySystem::MAX_SLOTS);
        m_goldText.setString(goldStr);
        m_goldText.setPosition(25.f, 565.f);
        window.draw(m_goldText);
    }

    m_hintText.setPosition(350.f, 579.f);
    window.draw(m_hintText);

    // 消息
    if (m_msgTimer > 0.f) {
        sf::RectangleShape msgBox(sf::Vector2f(740.f, 34.f));
        msgBox.setFillColor(sf::Color(20, 25, 38, 230));
        msgBox.setPosition(30.f, 520.f);
        window.draw(msgBox);

        m_msgText.setPosition(45.f, 528.f);
        window.draw(m_msgText);
    }
}
