#pragma execution_character_set("utf-8")
#include "QuestSystem.h"
#include "GameSession.h"
#include "CultivationSystem.h"
#include <algorithm>

// ============================================================
//  单例
// ============================================================
QuestSystem& QuestSystem::Instance() {
    static QuestSystem inst;
    return inst;
}

// ============================================================
//  初始化：定义所有主线任务（仅一次）
// ============================================================
void QuestSystem::Initialize() {
    if (m_initialized) return;  // 防止重复初始化
    m_initialized = true;

    m_quests.clear();
    DefineMainQuests();

    // 第一个任务默认为 Active（新手引导）
    if (!m_quests.empty()) {
        m_quests[0].status = QuestStatus::Active;
    }
}

void QuestSystem::Reset() {
    m_initialized = false;
    m_quests.clear();
    m_lastCompletedQuestName.clear();
    m_narrationTimer = 0.f;
    m_narrationWaitingConfirm = false;
    m_currentNarrationText.clear();
    m_pendingLocationNarrations.clear();
    m_pendingBossDefeat = PendingBossDefeat();
}

void QuestSystem::DefineMainQuests() {
    // ===== 任务1: 初到青牛镇（原著第1-2章）=====
    {
        QuestData q;
        q.id = "quest_001_start";
        q.name = "初到青牛镇";
        q.description = "你跟随三叔来到了青牛镇。先去韩家小院找三叔，\n然后前往七玄门，再到神手谷拜师学艺，学会长春功。";
        q.status = QuestStatus::Locked;

        QuestObjective obj1;
        obj1.description = "与三叔对话了解七玄门";
        obj1.type = QuestTargetType::TalkToNPC;
        obj1.targetId = "san_shu";
        obj1.requiredCount = 1;
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "前往七玄门，与王护法对话";
        obj2.type = QuestTargetType::TalkToNPC;
        obj2.targetId = "wang_hufa";
        q.objectives.push_back(obj2);

        QuestObjective obj3;
        obj3.description = "在神手谷学习【长春功】";
        obj3.type = QuestTargetType::LearnTechnique;
        obj3.targetId = "changchun";
        q.objectives.push_back(obj3);

        q.reward.gold = 50;
        q.reward.exp = 30;
        QuestReward::ItemReward item_cloth, item_pill_1;
        item_cloth.itemId = "cloth_armor";
        item_cloth.count = 1;
        item_pill_1.itemId = "qi_breakthrough_pill";
        item_pill_1.count = 1;
        q.reward.items.push_back(item_cloth);
        q.reward.items.push_back(item_pill_1);
        q.onAcceptNarration = "story_arrive_qingniu";
        q.onCompleteNarration = "story_learn_technique";

        m_quests.push_back(q);
    }

    // ===== 任务2: 勤修苦练（需突破+升级功法）=====
    {
        QuestData q;
        q.id = "quest_002_cultivate";
        q.name = "勤修苦练";
        q.description = "学会长春功后，勤加修炼！先突破到练气二层（按C键修炼积累经验，\n然后在修炼界面按B键突破），再将长春功升级到Lv.2（消耗练气点数）。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_001_start");

        QuestObjective obj1;
        obj1.description = "突破到练气二层（需要按B键，首次突破无需丹药）";
        obj1.type = QuestTargetType::Custom;
        obj1.targetId = "breakthrough_success";
        obj1.requiredCount = 1;
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "将长春功升级到Lv.2（按U键消耗1点练气点数）";
        obj2.type = QuestTargetType::UpgradeTechnique;
        obj2.targetId = "changchun";
        obj2.requiredCount = 2;
        q.objectives.push_back(obj2);

        q.reward.gold = 100;
        q.reward.exp = 40;
        QuestReward::ItemReward item_pill_sm;
        item_pill_sm.itemId = "exp_pill_small";
        item_pill_sm.count = 3;
        q.reward.items.push_back(item_pill_sm);
        q.onCompleteNarration = "story_cultivate_progress";

        m_quests.push_back(q);
    }

    // ===== 任务3: 初试锋芒 =====
    {
        QuestData q;
        q.id = "quest_003_first_combat";
        q.name = "初试锋芒";
        q.description = "修炼小成，该去后山历练一番了！击败3个敌人，检验自己的实力。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_002_cultivate");

        QuestObjective obj1;
        obj1.description = "击败 3 个敌人";
        obj1.type = QuestTargetType::DefeatEnemy;
        obj1.targetId = "any";
        obj1.requiredCount = 3;
        q.objectives.push_back(obj1);

        q.onCompleteNarration = "story_first_victory";

        m_quests.push_back(q);
    }

    // ===== 任务4: 坊市见闻 =====
    {
        QuestData q;
        q.id = "quest_004_market";
        q.name = "坊市见闻";
        q.description = "前往【嘉州城·市场】，与武器商和丹药师对话，了解坊市行情。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_003_first_combat");

        QuestObjective obj1;
        obj1.description = "与武器商对话";
        obj1.type = QuestTargetType::TalkToNPC;
        obj1.targetId = "wuqishang";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "与丹药师对话";
        obj2.type = QuestTargetType::TalkToNPC;
        obj2.targetId = "danyaoshi";
        q.objectives.push_back(obj2);

        q.reward.gold = 60;
        q.reward.exp = 25;
        // 旁白改为到达坊市时触发（不在接取时触发）
        // q.onAcceptNarration = "story_visit_market";

        m_quests.push_back(q);
    }

    // ===== 任务5: 突破练气 =====
    {
        QuestData q;
        q.id = "quest_005_breakthrough";
        q.name = "突破瓶颈";
        q.description = "修炼到了瓶颈！你需要一枚【练气丹】来冲破穴位。\n丹药师那有售卖，买一颗来试试吧。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_004_market");

        QuestObjective obj1;
        obj1.description = "境界突破成功一次（按 B 键）";
        obj1.type = QuestTargetType::Custom;
        obj1.targetId = "breakthrough_success";
        obj1.requiredCount = 1;
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "长春功达到 Lv.5（解锁新技能）";
        obj2.type = QuestTargetType::UpgradeTechnique;
        obj2.targetId = "changchun";
        obj2.requiredCount = 5;
        q.objectives.push_back(obj2);

        q.reward.gold = 200;
        q.reward.exp = 80;
        QuestReward::ItemReward item1, item2, item3;
        item1.itemId = "spirit_gathering_pill"; item1.count = 2;
        item2.itemId = "qi_breakthrough_pill"; item2.count = 1;
        item3.itemId = "exp_pill_small";       item3.count = 3;
        q.reward.items.push_back(item1);
        q.reward.items.push_back(item2);
        q.reward.items.push_back(item3);
        q.onCompleteNarration = "story_breakthrough";

        m_quests.push_back(q);
    }

    // ===== 任务6: 神秘小瓶（原著第10章）=====
    {
        QuestData q;
        q.id = "quest_006_bottle";
        q.name = "神秘小瓶";
        q.description = "修炼之余外出走走。去【七玄门外门】后面的树林（后山）转转，\n据说最近有人在那里捡到古怪的东西。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_005_breakthrough");

        QuestObjective obj1;
        obj1.description = "前往七玄门后山探索";
        obj1.type = QuestTargetType::ExploreLocation;
        obj1.targetId = "qixuanmen_back";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "在后山获得【神秘小瓶】";
        obj2.type = QuestTargetType::CollectItem;
        obj2.targetId = "mystic_bottle";
        obj2.requiredCount = 1;
        q.objectives.push_back(obj2);

        q.reward.gold = 100;
        q.reward.exp = 60;
        q.onCompleteNarration = "story_found_bottle";
        // 不在接取时触发旁白，改为到达后山+走到小瓶附近时触发
        // q.onAcceptNarration = "story_hint_bottle";

        m_quests.push_back(q);
    }

    // ===== 任务7: 墨大夫的异样（原著第7-9章）=====
    {
        QuestData q;
        q.id = "quest_007_mo_scheme";
        q.name = "墨大夫的异样";
        q.description = "墨大夫最近行为古怪，经常闭关不出。\n去【神手谷】找张铁聊聊，然后调查炼骨崖。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_006_bottle");

        QuestObjective obj1;
        obj1.description = "在神手谷与张铁对话";
        obj1.type = QuestTargetType::TalkToNPC;
        obj1.targetId = "zhang_tie";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "调查炼骨崖（从神手谷北门进入）";
        obj2.type = QuestTargetType::ExploreLocation;
        obj2.targetId = "liangu_cliff";
        q.objectives.push_back(obj2);

        q.reward.gold = 80;
        q.reward.exp = 50;
        QuestReward::ItemReward item_pill_7;
        item_pill_7.itemId = "spirit_gathering_pill";
        item_pill_7.count = 2;
        q.reward.items.push_back(item_pill_7);
        q.onCompleteNarration = "story_mo_scheme";

        m_quests.push_back(q);
    }

    // ===== 任务8: 野狼帮入侵（原著野狼帮相关）=====
    {
        QuestData q;
        q.id = "quest_008_wolf_attack";
        q.name = "野狼帮入侵";
        q.description = "野狼帮大举进攻七玄门！王护法正在南门召集弟子防守。\n前往【七玄门外门】南门找王护法，协助击退来犯之敌。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_007_mo_scheme");

        QuestObjective obj1;
        obj1.description = "与王护法对话，领取防守任务";
        obj1.type = QuestTargetType::TalkToNPC;
        obj1.targetId = "wang_hufa";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "击败 3 个野狼帮敌人（野外/后山遇敌）";
        obj2.type = QuestTargetType::DefeatEnemy;
        obj2.targetId = "any";
        obj2.requiredCount = 3;
        q.objectives.push_back(obj2);

        q.reward.gold = 150;
        q.reward.exp = 70;
        QuestReward::ItemReward item;
        item.itemId = "hp_potion_medium";
        item.count = 3;
        q.reward.items.push_back(item);
        q.onCompleteNarration = "story_wolf_defeated";

        m_quests.push_back(q);
    }

    // ===== 任务9: 墨大夫的真相（原著夺舍剧情）=====
    {
        QuestData q;
        q.id = "quest_009_mo_truth";
        q.name = "墨大夫的真相";
        q.description = "野狼帮入侵的幕后似乎有墨大夫的影子...\n墨大夫已不在神手谷，去【炼骨崖】找他当面对质！";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_008_wolf_attack");

        QuestObjective obj1;
        obj1.description = "前往炼骨崖与墨大夫对质（注意：这可能会触发战斗！）";
        obj1.type = QuestTargetType::TalkToNPC;
        obj1.targetId = "mo_dafu";
        q.objectives.push_back(obj1);

        q.reward.gold = 200;
        q.reward.exp = 100;
        QuestReward::ItemReward item_jade, item_pill_9;
        item_jade.itemId = "mystic_ice_jade";
        item_jade.count = 1;
        item_pill_9.itemId = "qi_breakthrough_pill";
        item_pill_9.count = 2;
        q.reward.items.push_back(item_jade);
        q.reward.items.push_back(item_pill_9);
        q.onCompleteNarration = "story_mo_final";

        m_quests.push_back(q);
    }

    // ===== 任务10: 新的开始·黄枫谷（原著加入黄枫谷）=====
    {
        QuestData q;
        q.id = "quest_010_new_start";
        q.name = "新的开始";
        q.description = "七玄门已非久留之地。从七玄门外门的东门离开，\n前往【黄枫谷】寻求新的修仙之路。";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_009_mo_truth");

        QuestObjective obj1;
        obj1.description = "通过东门前往黄枫谷";
        obj1.type = QuestTargetType::ExploreLocation;
        obj1.targetId = "huangfengu";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "击败2个黄枫谷弟子证明实力";
        obj2.type = QuestTargetType::DefeatEnemy;
        obj2.targetId = "qi_disciple";
        obj2.requiredCount = 2;
        q.objectives.push_back(obj2);

        q.reward.gold = 300;
        q.reward.exp = 120;
        QuestReward::ItemReward item1, item2;
        item1.itemId = "spirit_gathering_pill";
        item1.count = 5;
        item2.itemId = "green_gown";
        item2.count = 1;
        q.reward.items.push_back(item1);
        q.reward.items.push_back(item2);
        q.onCompleteNarration = "story_huangfengu_entry";

        m_quests.push_back(q);
    }

    // ===== 任务11: 血色禁地（原著经典副本）=====
    {
        QuestData q;
        q.id = "quest_011_xuese";
        q.name = "血色禁地";
        q.description = "黄枫谷的【血色禁地】即将开启！修炼到足够实力后进入其中，\n寻找珍贵的灵药和机缘。注意：禁地内危险重重！";
        q.status = QuestStatus::Locked;
        q.prerequisiteQuests.push_back("quest_010_new_start");

        QuestObjective obj1;
        obj1.description = "进入血色禁地（从七玄门外门西门进入）";
        obj1.type = QuestTargetType::ExploreLocation;
        obj1.targetId = "xuese";
        q.objectives.push_back(obj1);

        QuestObjective obj2;
        obj2.description = "击败禁地中的妖兽（3只）";
        obj2.type = QuestTargetType::DefeatEnemy;
        obj2.targetId = "any";
        obj2.requiredCount = 3;
        q.objectives.push_back(obj2);

        QuestObjective obj3;
        obj3.description = "长春功修炼到 Lv.7";
        obj3.type = QuestTargetType::UpgradeTechnique;
        obj3.targetId = "changchun";
        obj3.requiredCount = 7;
        q.objectives.push_back(obj3);

        q.reward.gold = 500;
        q.reward.exp = 200;
        QuestReward::ItemReward item;
        item.itemId = "spirit_gathering_pill";
        item.count = 3;
        q.reward.items.push_back(item);
        q.onCompleteNarration = "story_xuese_done";

        m_quests.push_back(q);
    }
}

// ============================================================
//  获取进行中的任务
// ============================================================
std::vector<const QuestData*> QuestSystem::GetActiveQuests() const {
    std::vector<const QuestData*> result;
    for (const auto& q : m_quests) {
        if (q.status == QuestStatus::Active || q.status == QuestStatus::Completed) {
            result.push_back(&q);
        }
    }
    return result;
}

QuestData* QuestSystem::GetQuest(const std::string& id) {
    for (auto& q : m_quests) {
        if (q.id == id) return &q;
    }
    return nullptr;
}

const QuestData* QuestSystem::GetQuest(const std::string& id) const {
    for (const auto& q : m_quests) {
        if (q.id == id) return &q;
    }
    return nullptr;
}

// ============================================================
//  接取任务
// ============================================================
bool QuestSystem::AcceptQuest(const std::string& questId) {
    auto* q = GetQuest(questId);
    if (!q || q->status != QuestStatus::Locked) return false;

    // 检查前置任务
    for (const auto& preId : q->prerequisiteQuests) {
        auto* pre = GetQuest(preId);
        if (!pre || pre->status != QuestStatus::Rewarded) return false;
    }

    q->status = QuestStatus::Active;

    // 检查已有条件是否已满足（接任务前已完成的事项）
    auto& player = GameSession::Instance().GetPlayer();
    auto& inv = GameSession::Instance().GetInventory();
    
    for (auto& obj : q->objectives) {
        if (obj.completed) continue;
        
        switch (obj.type) {
        case QuestTargetType::LearnTechnique:
            // 检查玩家是否已学会该功法
            for (const auto& lt : player.GetLearned()) {
                if (lt.techniqueId == obj.targetId && lt.level >= 1) {
                    obj.currentCount = obj.requiredCount;
                    obj.completed = true;
                }
            }
            break;
            
        case QuestTargetType::UpgradeTechnique:
            // 检查功法是否已达到指定等级
            for (const auto& lt : player.GetLearned()) {
                if (lt.techniqueId == obj.targetId) {
                    if (lt.level >= obj.requiredCount) {
                        obj.currentCount = obj.requiredCount;
                        obj.completed = true;
                    } else {
                        // 已学会但等级未达标：将当前等级作为进度起点
                        // 这样 requiredCount 就是目标等级，每次升级加1
                        obj.currentCount = lt.level;
                    }
                }
            }
            break;
            
        case QuestTargetType::CollectItem:
            // 检查是否已拥有指定物品
            if (inv.HasItem(obj.targetId, obj.requiredCount)) {
                obj.currentCount = obj.requiredCount;
                obj.completed = true;
            }
            break;
            
        case QuestTargetType::ReachLevel:
            // 检查是否已达到指定等级
            {
                std::string curLevel = CultivationSystem::GetLevelString(player.GetCulti());
                if (curLevel == obj.targetId) {
                    obj.currentCount = obj.requiredCount;
                    obj.completed = true;
                }
            }
            break;
            
        case QuestTargetType::Custom:
            if (obj.checkFunc && obj.checkFunc()) {
                obj.currentCount = obj.requiredCount;
                obj.completed = true;
            }
            break;
            
        default:
            break;
        }
    }

    // 检查所有目标是否已达成
    CheckCompletion(q->id);

    if (!q->onAcceptNarration.empty()) {
        SetNarration(q->onAcceptNarration, NarrationEvent::Type::Story, 4.f);
    }

    // 特定任务：条件旁白（到达指定地点才触发）
    if (q->id == "quest_004_market") {
        AddLocationNarration("jiazhou_market", "story_visit_market");
    }
    // 任务6：到达后山时触发提示旁白
    if (q->id == "quest_006_bottle") {
        AddLocationNarration("qixuanmen_back", "story_hint_bottle");
    }
    // 任务9：注册墨大夫Boss击败事件
    if (q->id == "quest_009_mo_truth") {
        AddPendingBossDefeat("mo_dafu_boss",
            L"（墨大夫倒在地上，面容扭曲）\n不可能...我筹划了数十年...\n怎么会败在你这个黄毛小子手上！\n\n（墨大夫的身形逐渐变得透明，灵力四散消逸...）",
            "story_mo_defeated");
    }

    return true;
}

// ============================================================
//  更新目标进度
// ============================================================
void QuestSystem::UpdateProgress(QuestTargetType type, const std::string& targetId, int count) {
    for (auto& q : m_quests) {
        if (q.status != QuestStatus::Active) continue;

        for (auto& obj : q.objectives) {
            if (obj.completed) continue;
            if (obj.type != type) continue;

            // 根据类型处理不同的逻辑
            switch (type) {
            case QuestTargetType::DefeatEnemy:
                if (obj.targetId == "any" || obj.targetId == targetId) {
                    obj.currentCount += count;
                }
                break;

            case QuestTargetType::TalkToNPC:
            case QuestTargetType::LearnTechnique:
            case QuestTargetType::UpgradeTechnique:
            case QuestTargetType::CollectItem:
            case QuestTargetType::ExploreLocation:
                // 这些类型直接匹配 targetId
                if (obj.targetId == targetId) {
                    obj.currentCount += count;
                }
                break;

            case QuestTargetType::ReachLevel:
                // targetId 格式："Qi_3" 表示练气期第3层
                // 这里需要外部传入玩家当前等级信息
                if (obj.targetId == targetId) {
                    obj.currentCount = count;  // count 这里用作"是否达到"
                }
                break;

            case QuestTargetType::Custom:
                // 如果存在 checkFunc，用它作为额外条件
                // 如果 checkFunc 为空，直接按 targetId 匹配（如 "cultivate_3"）
                if (obj.targetId == targetId) {
                    if (!obj.checkFunc || obj.checkFunc()) {
                        obj.currentCount += count;
                    }
                }
                break;
            }

            // 检查是否完成
            if (obj.currentCount >= obj.requiredCount) {
                obj.currentCount = obj.requiredCount;
                obj.completed = true;
            }
        }

        CheckCompletion(q.id);
    }
}

// ============================================================
//  检查位置条件（由地图系统调用）
// ============================================================
void QuestSystem::CheckLocation(const std::string& mapId, int tileX, int tileY) {
    for (auto& q : m_quests) {
        if (q.status != QuestStatus::Active) continue;

        for (auto& obj : q.objectives) {
            if (obj.completed) continue;
            if (obj.type != QuestTargetType::ExploreLocation) continue;

            // 检查是否到达指定地点
            // targetId 格式："mapId:x:y" 或只是 "mapId"
            if (obj.targetId == mapId ||
                obj.targetId.find(mapId + ":") == 0) {
                obj.currentCount = 1;
                obj.completed = true;
            }
        }

        CheckCompletion(q.id);
    }

    // 检查条件旁白：到达指定地图时触发
    for (auto& pn : m_pendingLocationNarrations) {
        if (!pn.triggered && pn.mapId == mapId) {
            pn.triggered = true;
            SetNarration(pn.narrationKey, NarrationEvent::Type::Story, 4.f);
        }
    }
}

void QuestSystem::AddLocationNarration(const std::string& mapId, const std::string& narrationKey) {
    // 避免重复
    for (const auto& pn : m_pendingLocationNarrations) {
        if (pn.mapId == mapId && pn.narrationKey == narrationKey) return;
    }
    PendingLocationNarration pn;
    pn.mapId = mapId;
    pn.narrationKey = narrationKey;
    pn.triggered = false;
    m_pendingLocationNarrations.push_back(pn);
}

void QuestSystem::ClearLocationNarrations() {
    m_pendingLocationNarrations.clear();
}

void QuestSystem::AddPendingBossDefeat(const std::string& bossId, const std::wstring& dialogue, const std::string& narrationKey) {
    m_pendingBossDefeat.bossId = bossId;
    m_pendingBossDefeat.defeatDialogue = dialogue;
    m_pendingBossDefeat.narrationKey = narrationKey;
    m_pendingBossDefeat.triggered = false;
}

bool QuestSystem::CheckBossDefeat(const std::string& enemyId) {
    if (m_pendingBossDefeat.triggered) return false;
    if (m_pendingBossDefeat.bossId == enemyId) {
        m_pendingBossDefeat.triggered = true;
        return true;
    }
    return false;
}

const QuestSystem::PendingBossDefeat* QuestSystem::GetPendingBossDefeat() const {
    if (m_pendingBossDefeat.triggered && !m_pendingBossDefeat.bossId.empty())
        return &m_pendingBossDefeat;
    return nullptr;
}

void QuestSystem::ClearPendingBossDefeat() {
    m_pendingBossDefeat = PendingBossDefeat();
}

// ============================================================
//  检查等级条件（由修炼系统调用）
// ============================================================
void QuestSystem::CheckLevel(const std::string& levelStr) {
    for (auto& q : m_quests) {
        if (q.status != QuestStatus::Active) continue;

        for (auto& obj : q.objectives) {
            if (obj.completed) continue;
            if (obj.type != QuestTargetType::ReachLevel) continue;

            if (obj.targetId == levelStr) {
                obj.currentCount = 1;
                obj.completed = true;
            }
        }

        CheckCompletion(q.id);
    }
}

// ============================================================
//  检查道具条件（由背包系统调用）
// ============================================================
void QuestSystem::CheckItemCollection(const std::string& itemId, int count) {
    for (auto& q : m_quests) {
        if (q.status != QuestStatus::Active) continue;

        for (auto& obj : q.objectives) {
            if (obj.completed) continue;
            if (obj.type != QuestTargetType::CollectItem) continue;

            if (obj.targetId == itemId) {
                obj.currentCount = count;  // count 应该是当前拥有的数量
                if (obj.currentCount >= obj.requiredCount) {
                    obj.currentCount = obj.requiredCount;
                    obj.completed = true;
                }
            }
        }

        CheckCompletion(q.id);
    }
}

// ============================================================
//  检查任务完成状态
// ============================================================
void QuestSystem::CheckCompletion(const std::string& questId) {
    auto* q = GetQuest(questId);
    if (!q || q->status != QuestStatus::Active) return;

    bool allDone = true;
    for (const auto& obj : q->objectives) {
        if (!obj.completed) { allDone = false; break; }
    }

    if (allDone) {
        q->status = QuestStatus::Completed;
        m_lastCompletedQuestName = q->name;
        if (!q->onCompleteNarration.empty()) {
            SetNarration(q->onCompleteNarration, NarrationEvent::Type::Story, 5.f);
        }
    }
}

// ============================================================
//  领取奖励
// ============================================================
bool QuestSystem::ClaimReward(const std::string& questId) {
    auto* q = GetQuest(questId);
    if (!q || q->status != QuestStatus::Completed) return false;

    if (q->reward.gold > 0) {
        GameSession::Instance().GetPlayer().AddGold(q->reward.gold);
    }

    // 发放修炼经验
    if (q->reward.exp > 0) {
        GameSession::Instance().GetPlayer().GetCultiMutable().cultivationExp += q->reward.exp;
    }

    auto& inv = GameSession::Instance().GetInventory();
    for (const auto& item : q->reward.items) {
        inv.AddItem(item.itemId, item.count);
    }

    q->status = QuestStatus::Rewarded;

    // 尝试解锁后续任务
    for (auto& nextQ : m_quests) {
        if (nextQ.status == QuestStatus::Locked) {
            bool allPreDone = true;
            for (const auto& preId : nextQ.prerequisiteQuests) {
                auto* pre = GetQuest(preId);
                if (!pre || pre->status != QuestStatus::Rewarded) {
                    allPreDone = false; break;
                }
            }
            if (allPreDone && !nextQ.prerequisiteQuests.empty()) {
                AcceptQuest(nextQ.id);
            }
        }
    }

    return true;
}

// ============================================================
//  读档恢复：自动接取满足前置条件的 Locked 任务
// ============================================================
void QuestSystem::ResumeAfterLoad() {
    bool anyAccepted = false;
    for (auto& nextQ : m_quests) {
        if (nextQ.status != QuestStatus::Locked) continue;
        if (nextQ.prerequisiteQuests.empty()) continue;

        bool allPreDone = true;
        for (const auto& preId : nextQ.prerequisiteQuests) {
            auto* pre = GetQuest(preId);
            if (!pre || pre->status != QuestStatus::Rewarded) {
                allPreDone = false;
                break;
            }
        }
        if (allPreDone) {
            // 静默接取（不触发旁白）
            nextQ.status = QuestStatus::Active;
            anyAccepted = true;
        }
    }
}

bool QuestSystem::HasCompletedUnclaimed() const {
    for (const auto& q : m_quests) {
        if (q.status == QuestStatus::Completed) return true;
    }
    return false;
}

// ============================================================
//  旁白系统
// ============================================================

QuestSystem::NarrationEvent* QuestSystem::GetCurrentNarration() {
    if (m_narrationTimer > 0.f || m_narrationWaitingConfirm)
        return &m_currentNarration;
    return nullptr;
}

float QuestSystem::GetNarrationTimer() const {
    return m_narrationTimer;
}

bool QuestSystem::IsNarrationWaitingConfirm() const {
    return m_narrationWaitingConfirm;
}

void QuestSystem::ConfirmNarration() {
    if (m_narrationWaitingConfirm) {
        m_narrationWaitingConfirm = false;
        m_narrationTimer = 0.f;
        m_currentNarrationText.clear();
    }
}

void QuestSystem::SetNarration(const std::string& key, NarrationEvent::Type type, float dur) {
    static const std::map<std::string, std::wstring> NARRATIONS = {
        {"story_leave_home",
         L"（韩立站在村口，回头望了望渐行渐远的小山村。\n母亲正站在土墙边偷偷抹着眼泪，父亲吧嗒吧嗒抽着旱烟。）\n\n「等我挣了钱，就马上回来！」\n——年幼的韩立在心里暗暗发誓。"},
        {"story_arrive_qingniu",
         L"马车一路颠簸，三天后终于到了一个小镇——青牛镇。\n「这就是三叔说的那个镇子啊...」\n韩立好奇地打量着这个比村子大得多的镇子。"},
        {"story_wang_hufa_arrives",
         L"第三天傍晚，一辆漆黑油亮的马车停在了酒楼门口。\n车上插着一面绣着「玄」字的小三角黑旗。\n「七玄门的人来了！」三叔神色一紧，连忙迎了上去。"},
        {"story_start",
         L"「韩立啊，从今天起，你就是我墨大夫的弟子了。」\n——墨大夫的声音低沉而有力"},
        {"story_learn_technique",
         L"★ 功法【长春功】入手！\n\n（韩立内心：这口诀...怎么感觉体内有一股凉意在涌动？）"},
        {"story_cultivate_progress",
         L"（韩立内心：修炼果然不是一蹴而就的事...但每次都能感觉到灵气在经脉中流动。）\n\n★ 主线任务更新！"},
        {"story_first_victory",
         L"★ 首战告捷！\n\n「不错，虽然招式粗糙，但至少懂得如何保护自己了。」\n——仿佛有人在耳边低语"},
        {"story_visit_market",
         L"坊市之中人声鼎沸，各色摊位琳琅满目。\n（韩立内心：这些丹药和兵器...恐怕要攒好久的灵石才行。）"},
        {"story_breakthrough",
         L"★★★ 突破成功！★★★\n\n一股磅礴的力量涌遍全身，韩立感觉整个世界都变得清晰起来。\n「这便是...修仙者的世界吗？」"},
        {"story_found_bottle",
         L"★ 获得【神秘小瓶】！\n\n（韩立盯着手中这个沾满泥土的绿色小瓶，瓶身有墨绿色的叶状花纹。）\n「这东西...看起来有些年头了，沉甸甸的，不像是瓷的。」"},
        {"story_hint_bottle",
         L"（韩立沿着小路走去，脚下突然踢到一个硬物，\n疼得他蹲在地上好半天才缓过劲来...）\n\n★ 调查踢到的是什么东西"},
        {"story_mo_scheme",
         L"（韩立内心：墨大夫这些日子总是神神秘秘的...\n张铁修炼的那门象甲功也是霸道得很，总觉得哪里不对劲。）"},
        {"story_wolf_defeated",
         L"★ 击退野狼帮！\n\n王护法拍了拍你的肩膀：「好小子，没给七玄门丢脸！」\n但远处神手谷的方向，似乎隐隐传来异样的气息..."},
        {"story_mo_final",
         L"（韩立回想起墨大夫教自己修炼的每一个细节，背后不禁冒出一阵冷汗...\n「原来...他一直都在打我的主意？」）\n\n★ 七玄门已非久留之地！"},
        {"story_mo_defeated",
         L"（墨大夫的身影化作一缕黑烟，消散在炼骨崖的风中...）\n\n韩立站在崖边，久久无言。\n那些年跟着墨大夫学医的日子，原来都是一场精心设计的骗局。\n\n「从今天起，我要靠自己的力量走下去。」\n\n★ 墨大夫已被击败！七玄门的危机解除了。\n★ 主线任务已更新！"},
        {"story_huangfengu_entry",
         L"★ 加入黄枫谷！\n\n黄枫谷的山门巍峨耸立，云雾缭绕间隐约可见仙鹤飞舞。\n「这里...才是我真正应该待的地方。」"},
        {"story_xuese_done",
         L"★ 血色禁地试炼完成！\n\n从禁地中出来，韩立身上满是伤痕，但眼神却异常明亮。\n「这瓶里的灵药...加上小瓶的催熟效果，筑基有望了！」"},
        {"inner_mystery",
         L"（韩立内心：墨大夫教我的功法...总觉得哪里有些奇怪。\n他到底想让我做什么？）"},
        {"hint_new_quest",
         L"★ 新的主线任务已开启！按 Q 键查看任务列表"},
    };

    m_currentNarration.text = key;
    m_currentNarration.duration = dur;
    m_currentNarration.type = type;
    m_currentNarration.waitForConfirm = true;  // 默认等待玩家确认
    m_narrationTimer = dur;  // 保留时长用于渐入动画等
    m_narrationWaitingConfirm = true;

    // 也存储 wstring 到 text 字段（渲染时用）
    auto it = NARRATIONS.find(key);
    if (it != NARRATIONS.end()) {
        // 将 wstring 暂存到 m_currentNarration.text（实际渲染时通过 GetCurrentNarrationText 获取）
        m_currentNarrationText = it->second;
    } else {
        m_currentNarrationText = L"";
    }
}

void QuestSystem::UpdateNarration(float dt) {
    // 等待确认模式下不自动消失
    if (m_narrationWaitingConfirm) return;

    if (m_narrationTimer > 0.f) {
        m_narrationTimer -= dt;
        if (m_narrationTimer <= 0.f) {
            m_narrationTimer = 0.f;
        }
    }
}

void QuestSystem::ClearNarration() {
    m_narrationTimer = 0.f;
    m_narrationWaitingConfirm = false;
}

std::wstring QuestSystem::GetCurrentNarrationText() const {
    if (m_narrationTimer > 0.f || m_narrationWaitingConfirm)
        return m_currentNarrationText;
    return L"";
}
