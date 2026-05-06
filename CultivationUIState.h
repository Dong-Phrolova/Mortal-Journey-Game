#pragma once
#include "GameState.h"
#include <SFML/Graphics.hpp>

class CultivationUIState : public GameState {
public:
    CultivationUIState();

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override;
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    void DoCultivate();          // 修炼选中功法（获得功法经验+属性加成）
    void TryBreakthrough();      // 尝试境界突破
    void TryUpgradeTechnique();   // 尝试升级选中功法（U键）
    std::wstring GetCombatSkillDesc(const std::string& techId, int level);
    std::wstring GetAttrName(const std::string& attr);   // "hp" → "生命"
    int         GetAttrBonus(const std::string& attr, const std::string& techId, int level);
    // 功法经验系统
    int         GetTechExp(const std::string& techId);   // 查询功法经验
    void        AddTechExp(const std::string& techId, int amt);  // 增加功法经验
    int         TechExpToLevelUp(const std::string& techId);     // 升级所需经验
    bool        CanUpgradeTech(const std::string& techId);       // 是否可以升级

    sf::Font m_font;

    // 境界显示
    sf::Text m_titleText;
    sf::Text m_realmName;
    sf::Text m_expBarLabel;
    sf::Text m_expText;           // 当前/需要 经验值

    // 属性面板
    sf::Text m_attrLabels[6];     // HP MP ATK DEF SPD SPI
    sf::Text m_attrValues[6];

    // 功法面板（右侧）
    sf::Text m_techTitle;
    sf::Text m_techItems[8];      // 已学功法列表
    sf::Text m_techDetailTitle;   // 选中功法详情标题
    sf::Text m_techDetailLines[8];// 选中功法的详细信息

    // 战斗技能预览
    sf::Text m_skillTitle;
    sf::Text m_skillItems[6];     // 对应的战斗技能 / 解锁信息
    int m_selectedTech = 0;       // 当前选中的功法索引

    // 操作按钮提示
    sf::Text m_actionHints[4];    // C修炼 U升级 B突破 ESC返回

    // 消息区域
    sf::Text m_messageText;
    float   m_msgTimer = 0.f;

    // 经验条动画
    float m_animTimer = 0.f;

    // 功法经验存储（内存中，不持久化到存档的简化版）
    struct TechExpEntry {
        std::string techId;
        int exp = 0;
    };
    std::vector<TechExpEntry> m_techExpList;
};
