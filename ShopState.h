#pragma once
#pragma execution_character_set("utf-8")
#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// 商店商品类型
enum class ShopItemType {
    Item,       // 普通物品（从 items.json）
    Technique,  // 功法（从 techniques.json）
    Equipment   // 装备（EquipmentSystem 内置数据）
};

// 商店中的单个商品条目
struct ShopItem {
    std::string itemId;           // 物品/功法/装备 ID
    ShopItemType itemType = ShopItemType::Item;  // 商品类型
    int price = 0;               // 售价
    int stock = 99;              // 库存（-1=无限）
};

class ShopState : public GameState {
public:
    // shopType 决定售卖的物品类别:
    //   "weapon"     — 武器商（武器+部分装备）
    //   "medicine"   — 丹药师（丹药+消耗品）
    //   "technique"  — 功法商人（售卖各种功法秘籍）★新增
    //   "equipment"  — 装备店（防具+饰品）★新增
    //   "misc"       — 杂货商
    //   "general"    — 综合商店
    ShopState(const std::wstring& shopName, const std::string& shopType);

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override {}
    void Render(sf::RenderWindow& window) override;
    void OnKeyPressed(sf::Keyboard::Key key);

private:
    void BuyItem(int idx);           // 购买选中物品（自动识别类型处理）
    void SellItem(int idx);          // 出售背包物品
    void InitShopItems();            // 根据商店类型初始化商品列表

    sf::Font m_font;

    std::wstring m_shopName;
    std::string m_shopType;

    // 商品列表
    std::vector<ShopItem> m_shopItems;
    int m_selectedItem = 0;
    int m_scrollOffset = 0;

    // 背包物品（出售用）
    int m_sellSelected = 0;
    bool m_inSellMode = false;  // false=购买模式, true=出售模式

    // UI 元素
    sf::Text m_titleText;
    sf::Text m_goldText;
    sf::Text m_shopItemText[8];      // 商品列表
    sf::Text m_invItemText[6];        // 背包列表（出售模式用）
    sf::Text m_detailText[4];         // 选中物品详情
    sf::Text m_msgText;
    float   m_msgTimer = 0.f;
    sf::Text m_hintText;

    // 动画
    float m_animTimer = 0.f;
};
