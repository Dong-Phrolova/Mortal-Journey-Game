#pragma execution_character_set("utf-8")
#define NOMINMAX
#include "ShopState.h"
#include "GameState.h"
#include "Player.h"
#include "GameSession.h"
#include "ConfigManager.h"
#include "EquipmentSystem.h"
#include "QuestSystem.h"
#include <algorithm>
#include <Windows.h>

static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// ============================================================
//  构造函数
// ============================================================
ShopState::ShopState(const std::wstring& shopName, const std::string& shopType)
    : m_shopName(shopName), m_shopType(shopType)
{
    m_type = GameStateType::Shop;

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    // 标题
    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(24);

    // 灵石显示
    m_goldText.setFont(m_font);
    m_goldText.setCharacterSize(16);
    m_goldText.setFillColor(sf::Color(255, 215, 80));

    // 商品列表
    for (int i = 0; i < 8; ++i) {
        m_shopItemText[i].setFont(m_font);
        m_shopItemText[i].setCharacterSize(15);
        m_shopItemText[i].setPosition(50.f, 95.f + i * 32.f);
    }

    // 背包列表（出售模式）
    for (int i = 0; i < 6; ++i) {
        m_invItemText[i].setFont(m_font);
        m_invItemText[i].setCharacterSize(14);
        m_invItemText[i].setPosition(50.f, 95.f + i * 28.f);
    }

    // 详情文本
    for (int i = 0; i < 4; ++i) {
        m_detailText[i].setFont(m_font);
        m_detailText[i].setCharacterSize(14);
        m_detailText[i].setPosition(430.f, 100.f + i * 26.f);
    }

    // 消息
    m_msgText.setFont(m_font);
    m_msgText.setCharacterSize(15);

    // 提示
    m_hintText.setFont(m_font);
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(140, 140, 150));

    InitShopItems();
}

// ============================================================
//  根据商店类型初始化商品列表
// ============================================================
void ShopState::InitShopItems() {
    m_shopItems.clear();

    if (m_shopType == "weapon") {
        // 武器商：卖武器和基础物品
        // 物品类
        ShopItem si; si.itemType = ShopItemType::Item;
        si.itemId = "hp_potion_small";     si.price = 8;   si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "mp_potion_small";     si.price = 12;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "iron_wood_sword";     si.price = 30;  si.stock = 5;
        m_shopItems.push_back(si);

        // 装备类（武器）
        ShopItem se; se.itemType = ShopItemType::Equipment;
        se.itemId = "cold_iron_sword";    se.price = 80;  se.stock = 3;
        m_shopItems.push_back(se);
        se.itemId = "shadow_needle";      se.price = 350; se.stock = 2;
        m_shopItems.push_back(se);

    } else if (m_shopType == "medicine") {
        // 丹药师：卖各种丹药
        ShopItem si; si.itemType = ShopItemType::Item;
        si.itemId = "hp_potion_small";      si.price = 8;   si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "hp_potion_medium";     si.price = 22;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "mp_potion_small";      si.price = 12;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "spirit_gathering_pill"; si.price = 45;  si.stock = 20;
        m_shopItems.push_back(si);
        si.itemId = "breakthrough_pill";   si.price = 650; si.stock = 3;
        m_shopItems.push_back(si);

    } else if (m_shopType == "technique") {
        // ★ 功法商人：售卖功法秘籍 ★
        ShopItem st; st.itemType = ShopItemType::Technique;

        // 长春功 — 墨大夫的基础功法，新手友好
        st.itemId = "changchun";           st.price = 0;    st.stock = 1;   // 免费赠送
        m_shopItems.push_back(st);

        // 眨眼剑法 — 进阶剑技
        st.itemId = "blink_sword";         st.price = 150;  st.stock = 3;
        m_shopItems.push_back(st);

        // 罗烟步 — 身法
        st.itemId = "luoyan";              st.price = 200;  st.stock = 3;
        m_shopItems.push_back(st);

        // 御剑术 — 高级技能
        st.itemId = "yujian";              st.price = 400;  st.stock = 2;
        m_shopItems.push_back(st);

        // 青元剑诀 — 强力功法（贵）
        st.itemId = "qinyuan";             st.price = 800;  st.stock = 1;
        m_shopItems.push_back(st);

    } else if (m_shopType == "equipment") {
        // ★ 装备店：防具+饰品 ★

        // 防具
        ShopItem se; se.itemType = ShopItemType::Equipment;
        se.itemId = "cloth_armor";          se.price = 15;  se.stock = 5;
        m_shopItems.push_back(se);
        se.itemId = "leather_armor";        se.price = 60;  se.stock = 3;
        m_shopItems.push_back(se);
        se.itemId = "steel_armor";          se.price = 280; se.stock = 2;
        m_shopItems.push_back(se);

        // 饰品
        se.itemId = "iron_necklace";        se.price = 20;  se.stock = 5;
        m_shopItems.push_back(se);
        se.itemId = "spirit_ring";         se.price = 100; se.stock = 3;
        m_shopItems.push_back(se);
        se.itemId = "gold_thread_belt";     se.price = 200; se.stock = 2;
        m_shopItems.push_back(se);
        se.itemId = "speed_boots";         se.price = 300; se.stock = 2;
        m_shopItems.push_back(se);

    } else if (m_shopType == "misc") {
        // 杂货商：卖杂货和材料
        ShopItem si; si.itemType = ShopItemType::Item;
        si.itemId = "hp_potion_small";       si.price = 10;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "mp_potion_small";       si.price = 15;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "mystic_ice_jade";       si.price = 150; si.stock = 10;
        m_shopItems.push_back(si);
        si.itemId = "centipede_shell";       si.price = 120; si.stock = 8;
        m_shopItems.push_back(si);

        // 杂货商也卖点基础装备
        ShopItem se; se.itemType = ShopItemType::Equipment;
        se.itemId = "cloth_armor";           se.price = 20;  se.stock = 5;
        m_shopItems.push_back(se);
        se.itemId = "iron_necklace";         se.price = 25;  se.stock = 5;
        m_shopItems.push_back(se);

    } else {
        // general: 综合商店，卖所有基础物品
        ShopItem si; si.itemType = ShopItemType::Item;
        si.itemId = "hp_potion_small";      si.price = 10;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "mp_potion_small";      si.price = 15;  si.stock = 99;
        m_shopItems.push_back(si);
        si.itemId = "spirit_gathering_pill"; si.price = 50;  si.stock = 15;
        m_shopItems.push_back(si);

        ShopItem se; se.itemType = ShopItemType::Equipment;
        se.itemId = "iron_wood_sword";      se.price = 25;  se.stock = 3;
        m_shopItems.push_back(se);
        se.itemId = "cloth_armor";          se.price = 15;  se.stock = 5;
        m_shopItems.push_back(se);
    }
}

void ShopState::Enter() {
    m_selectedItem = 0;
    m_scrollOffset = 0;
    m_sellSelected = 0;
    m_inSellMode = false;
    m_msgTimer = 0.f;
}

void ShopState::Update(float dt) {
    m_animTimer += dt;
    if (m_msgTimer > 0.f) m_msgTimer -= dt;
}

void ShopState::OnKeyPressed(sf::Keyboard::Key key) {
    int itemCount = m_inSellMode
        ? (int)GameSession::Instance().GetInventory().GetItems().size()
        : (int)m_shopItems.size();

    if (key == sf::Keyboard::Left || key == sf::Keyboard::Q) {
        // 切换购买/出售模式
        if (!m_inSellMode) {
            // 背包非空才能切换到出售
            if (!GameSession::Instance().GetInventory().GetItems().empty())
                m_inSellMode = true;
            // 背包空则什么都不做（保持在购买模式）
        } else {
            m_inSellMode = false;
        }
        m_selectedItem = 0;
        m_scrollOffset = 0;
        return;
    }
    if (key == sf::Keyboard::Right) {
        m_inSellMode = false;
        m_selectedItem = 0;
        m_scrollOffset = 0;
        return;
    }

    switch (key) {
        case sf::Keyboard::Up:
            m_selectedItem = std::max(0, m_selectedItem - 1);
            if (m_selectedItem < m_scrollOffset) m_scrollOffset = m_selectedItem;
            break;
        case sf::Keyboard::Down:
            m_selectedItem = std::min(itemCount - 1, m_selectedItem + 1);
            if (m_selectedItem >= m_scrollOffset + 7) m_scrollOffset = m_selectedItem - 6;
            break;
        case sf::Keyboard::Return:
        case sf::Keyboard::Space:
        case sf::Keyboard::Z:
            if (m_inSellMode)
                SellItem(m_selectedItem);   // 暂不实现完整出售逻辑
            else
                BuyItem(m_selectedItem);
            break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::X:
            GameStateManager::Instance().PopState();
            break;
        default: break;
    }
}

void ShopState::BuyItem(int idx) {
    if (idx < 0 || idx >= (int)m_shopItems.size()) return;

    auto& item = m_shopItems[idx];
    auto* player = &GameSession::Instance().GetPlayer();

    // 检查库存
    if (item.stock == 0) {
        m_msgText.setString(L"该商品已售罄！");
        m_msgText.setFillColor(sf::Color(255, 120, 100));
        m_msgTimer = 2.f;
        return;
    }

    int price = item.price;

    // 免费商品（如新手功法赠送）跳过灵石检查
    bool isFree = (price <= 0);
    if (!isFree) {
        // 检查灵石
        if (player->GetGold() < price) {
            m_msgText.setString(L"灵石不足！需要 " + std::to_wstring(price) + L" 灵石");
            m_msgText.setFillColor(sf::Color(255, 120, 100));
            m_msgTimer = 2.f;
            return;
        }
    }

    std::wstring itemName;

    // 根据商品类型分别处理
    if (item.itemType == ShopItemType::Technique) {
        // ===== 购买功法 =====
        const auto* t = ConfigManager::Instance().GetTechnique(item.itemId);
        if (!t) {
            m_msgText.setString(L"功法数据错误！");
            m_msgText.setFillColor(sf::Color(255, 100, 100));
            m_msgTimer = 2.f;
            return;
        }

        // 检查是否已学会
        if (player->GetLearned(item.itemId)) {
            m_msgText.setString(L"已经学会了这个功法！");
            m_msgText.setFillColor(sf::Color(255, 200, 100));
            m_msgTimer = 2.f;
            return;
        }

        // 学习功法
        player->LearnTechnique(item.itemId);
        itemName = Utf8ToWide(t->name) + L"（功法）";

        // 通知任务系统
        QuestSystem::Instance().UpdateProgress(QuestTargetType::LearnTechnique, item.itemId);

        m_msgText.setString(L"习得功法【" + Utf8ToWide(t->name) + L"】！");
        m_msgText.setFillColor(isFree ? sf::Color(150, 220, 255) : sf::Color(100, 230, 150));

    } else if (item.itemType == ShopItemType::Equipment) {
        // ===== 购买装备 =====
        const auto* eq = EquipmentSystem::Instance().GetEquipment(item.itemId);
        if (!eq) {
            m_msgText.setString(L"装备数据错误！");
            m_msgText.setFillColor(sf::Color(255, 100, 100));
            m_msgTimer = 2.f;
            return;
        }

        // 装备到对应槽位（自动替换旧装备，旧装备回到背包...简化版直接替换）
        std::string oldEquipId = EquipmentSystem::Instance().Equip(item.itemId);
        itemName = Utf8ToWide(eq->name) + L"（" + eq->GetSlotName() + L"）";

        m_msgText.setString(L"装备了【" + Utf8ToWide(eq->name) + L"】"
            + (oldEquipId.empty() ? L"" : L"，替换了旧装备"));
        m_msgText.setFillColor(sf::Color(100, 230, 150));

    } else {
        // ===== 购买普通物品（原有逻辑）=====
        // 检查背包空间
        auto& inv = GameSession::Instance().GetInventory();
        if ((int)inv.GetItems().size() >= InventorySystem::MAX_SLOTS) {
            m_msgText.setString(L"背包已满！");
            m_msgText.setFillColor(sf::Color(255, 120, 100));
            m_msgTimer = 2.f;
            return;
        }

        // 执行购买
        inv.AddItem(item.itemId, 1);
        const auto* data = ConfigManager::Instance().GetItem(item.itemId);
        itemName = data ? Utf8ToWide(data->name) : Utf8ToWide(item.itemId);

        m_msgText.setString(L"购买了 " + itemName + L"！"
            + (isFree ? L"" : L"花费 " + std::to_wstring(price) + L" 灵石"));
        m_msgText.setFillColor(isFree ? sf::Color(150, 220, 255) : sf::Color(100, 230, 150));
    }

    // 扣除灵石（非免费商品）
    if (!isFree) {
        player->SpendGold(price);
    }

    // 减少库存
    if (item.stock > 0) item.stock--;

    m_msgTimer = 2.5f;
}

void ShopState::SellItem(int idx) {
    // 简化版出售功能
    auto& inv = GameSession::Instance().GetInventory();
    const auto& items = inv.GetItems();
    if (idx < 0 || idx >= (int)items.size()) return;

    const auto& invItem = items[idx];
    const auto* data = ConfigManager::Instance().GetItem(invItem.itemId);
    if (!data) return;

    int sellPrice = data->value / 2;  // 半价回收
    if (sellPrice <= 0) sellPrice = 1;

    inv.RemoveItem(invItem.itemId, 1);
    GameSession::Instance().GetPlayer().AddGold(sellPrice);

    std::wstring itemName = Utf8ToWide(data->name);
    m_msgText.setString(L"出售了 " + itemName + L"，获得 " + std::to_wstring(sellPrice) + L" 灵石");
    m_msgText.setFillColor(sf::Color(200, 200, 150));
    m_msgTimer = 2.f;

    if (idx >= (int)inv.GetItems().size())
        m_selectedItem = std::max(0, (int)inv.GetItems().size() - 1);
}

// ============================================================
//  渲染
// ============================================================
void ShopState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(18, 20, 30));

    // 标题栏背景
    sf::RectangleShape titleBg(sf::Vector2f(760.f, 48.f));
    titleBg.setFillColor(sf::Color(35, 28, 45, 250));
    titleBg.setPosition(20.f, 12.f);
    titleBg.setOutlineThickness(1.f);
    titleBg.setOutlineColor(sf::Color(90, 70, 120));
    window.draw(titleBg);

    // 商店名
    m_titleText.setString(L"◈ " + m_shopName + L" ◈");
    m_titleText.setFillColor(sf::Color(220, 180, 100));
    m_titleText.setPosition(280.f, 20.f);
    window.draw(m_titleText);

    // 灵石数
    auto* player = &GameSession::Instance().GetPlayer();
    m_goldText.setString(L"灵石: " + std::to_wstring(player->GetGold()));
    m_goldText.setPosition(600.f, 24.f);
    window.draw(m_goldText);

    // ===== 左侧：物品列表面板 =====
    sf::RectangleShape listPanel(sf::Vector2f(380.f, 290.f));
    listPanel.setFillColor(sf::Color(28, 33, 48, 240));
    listPanel.setPosition(20.f, 75.f);
    listPanel.setOutlineThickness(1.f);
    listPanel.setOutlineColor(sf::Color(55, 70, 90));
    window.draw(listPanel);

    if (!m_inSellMode) {
        // 购买模式标题
        sf::Text buyTitle(L"— 购买 —", m_font, 16);
        buyTitle.setFillColor(sf::Color(130, 200, 150));
        buyTitle.setPosition(185.f, 80.f);
        window.draw(buyTitle);

        int maxShow = 8;
        int start = std::max(0, m_selectedItem - maxShow + 1);

        for (int i = 0; i < maxShow && (start + i) < (int)m_shopItems.size(); ++i) {
            int idx = start + i;
            const auto& si = m_shopItems[idx];
            bool sel = (idx == m_selectedItem);
            const auto* data = ConfigManager::Instance().GetItem(si.itemId);

            float by = 100.f + i * 31.f;
            sf::Color bgCol = sel ? sf::Color(45, 55, 35) : sf::Color(28, 33, 48);
            if (sel) {
                sf::RectangleShape hl(sf::Vector2f(370.f, 29.f));
                hl.setFillColor(bgCol);
                hl.setPosition(22.f, by - 2.f);
                window.draw(hl);
            }

            if (data) {
                std::wstring nameW = Utf8ToWide(data->name);
                std::wstring priceStr = (si.stock > 0)
                    ? (std::to_wstring(si.price) + L"灵")
                    : L"售罄";

                std::wstring prefix = sel ? L"▶ " : L"  ";
                m_shopItemText[i].setString(prefix + nameW
                    + std::wstring(16 - nameW.length(), L' ')
                    + L"  " + priceStr);
                m_shopItemText[i].setFillColor(sel ? sf::Color(255, 220, 80)
                    : (si.stock > 0 ? sf::Color(190, 200, 200) : sf::Color(110, 110, 110)));
            } else if (si.itemType == ShopItemType::Technique) {
                // 功法显示
                const auto* t = ConfigManager::Instance().GetTechnique(si.itemId);
                std::wstring nameW = t ? Utf8ToWide(t->name) : Utf8ToWide(si.itemId);
                std::wstring priceStr = (si.price <= 0) ? L"免费"
                    : (si.stock > 0 ? std::to_wstring(si.price) + L"灵" : L"售罄");
                bool learned = GameSession::Instance().GetPlayer().GetLearned(si.itemId);

                std::wstring prefix = sel ? L"▶ " : L"  ";
                m_shopItemText[i].setString(prefix + L"[功]" + nameW
                    + (learned ? L"(已学)" : L"")
                    + std::wstring(std::max(0, (int)(14 - nameW.length() - (learned?6:0))), L' ')
                    + L" " + priceStr);
                m_shopItemText[i].setFillColor(sel ? sf::Color(150, 220, 255)
                    : (learned ? sf::Color(140, 140, 140) : sf::Color(170, 210, 230)));
            } else if (si.itemType == ShopItemType::Equipment) {
                // 装备显示
                const auto* eq = EquipmentSystem::Instance().GetEquipment(si.itemId);
                std::wstring nameW = eq ? Utf8ToWide(eq->name) : Utf8ToWide(si.itemId);

                std::wstring prefix = sel ? L"▶ " : L"  ";
                m_shopItemText[i].setString(prefix + L"[装]" + nameW
                    + std::wstring(std::max(0, (int)(13 - nameW.length())), L' ')
                    + L" " + std::to_wstring(si.price) + L"灵");

                if (eq) m_shopItemText[i].setFillColor(sel ? eq->GetRarityColor()
                    : sf::Color(
                        (unsigned char)(eq->GetRarityColor().r * 0.7),
                        (unsigned char)(eq->GetRarityColor().g * 0.7),
                        (unsigned char)(eq->GetRarityColor().b * 0.7)));
                else m_shopItemText[i].setFillColor(sel ? sf::Color(255, 220, 80) : sf::Color(190, 190, 190));
            } else {
                m_shopItemText[i].setString(sel ? L"▶ ???" : L"  ???");
                m_shopItemText[i].setFillColor(sel ? sf::Color(255, 220, 80) : sf::Color(140, 140, 140));
            }
            window.draw(m_shopItemText[i]);
        }
    } else {
        // 出售模式标题
        sf::Text sellTitle(L"— 出售 —", m_font, 16);
        sellTitle.setFillColor(sf::Color(200, 150, 130));
        sellTitle.setPosition(185.f, 80.f);
        window.draw(sellTitle);

        const auto& invItems = GameSession::Instance().GetInventory().GetItems();
        int maxShow = 6;
        int start = std::max(0, m_selectedItem - maxShow + 1);

        for (int i = 0; i < maxShow && (start + i) < (int)invItems.size(); ++i) {
            int idx = start + i;
            const auto& iv = invItems[idx];
            bool sel = (idx == m_selectedItem);
            const auto* data = ConfigManager::Instance().GetItem(iv.itemId);

            float by = 100.f + i * 27.f;
            if (sel) {
                sf::RectangleShape hl(sf::Vector2f(370.f, 26.f));
                hl.setFillColor(sf::Color(55, 40, 30));
                hl.setPosition(22.f, by - 1.f);
                window.draw(hl);
            }

            if (data) {
                std::wstring nameW = Utf8ToWide(data->name);
                int sellPrice = std::max(1, data->value / 2);
                std::wstring prefix = sel ? L"▶ " : L"  ";
                m_invItemText[i].setString(prefix + nameW
                    + std::wstring(16 - nameW.length(), L' ')
                    + L" x" + std::to_wstring(iv.count)
                    + L"  售" + std::to_wstring(sellPrice) + L"灵");
                m_invItemText[i].setFillColor(sel ? sf::Color(255, 220, 80) : sf::Color(190, 180, 170));
            } else {
                m_invItemText[i].setString(sel ? L"▶ ???" : L"  ???");
                m_invItemText[i].setFillColor(sel ? sf::Color(255, 220, 80) : sf::Color(140, 140, 140));
            }
            window.draw(m_invItemText[i]);
        }
    }

    // ===== 右侧：物品详情面板 =====
    sf::RectangleShape detailPanel(sf::Vector2f(360.f, 290.f));
    detailPanel.setFillColor(sf::Color(28, 33, 48, 235));
    detailPanel.setPosition(418.f, 75.f);
    detailPanel.setOutlineThickness(1.f);
    detailPanel.setOutlineColor(sf::Color(55, 70, 90));
    window.draw(detailPanel);

    sf::Text detailTitle(L"— 物品详情 —", m_font, 17);
    detailTitle.setFillColor(sf::Color(170, 145, 95));
    detailTitle.setPosition(540.f, 82.f);
    window.draw(detailTitle);

    // 显示选中物品详情
    if (!m_inSellMode && m_selectedItem < (int)m_shopItems.size()) {
        const auto& si = m_shopItems[m_selectedItem];

        if (si.itemType == ShopItemType::Technique) {
            // ===== 功法详情显示 =====
            const auto* t = ConfigManager::Instance().GetTechnique(si.itemId);
            if (t) {
                std::wstring nameW = Utf8ToWide(t->name);
                sf::Text n(nameW, m_font, 22);
                n.setFillColor(sf::Color(150, 220, 255));
                n.setPosition(435.f, 108.f);
                window.draw(n);

                // 类型 + 属性
                sf::Text typeLine(L"类型: " + Utf8ToWide(t->type)
                    + L"  |  属性: " + Utf8ToWide(t->element), m_font, 14);
                typeLine.setFillColor(sf::Color(170, 170, 190));
                typeLine.setPosition(435.f, 138.f);
                window.draw(typeLine);

                // 描述
                sf::Text desc(Utf8ToWide(t->description), m_font, 13);
                desc.setFillColor(sf::Color(160, 160, 155));
                desc.setPosition(435.f, 158.f);
                window.draw(desc);

                // 等级上限
                sf::Text lvInfo(L"最高等级: " + std::to_wstring(t->maxLevel), m_font, 14);
                lvInfo.setFillColor(sf::Color(200, 180, 130));
                lvInfo.setPosition(435.f, 180.f);
                window.draw(lvInfo);

                // 加成信息
                std::wstring effStr;
                if (t->hpPerLevel) effStr += L"HP+" + std::to_wstring(t->hpPerLevel) + L"/Lv ";
                if (t->mpPerLevel) effStr += L"MP+" + std::to_wstring(t->mpPerLevel) + L"/Lv ";
                if (t->atkPerLevel) effStr += L"攻+" + std::to_wstring(t->atkPerLevel) + L"/Lv ";
                if (t->defPerLevel) effStr += L"防+" + std::to_wstring(t->defPerLevel) + L"/Lv ";
                if (!effStr.empty()) {
                    sf::Text eff(effStr, m_font, 13);
                    eff.setFillColor(sf::Color(150, 200, 150));
                    eff.setPosition(435.f, 202.f);
                    window.draw(eff);
                }

                // 价格
                sf::Text priceTxt;
                if (si.price <= 0) {
                    priceTxt.setString(L"★ 免费赠送 ★");
                    priceTxt.setFillColor(sf::Color(100, 200, 255));
                } else {
                    priceTxt.setString(L"售价: " + std::to_wstring(si.price) + L" 灵石");
                    priceTxt.setFillColor(sf::Color(255, 200, 80));
                }
                priceTxt.setFont(m_font); priceTxt.setCharacterSize(15);
                priceTxt.setPosition(435.f, 228.f);
                window.draw(priceTxt);

                // 检查是否已学会
                bool learned = GameSession::Instance().GetPlayer().GetLearned(si.itemId);
                if (learned) {
                    sf::Text learnedTxt(L"⚠ 已学会此功法", m_font, 14);
                    learnedTxt.setFillColor(sf::Color(255, 150, 80));
                    learnedTxt.setPosition(435.f, 250.f);
                    window.draw(learnedTxt);
                }
            }
        } else if (si.itemType == ShopItemType::Equipment) {
            // ===== 装备详情显示 =====
            const auto* eq = EquipmentSystem::Instance().GetEquipment(si.itemId);
            if (eq) {
                std::wstring nameW = Utf8ToWide(eq->name);

                // 名称（带品质颜色）
                sf::Text n(nameW, m_font, 22);
                n.setFillColor(eq->GetRarityColor());
                n.setPosition(435.f, 108.f);
                window.draw(n);

                // 品质 + 部位
                sf::Text infoLine(eq->GetRarityName() + L"  |  " + eq->GetSlotName(), m_font, 14);
                infoLine.setFillColor(eq->GetRarityColor());
                infoLine.setPosition(435.f, 138.f);
                window.draw(infoLine);

                // 描述
                sf::Text desc(Utf8ToWide(eq->description), m_font, 13);
                desc.setFillColor(sf::Color(160, 160, 155));
                desc.setPosition(435.f, 158.f);
                window.draw(desc);

                // 属性加成
                int ly = 180.f;
                if (eq->atkBonus > 0) { sf::Text t(L"攻击力 +" + std::to_wstring(eq->atkBonus), m_font, 13); t.setFillColor(sf::Color(255,120,120)); t.setPosition(435.f, ly); window.draw(t); ly+=18; }
                if (eq->defBonus > 0) { sf::Text t(L"防御力 +" + std::to_wstring(eq->defBonus), m_font, 13); t.setFillColor(sf::Color(120,120,255)); t.setPosition(435.f, ly); window.draw(t); ly+=18; }
                if (eq->hpBonus > 0)  { sf::Text t(L"生命上限 +" + std::to_wstring(eq->hpBonus), m_font, 13); t.setFillColor(sf::Color(120,255,120)); t.setPosition(435.f, ly); window.draw(t); ly+=18; }
                if (eq->mpBonus > 0)  { sf::Text t(L"法力上限 +" + std::to_wstring(eq->mpBonus), m_font, 13); t.setFillColor(sf::Color(120,180,255)); t.setPosition(435.f, ly); window.draw(t); ly+=18; }
                if (eq->spdBonus > 0){ sf::Text t(L"速度 +" + std::to_wstring(eq->spdBonus), m_font, 13); t.setFillColor(sf::Color(200,200,100)); t.setPosition(435.f,ly); window.draw(t); ly+=18; }

                // 特殊效果
                if (eq->vampiric || eq->counterAtk || eq->dodgeBonus || eq->critBonus) {
                    sf::Text specialTitle(L"— 特效 —", m_font, 13);
                    specialTitle.setFillColor(sf::Color(255, 200, 100));
                    specialTitle.setPosition(435.f, ly); window.draw(specialTitle); ly += 17;

                    if (eq->vampiric) {
                        sf::Text t(L"  吸血: 攻击回复 " + std::to_wstring((int)(eq->vampiricRate*100)) + L"% 伤害", m_font, 12);
                        t.setFillColor(sf::Color(255,80,120)); t.setPosition(435.f, ly); window.draw(t); ly+=16;
                    }
                    if (eq->dodgeBonus) {
                        sf::Text t(L"  闪避率 +" + std::to_wstring((int)(eq->dodgeRate*100)) + L"%", m_font, 12);
                        t.setFillColor(sf::Color(100,200,255)); t.setPosition(435.f, ly); window.draw(t); ly+=16;
                    }
                    if (eq->critBonus) {
                        sf::Text t(L"  暴击率 +" + std::to_wstring((int)(eq->critRate*100)) + L"% 伤害×" + std::to_wstring(eq->critDmg), m_font, 12);
                        t.setFillColor(sf::Color(255,215,0)); t.setPosition(435.f, ly); window.draw(t); ly+=16;
                    }
                    if (eq->counterAtk) {
                        sf::Text t(L"  反弹: 受到攻击时反弹 " + std::to_wstring((int)(eq->counterRate*100)) + L"%", m_font, 12);
                        t.setFillColor(sf::Color(255,100,100)); t.setPosition(435.f, ly); window.draw(t); ly+=16;
                    }
                }

                // 价格
                float py = (ly > 220.f) ? ly + 8.f : 220.f;
                sf::Text priceTxt(L"售价: " + std::to_wstring(si.price) + L" 灵石", m_font, 15);
                priceTxt.setFillColor(sf::Color(255, 200, 80));
                priceTxt.setPosition(435.f, py);
                window.draw(priceTxt);
            }
        } else {
            // ===== 原有普通物品详情显示（不变）=====
            const auto* data = ConfigManager::Instance().GetItem(si.itemId);
            if (data) {
                std::wstring nameW = Utf8ToWide(data->name);
                sf::Text n(nameW, m_font, 22);
                n.setFillColor(sf::Color(230, 210, 150));
                n.setPosition(435.f, 108.f);
                window.draw(n);

                sf::Text desc(Utf8ToWide(data->description), m_font, 13);
                desc.setFillColor(sf::Color(160, 160, 155));
                desc.setPosition(435.f, 138.f);
                window.draw(desc);

                sf::Text price(L"售价: " + std::to_wstring(si.price) + L" 灵石", m_font, 15);
                price.setFillColor(sf::Color(255, 200, 80));
                price.setPosition(435.f, 168.f);
                window.draw(price);

                if (data->effectType != EffectType::None) {
                    static auto effName = [](EffectType et) -> std::wstring {
                        switch (et) {
                            case EffectType::RestoreHp: return L"恢复生命";
                            case EffectType::RestoreMp: return L"恢复法力";
                            case EffectType::ExpBoost: return L"修炼经验加成";
                            case EffectType::BreakthroughAid: return L"突破辅助";
                            default: return L"特殊效果";
                        }
                    };
                    sf::Text eff(L"效果: " + effName(data->effectType)
                        + (data->effectValue > 0 ? L" +" + std::to_wstring(data->effectValue) : L""),
                        m_font, 14);
                    eff.setFillColor(sf::Color(100, 200, 150));
                    eff.setPosition(435.f, 196.f);
                    window.draw(eff);
                }

                sf::Text stock(L"库存: "
                    + (si.stock > 0 ? std::to_wstring(si.stock) + L" 件" : L"已售罄"),
                    m_font, 13);
                stock.setFillColor(si.stock > 0 ? sf::Color(150, 150, 150) : sf::Color(200, 100, 100));
                stock.setPosition(435.f, 222.f);
                window.draw(stock);
            }
        }
    } else if (m_inSellMode) {
        const auto& invItems = GameSession::Instance().GetInventory().GetItems();
        if (m_selectedItem < (int)invItems.size()) {
            const auto& iv = invItems[m_selectedItem];
            const auto* data = ConfigManager::Instance().GetItem(iv.itemId);
            if (data) {
                sf::Text n(Utf8ToWide(data->name), m_font, 22);
                n.setFillColor(sf::Color(230, 210, 150));
                n.setPosition(435.f, 108.f);
                window.draw(n);

                int sellPrice = std::max(1, data->value / 2);
                sf::Text sp(L"回收价: " + std::to_wstring(sellPrice) + L" 灵石", m_font, 15);
                sp.setFillColor(sf::Color(200, 180, 100));
                sp.setPosition(435.f, 145.f);
                window.draw(sp);

                sf::Text cnt(L"拥有数量: x" + std::to_wstring(iv.count), m_font, 14);
                cnt.setFillColor(sf::Color(160, 160, 160));
                cnt.setPosition(435.f, 175.f);
                window.draw(cnt);
            }
        }
    } else {
        sf::Text empty(L"选择一个商品查看详情", m_font, 15);
        empty.setFillColor(sf::Color(130, 130, 140));
        empty.setPosition(480.f, 150.f);
        window.draw(empty);
    }

    // 模式切换提示
    sf::Text modeTip;
    modeTip.setFont(m_font);
    modeTip.setCharacterSize(14);
    if (m_inSellMode) {
        modeTip.setString(L"◀ □ 出售模式    → 切换到购买");
        modeTip.setFillColor(sf::Color(200, 150, 130));
    } else {
        modeTip.setString(L"◀ □ 购买模式    → 切换到出售");
        modeTip.setFillColor(sf::Color(130, 200, 150));
    }
    modeTip.setPosition(420.f, 340.f);
    window.draw(modeTip);

    // 操作提示
    m_hintText.setString(L"↑↓ 选择   Enter/Z 购买/出售   ←→ 切换模式   ESC 返回");
    m_hintText.setPosition(180.f, 573.f);
    window.draw(m_hintText);

    // 消息
    if (m_msgTimer > 0.f) {
        sf::RectangleShape msgBox(sf::Vector2f(740.f, 34.f));
        msgBox.setFillColor(sf::Color(20, 25, 38, 230));
        msgBox.setPosition(30.f, 520.f);
        msgBox.setOutlineThickness(1.f);
        msgBox.setOutlineColor(sf::Color(80, 100, 80));
        window.draw(msgBox);

        m_msgText.setPosition(45.f, 528.f);
        window.draw(m_msgText);
    }
}
