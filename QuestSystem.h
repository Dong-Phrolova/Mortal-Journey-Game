#pragma once
#pragma execution_character_set("utf-8")
#include <string>
#include <vector>
#include <functional>

// ============================================================
//  任务状态枚举
// ============================================================
enum class QuestStatus {
    Locked,       // 未解锁（前置任务未完成）
    Active,       // 已接取/进行中
    Completed,    // 已完成（可领奖励）
    Rewarded      // 已领奖（最终状态）
};

// ============================================================
//  任务目标类型
// ============================================================
enum class QuestTargetType {
    TalkToNPC,        // 与指定 NPC 对话
    DefeatEnemy,      // 击败指定敌人 N 次
    ReachLevel,       // 境界达到指定等级
    LearnTechnique,   // 学习指定功法
    UpgradeTechnique, // 功法升级到指定等级
    CollectItem,      // 收集/拥有指定物品
    ExploreLocation,  // 到达指定地点
    Custom            // 自定义条件（通过回调检测）
};

// ============================================================
//  单个任务目标
// ============================================================
struct QuestObjective {
    std::string description;           // 目标描述（如 "与墨大夫对话"）
    QuestTargetType type = QuestTargetType::TalkToNPC;
    std::string targetId;              // 目标 ID（NPC ID / 敌人 ID / 物品 ID 等）
    int requiredCount = 1;             // 需要的数量/次数
    int currentCount = 0;              // 当前进度
    bool completed = false;            // 是否已完成

    // 自定义检测回调（仅 Custom 类型使用）
    std::function<bool()> checkFunc;
};

// ============================================================
//  任务奖励
// ============================================================
struct QuestReward {
    int gold = 0;                      // 灵石奖励
    int exp = 0;                       // 经验奖励
    struct ItemReward {
        std::string itemId;
        int count = 1;
    };
    std::vector<ItemReward> items;     // 物品奖励
};

// ============================================================
//  任务数据结构
// ============================================================
struct QuestData {
    std::string id;                    // 任务唯一 ID
    std::string name;                  // 任务名称
    std::string description;           // 任务简介
    QuestStatus status = QuestStatus::Locked;

    std::vector<QuestObjective> objectives;  // 目标列表（全部完成=任务完成）
    QuestReward reward;               // 奖励

    std::vector<std::string> prerequisiteQuests;  // 前置任务ID列表

    // 接取任务时触发的旁白
    std::string onAcceptNarration;
    // 完成任务时触发的旁白
    std::string onCompleteNarration;
};

// ============================================================
//  任务系统管理器（单例）
// ============================================================
class QuestSystem {
public:
    static QuestSystem& Instance();

    // 初始化所有主线任务定义
    void Initialize();

    // 重置所有任务状态（用于新游戏）
    void Reset();

    // 获取所有任务（const引用，用于UI渲染）
    const std::vector<QuestData>& GetAllQuests() const { return m_quests; }

    // 获取进行中的任务
    std::vector<const QuestData*> GetActiveQuests() const;

    // 获取某个任务
    QuestData* GetQuest(const std::string& id);
    const QuestData* GetQuest(const std::string& id) const;

    // 尝试接取任务（自动检查前置条件）
    bool AcceptQuest(const std::string& questId);

    // 更新目标进度（由游戏事件调用）
    void UpdateProgress(QuestTargetType type, const std::string& targetId, int count = 1);

    // 检查位置条件（由地图系统调用）
    void CheckLocation(const std::string& mapId, int tileX, int tileY);

    // 检查等级条件（由修炼系统调用）
    void CheckLevel(const std::string& levelStr);

    // 检查道具条件（由背包系统调用）
    void CheckItemCollection(const std::string& itemId, int count);

    // 检查并完成任务
    void CheckCompletion(const std::string& questId);

    // 领取任务奖励（返回是否成功）
    bool ClaimReward(const std::string& questId);

    // 读档后恢复：自动接取所有满足前置条件的 Locked 任务
    void ResumeAfterLoad();

    // 检查是否有已完成的未领取奖励任务
    bool HasCompletedUnclaimed() const;

    // 最近完成的任务名（WorldMapState 用于通知）
    std::string m_lastCompletedQuestName;

    // 触发旁白（供外部调用显示剧情文本）
    struct NarrationEvent {
        std::string text;             // 旁白内容 key
        float duration = 4.f;          // 显示时长(秒)
        enum Type { Story, InnerThought, Hint } type = Type::Story;
        std::wstring wtext;          // 宽字符旁白文本（直接渲染用）
    };
    NarrationEvent* GetCurrentNarration();
    float GetNarrationTimer() const;
    void SetNarration(const std::string& text, NarrationEvent::Type type = NarrationEvent::Story, float dur = 4.f);
    void UpdateNarration(float dt);
    void ClearNarration();
    std::wstring GetCurrentNarrationText() const;

private:
    QuestSystem() = default;
    std::vector<QuestData> m_quests;

    NarrationEvent m_currentNarration;
    float m_narrationTimer = 0.f;
    std::wstring m_currentNarrationText;  // 旁白宽字符串缓存

    void DefineMainQuests();  // 定义所有主线任务

    bool m_initialized = false;  // 是否已初始化（防止重复初始化丢失进度）
};
