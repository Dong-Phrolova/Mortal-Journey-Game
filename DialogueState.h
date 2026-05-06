#pragma once
#include "GameState.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// 对话选项
struct DialogueOption {
    std::wstring text;
    std::function<void()> onSelected;  // 选择后的回调
};

class DialogueState : public GameState {
public:
    DialogueState(const std::string& npcId);

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override;
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

private:
    void BuildDialogueTree();
    void ShowOptions();
    void AddLog(const std::wstring& msg);

    std::string m_npcId;
    const struct NPCData* m_npc = nullptr;

    // 对话状态
    int m_currentNode = 0;       // 当前对话节点索引
    int m_selectedOption = 0;
    bool m_showingOptions = false;
    bool m_finished = false;

    // 对话树（简单实现：每个节点 = 一句话 + 选项列表）
    struct DialogueNode {
        std::wstring speakerName;
        std::wstring text;           // NPC 说的话
        std::vector<DialogueOption> options; // 玩家可选择的回复
    };
    std::vector<DialogueNode> m_dialogueNodes;

    // UI
    sf::Font m_font;
    sf::Text m_namePlate;
    sf::Text m_dialogueText;
    sf::Text m_optionText[4];
    sf::Text m_hintText;
    float   m_textTimer = 0.f;

    // 日志/获得提示
    sf::Text m_logText;
    std::wstring m_lastLog;
    float   m_logTimer = 0.f;
};
