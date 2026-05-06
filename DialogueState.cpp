#pragma execution_character_set("utf-8")
#include "DialogueState.h"
#include "GameState.h"
#include "ConfigManager.h"
#include "GameSession.h"
#include "QuestSystem.h"
#include <cmath>
#define NOMINMAX
#include <Windows.h>

// ============================================================
//  UTF-8 std::string → std::wstring 正确转换
// ============================================================
static std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 1) return L"";
    std::wstring ws(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// 判断玩家是否已学过指定功法
static bool HasLearned(const std::string& techId) {
    for (const auto& lt : GameSession::Instance().GetPlayer().GetLearned())
        if (lt.techniqueId == techId) return true;
    return false;
}

// 判断任务状态
static bool IsQuestStatus(const std::string& questId, QuestStatus status) {
    auto* q = QuestSystem::Instance().GetQuest(questId);
    return q && q->status == status;
}
static bool IsQuestAtLeast(const std::string& questId, QuestStatus minStatus) {
    auto* q = QuestSystem::Instance().GetQuest(questId);
    if (!q) return false;
    return static_cast<int>(q->status) >= static_cast<int>(minStatus);
}

DialogueState::DialogueState(const std::string& npcId) {
    m_type = GameStateType::Dialogue;
    m_npcId = npcId;
    m_npc = ConfigManager::Instance().GetNPC(npcId);

    if (!m_font.loadFromFile("C:/Windows/Fonts/simsun.ttc"))
        m_font.loadFromFile("C:/Windows/Fonts/msyh.ttc");

    m_namePlate.setFont(m_font);
    m_namePlate.setCharacterSize(20);
    m_namePlate.setFillColor(sf::Color(255, 215, 120));

    m_dialogueText.setFont(m_font);
    m_dialogueText.setCharacterSize(17);
    m_dialogueText.setFillColor(sf::Color(240, 240, 230));

    for (int i = 0; i < 4; ++i) {
        m_optionText[i].setFont(m_font);
        m_optionText[i].setCharacterSize(16);
        m_optionText[i].setPosition(80.f, 420.f + i * 30.f);
    }

    m_hintText.setFont(m_font);
    m_hintText.setCharacterSize(13);
    m_hintText.setFillColor(sf::Color(140, 140, 140));
    m_hintText.setPosition(580.f, 555.f);

    m_logText.setFont(m_font);
    m_logText.setCharacterSize(15);
    m_logText.setFillColor(sf::Color(100, 255, 150));  // 绿色获得提示
    m_logText.setPosition(240.f, 60.f);  // 对话框上方显示

    BuildDialogueTree();
}

void DialogueState::BuildDialogueTree() {
    m_dialogueNodes.clear();

    // 安全保护：如果 NPC 数据不存在，使用 npcId 作为名字构建默认对话
    std::wstring displayName;
    if (m_npc) {
        displayName = Utf8ToWide(m_npc->name);
    } else {
        // 从 npcId 推断显示名（备用方案）
        displayName = L"???";
        // 尝试从 WorldMapState 传来的 ID 做基本匹配
        if (m_npcId == "mo_dafu")       displayName = L"墨大夫";
        else if (m_npcId == "li_feiyu")  displayName = L"厉飞雨";
        else if (m_npcId == "zhang_tie") displayName = L"张铁";
        else if (m_npcId == "wang_hufa" || m_npcId == "王护法") displayName = L"王护法";
        else if (m_npcId == "yue_tangzhu" || m_npcId == "岳堂主") displayName = L"岳堂主";
        else if (m_npcId == "wan_xiaoshan") displayName = L"万小山";
        else if (m_npcId == "mo_yuzhu")  displayName = L"墨玉珠";
        else if (m_npcId == "chen_qiaoqian") displayName = L"陈巧倩";
        else if (m_npcId == "yaofang_huoji") displayName = L"药房伙计";
        else if (m_npcId == "shanmin")   displayName = L"山民";
        else if (m_npcId == "wuqishang") displayName = L"武器商";
        else if (m_npcId == "danyaoshi") displayName = L"丹药师";
        else if (m_npcId == "zahuoshang")displayName = L"杂货商";
        else if (m_npcId == "shuoshuren") displayName = L"说书人";
        else if (m_npcId == "xinghai_shouhuzhe") displayName = L"星海守护者";
        else if (m_npcId == "shoumen_dizi") displayName = L"守门弟子";
        else if (m_npcId == "san_shu")     displayName = L"韩三叔";
        else if (m_npcId == "zhang_jun")   displayName = L"张均";
        else if (m_npcId == "wu_yan")      displayName = L"舞岩";
    }

    const std::wstring& npcName = displayName;

    // 根据不同NPC构建不同的对话树
    if (m_npcId == "mo_dafu") {
        // 墨大夫 — 根据剧情阶段提供不同对话
        bool hasChangchun = HasLearned("changchun");

        // 阶段3：任务9"墨大夫的真相"激活 — 对峙
        if (IsQuestAtLeast("quest_009_mo_truth", QuestStatus::Active)) {
            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫背对着你，听到脚步声后缓缓转身）\n你来了...很好，我正好有事要跟你说。",
                {
                    {L"墨大夫，野狼帮的事跟你有关系吧？", [this]() { m_currentNode = 1; }},
                    {L"你到底想干什么？", [this]() { m_currentNode = 2; }},
                    {L"（保持沉默）", [this]() { m_currentNode = 3; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫眼中精光一闪，嘴角露出一丝意味深长的笑）\n呵呵...看来你比我想象的聪明。不错，野狼帮确实\n是我故意引来的。但你知道为什么吗？",
                {{L"为什么？", [this]() { m_currentNode = 2; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"因为我需要一副新的躯体！\n（墨大夫的面容突然变得狰狞）\n你的身体资质很不错，正好适合我。不要怪我，\n要怪就怪你命不好！",
                {{L"（摆出战斗姿态）做梦！", [this]() {
                    extern void OnStartCombat(const std::string& enemyId);
                    OnStartCombat("mo_dafu_boss");
                    m_finished = true;
                }}, {L"师父饶命！", [this]() { m_currentNode = 3; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"求饶是没用的。我筹划了这么久，岂会因你一句话\n就放弃？来吧，让我看看你这段时间的修炼成果！",
                {{L"（只能一战了！）", [this]() {
                    extern void OnStartCombat(const std::string& enemyId);
                    OnStartCombat("mo_dafu_boss");
                    m_finished = true;
                }}, {L"逃！", [this]() { m_finished = true; }}}
            });
        }
        // 阶段2：任务7"墨大夫的异样"激活 — 起疑心
        else if (IsQuestAtLeast("quest_007_mo_scheme", QuestStatus::Active)) {
            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫神色有些疲惫，眼神闪烁）\n你来了...最近修炼如何？有没有什么异样的感觉？",
                {
                    {L"师父，你最近是不是有什么事瞒着我？", [this]() { m_currentNode = 1; }},
                    {L"弟子修炼一切正常", [this]() { m_currentNode = 2; }},
                    {L"（总觉得师父今天怪怪的）", [this]() { m_currentNode = 3; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫脸色微变，随即恢复平静）\n呵呵...小孩子别胡思乱想。我只是一直在钻研一种\n新丹方，费神了些。你用心修炼就好，别的事...\n不要多问。",
                {{L"...是，弟子明白", [this]() { m_finished = true; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"嗯...那就好。我给你的长春功要多加练习，\n等你练到一定程度，我自然会教你更高深的功法。",
                {{L"多谢师父栽培", [this]() { m_finished = true; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（你注意到墨大夫的眼神中有一丝不易察觉的\n异样光芒，但转瞬即逝。他似乎沉浸在自己的\n思绪中，没有注意到你的观察。）",
                {{L"（默默退下，心中起疑）", [this]() { m_finished = true; }}}
            });
        }
        // 阶段1：已学长春功 — 指点修炼
        else if (hasChangchun) {
            m_dialogueNodes.push_back({
                npcName,
                L"你来了。口诀练得如何了？这套长春功虽然入门简单，\n但想要有所成，还需勤加修炼，不可懈怠。",
                { {} , {}, {} }
            });

            auto& opts = m_dialogueNodes.back().options;
            opts[0] = {L"请师父指点修炼", [this]() { m_currentNode = 1; }};
            opts[1] = {L"师父，张铁他修炼得如何？", [this]() { m_currentNode = 2; }};
            bool needBreakthrough = IsQuestAtLeast("quest_005_breakthrough", QuestStatus::Active);
            if (needBreakthrough) {
                opts[2] = {L"师父，我感觉到瓶颈了...", [this]() { m_currentNode = 3; }};
            } else {
                opts[2] = {L"弟子告退", [this]() { m_finished = true; }};
            }

            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫把了把你的脉，微微点头）\n嗯...经脉畅通了些，但气息还太弱。每日清晨和傍晚\n各打坐一个时辰，不可间断。",
                {{L"弟子遵命", [this]() { m_finished = true; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"张铁...他修炼的是象甲功，一种刚猛的外功。\n他体格魁梧，适合这门功夫，但这功法修炼起来\n要承受不小的苦楚。",
                {{L"原来如此...", [this]() { m_currentNode = 0; }}}
            });
            if (needBreakthrough) {
                m_dialogueNodes.push_back({
                    npcName,
                    L"（墨大夫眼中闪过一丝异色，随即笑道）\n瓶颈？哈哈，这是好事！说明你的积累已经到了\n突破的边缘。继续修炼，做好准备之后尝试突破\n境界。这是每个修仙者必经之路。",
                    {{L"弟子明白了！", [this]() { m_finished = true; }}}
                });
            }
        } else {
            // 初次见面 — 传功（原著第5-6章）
            m_dialogueNodes.push_back({
                npcName,
                L"你二人从即日起便是我的记名弟子。我会教你们一些\n采药炼药的常识，也许还会教你们一些医术。\n但决不会教你们武功。",
                { {} , {}, {} }
            });

            {
                auto& opts = m_dialogueNodes.back().options;
                opts[0] = {L"弟子明白，请师父传授功法", [this]() {
                    extern void OnLearnTechnique(const std::string& techId);
                    OnLearnTechnique("changchun");
                    AddLog(L"获得了【长春功】口诀！");
                    m_currentNode = 1;
                }};
                opts[1] = {L"请问师父，什么是修仙？", [this]() { m_currentNode = 2; }};
                opts[2] = {L"弟子先告退了", [this]() { m_finished = true; }};
            }
            m_dialogueNodes.push_back({
                npcName,
                L"我有一套修身养性的口诀要教你二人，虽然不能让你\n克敌制胜，但也能让你强身健体。你若是想学几手武功，\n可以去几位教习那里去学，我不会反对。",
                {{L"弟子谨记师父教诲（获得长春功）", [this]() {
                    extern void OnLearnTechnique(const std::string& techId);
                    OnLearnTechnique("changchun");
                    AddLog(L"获得了【长春功】口诀！");
                    m_finished = true;
                }}, {L"师父，什么是经脉？", [this]() { m_currentNode = 3; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"修仙？呵呵...那是一条不归路。你可知这世上的人\n分三六九等，修仙者也分三六九等。灵根资质、功法\n机缘、心性毅力，缺一不可。",
                {{L"...（若有所思）", [this]() { m_currentNode = 0; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（墨大夫捻了捻胡须）\n人体有十二正经、奇经八脉。修炼的第一步，就是\n感应体内的气，引导其在经脉中运转。你慢慢体会吧。",
                {{L"我试试看", [this]() { m_currentNode = 1; }}}
            });
        }

    } else if (m_npcId == "wang_hufa" || m_npcId == "王护法") {
        // 王护法 — 七玄门守门护法（原著第2章）
        // 根据剧情阶段区分对话
        if (IsQuestAtLeast("quest_008_wolf_attack", QuestStatus::Active) &&
            !IsQuestAtLeast("quest_008_wolf_attack", QuestStatus::Rewarded)) {
            // 任务8：野狼帮入侵阶段
            m_dialogueNodes.push_back({
                npcName,
                L"（王护法面色严峻，手持长刀）\n野狼帮的人已经攻过来了！\n韩立，你来得正好，后山那边也出现了敌人。\n去后山击退他们，别让野狼帮从后面包抄我们！",
                {
                    {L"是！我这就去后山！", [this]() { m_finished = true; }},
                    {L"野狼帮有多少人？", [this]() { m_currentNode = 1; }},
                    {L"我需要做准备", [this]() { m_finished = true; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"来势汹汹，少说也有百来号人！\n不过你不用担心正面，那里有岳堂主和我顶着。\n你只需去后山清理那些偷袭的杂碎就行。\n小心点，那些人不好对付！",
                {{L"明白，我这就出发！", [this]() { m_finished = true; }}}
            });
        } else {
            // 普通阶段
            m_dialogueNodes.push_back({
                npcName,
                L"哼！你就是新来的弟子？最近路上不太平，野狼帮的人\n越来越嚣张了。你没事别到处乱跑，老老实实在门内待着。",
                {
                    {L"请问护法，七玄门有多少弟子？", [this]() { m_currentNode = 1; }},
                    {L"野狼帮是什么？", [this]() { m_currentNode = 2; }},
                    {L"是，弟子明白", [this]() { m_finished = true; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"七玄门分内门外门，内门有百锻堂、七绝堂、供奉堂、\n血刃堂四个分堂。外门有飞鸟堂、聚宝堂、四海堂、\n外刃堂。门内弟子三四千人，是本地两大霸主之一！",
                {{L"多谢护法指点", [this]() { m_currentNode = 0; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"野狼帮？哼，一伙马贼出身的匪帮！仗着人多势众，\n经常跟我们七玄门抢地盘。真要打起来，他们哪是我们\n的对手！不过你小子还太嫩，遇上了就跑，知道吗？",
                {{L"...知道了", [this]() { m_currentNode = 0; }}}
            });
        }

    } else if (m_npcId == "yue_tangzhu" || m_npcId == "岳堂主") {
        // 岳堂主 — 考核主事（原著第4章）
        m_dialogueNodes.push_back({
            npcName,
            L"你就是这次通过测试的弟子之一？不错，能到炼骨崖顶\n的都不容易。不过修行之路还长着呢，这只是开始。\n你们这些新来的，都给我好好练！",
            {
                {L"堂主，炼骨崖测试是怎么回事？", [this]() { m_currentNode = 1; }},
                {L"弟子一定努力修行", [this]() { m_finished = true; }},
                {L"告辞了", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"先去竹林，再爬岩壁，最后攀上三十丈悬崖！\n能到崖顶的才能正式入门。不过你们这一批...\n也就那舞岩有点本事，可惜走了后门去了七绝堂。",
            {{L"（原来如此...）", [this]() { m_currentNode = 0; }}}
        });

    } else if (m_npcId == "li_feiyu") {
        // 厉飞雨 — 同门，性格直爽好战（原著）
        m_dialogueNodes.push_back({
            npcName,
            L"韩师弟！来来来，陪我练几招。\n我这身子骨虽然不太好，但剑法可没落下！",
            {
                {L"好，请飞雨兄指教", [this]() {
                    extern void OnStartCombat(const std::string& enemyId);
                    OnStartCombat("mortal_thug");
                    m_finished = true;
                }},
                {L"飞雨兄，你对七玄门怎么看？", [this]() { m_currentNode = 1; }},
                {L"改天再切磋", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"七玄门嘛...曾经也是越国数一数二的大门派。可惜自从\n七绝上人仙逝后，就一代不如一代了。不过瘦死的骆驼\n比马大，在这彩霞山一带，还是没人敢惹的。",
            {{L"原来如此...", [this]() { m_currentNode = 0; }}}
        });

    } else if (m_npcId == "wan_xiaoshan") {
        // 万小山
        m_dialogueNodes.push_back({
            npcName,
            L"韩兄弟！又来找我了？\n哈哈，正好我这里有几枚聚气丹，送给你吧！",
            {
                {L"多谢万兄！（接受礼物）", [this]() {
                    extern void OnGiveItem(const std::string& itemId, int count);
                    OnGiveItem("spirit_gathering_pill", 3);
                    m_currentNode = 1;
                }},
                {L"这怎么好意思...", [this]() { m_currentNode = 2; }},
                {L"我还有事先走了", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"咱俩谁跟谁啊！好好修炼，争取早日练气十层！\n到时候我们一起去黄枫谷拜师！",
            {{L"一言为定！", [this]() { m_finished = true; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"拿着拿着！别跟我客气！\n你要是不收，我可要生气了啊！",
            {{L"（只好收下）", [this]() {
                extern void OnGiveItem(const std::string& itemId, int count);
                OnGiveItem("spirit_gathering_pill", 3);
                m_currentNode = 1;
            }}}
        });

    } else if (m_npcId == "zhang_tie") {
        // 张铁 — 根据剧情阶段不同对话
        if (IsQuestAtLeast("quest_007_mo_scheme", QuestStatus::Active) &&
            !IsQuestAtLeast("quest_008_wolf_attack", QuestStatus::Rewarded)) {
            // 任务7激活后：发现墨大夫异样的阶段
            m_dialogueNodes.push_back({
                npcName,
                L"韩师兄！你来得正好！\n（张铁压低声音）你有没有觉得墨师父最近\n很奇怪？他经常一个人躲在炼丹房不出来。",
                {
                    {L"确实有些不对劲...", [this]() { m_currentNode = 1; }},
                    {L"你想多了吧？", [this]() { m_currentNode = 2; }},
                    {L"我去找他问问", [this]() { m_finished = true; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"对吧！我昨天晚上起来上厕所，看到炼丹房\n的灯还亮着，里面传来奇怪的声音...像是\n在念什么咒语。我一靠近，声音就停了。",
                {{L"（若有所思）这确实可疑...", [this]() { m_currentNode = 2; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"也许吧...但我总觉得心里不踏实。\n韩师兄，你修炼比我快，要是真有什么事，\n你可要帮帮我啊。",
                {{L"放心，有我在", [this]() { m_finished = true; }}}
            });
        } else {
            // 普通阶段
            m_dialogueNodes.push_back({
                npcName,
                L"韩师兄！今天的训练太累了...\n不过我感觉力气比以前大了不少！",
                {
                    {L"继续加油，张铁", [this]() { m_currentNode = 1; }},
                    {L"休息一下吧", [this]() { m_finished = true; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"是！师兄放心，我不会辜负墨师父的期望的！\n总有一天我要成为像七玄门长老那样厉害的人物！",
                {{L"（拍了拍他的肩膀）", [this]() { m_finished = true; }}}
            });
        }

    } else if (m_npcId == "mo_yuzhu") {
        // 墨玉珠
        m_dialogueNodes.push_back({
            npcName,
            L"韩立哥哥~ 你又来了呀！\n爹爹正在炼丹呢，让我来陪你聊聊吧~",
            {
                {L"玉珠妹妹好", [this]() { m_currentNode = 1; }},
                {L"你父亲在忙什么？", [this]() { m_currentNode = 2; }},
                {L"我找墨大夫有事", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"嘿嘿，韩立哥哥今天看起来精神不错嘛~\n是不是修炼又有进展了？",
            {{L"还行吧，一点点", [this]() { m_finished = true; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"爹爹说他在研究一种新丹方，已经闭关好几天了。\n他总是这样，一研究起来就什么都忘了...",
            {{L"原来如此", [this]() { m_currentNode = 0; }}}
        });

    } else if (m_npcId == "chen_qiaoqian") {
        // 陈巧倩
        m_dialogueNodes.push_back({
            npcName,
            L"（一位身着淡青色长裙的女子正静静地看着你）\n...你是外门弟子？",
            {
                {L"弟子见过师姐", [this]() { m_currentNode = 1; }},
                {L"路过而已", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"嗯...你的气息还算纯正。好好修炼，黄枫谷虽不是越国第一门派，\n但也足以让你在这片天地立足。",
            {{L"多谢师姐指点", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "wuqishang") {
        // 武器商
        m_dialogueNodes.push_back({
            npcName,
            L"客官，来看看！本店兵器都是精铁打造，削铁如泥！\n修仙路上，一把好兵刃可是保命的家伙。",
            {
                {L"我想买点东西", [this]() {
                    extern void OnOpenShop(const std::wstring& name, const std::string& type);
                    OnOpenShop(L"铁匠铺·武器商", "weapon");
                    m_finished = true;
                }},
                {L"这些刀剑怎么卖？", [this]() { m_currentNode = 1; }},
                {L"改天再来", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"小回春丹 8 灵石，铁木剑 30 灵石。\n要是你手头紧，我可以给你打个九折...开玩笑的，概不还价！",
            {{L"好吧，我看看有什么", [this]() {
                extern void OnOpenShop(const std::wstring& name, const std::string& type);
                OnOpenShop(L"铁匠铺·武器商", "weapon");
                m_finished = true;
            }}, {L"告辞了", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "danyaoshi") {
        // 丹药师
        m_dialogueNodes.push_back({
            npcName,
            L"炼丹一炷香，修仙万年长。\n...这位道友，我看你气息虚浮，可是需要些丹药调理？",
            {
                {L"请推荐一些丹药", [this]() {
                    extern void OnOpenShop(const std::wstring& name, const std::string& type);
                    OnOpenShop(L"百草堂·丹药师", "medicine");
                    m_finished = true;
                }},
                {L"筑基丹有吗？", [this]() { m_currentNode = 1; }},
                {L"只是路过看看", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"筑基丹？那可是辅助突破的珍贵丹药，650 灵石一枚。\n不过以你目前的境界，用了也是浪费...还是先从基础丹药开始吧。",
            {{L"那我先买些基础丹药", [this]() {
                extern void OnOpenShop(const std::wstring& name, const std::string& type);
                OnOpenShop(L"百草堂·丹药师", "medicine");
                m_finished = true;
            }}, {L"谢谢告知", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "zahuoshang") {
        // 杂货商
        m_dialogueNodes.push_back({
            npcName,
            L"嘿嘿，客官要什么？我这里什么都有——\n药材、矿石、杂七杂八的东西，就看你掏不掏得出灵石了！",
            {
                {L"让我看看你的货物", [this]() {
                    extern void OnOpenShop(const std::wstring& name, const std::string& type);
                    OnOpenShop(L"聚宝阁·杂货商", "misc");
                    m_finished = true;
                }},
                {L"有没有稀罕物？", [this]() { m_currentNode = 1; }},
                {L"只是随便逛逛", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"稀罕物？哈哈，那可多了去了！玄冰玉、万毒蝉蜕...\n不过价格嘛，嘿嘿嘿，都是好东西啊。",
            {{L"开个价看看", [this]() {
                extern void OnOpenShop(const std::wstring& name, const std::string& type);
                OnOpenShop(L"聚宝阁·杂货商", "misc");
                m_finished = true;
            }}, {L"太贵了算了", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "yaofang_huoji") {
        // 药房伙计
        m_dialogueNodes.push_back({
            npcName,
            L"这里是墨大夫的药房，闲人免进。\n...哦，你是师父的弟子？那进来吧，有什么需要的吗？",
            {
                {L"有没有疗伤的丹药？", [this]() {
                    extern void OnOpenShop(const std::wstring& name, const std::string& type);
                    OnOpenShop(L"药房·伙计", "medicine");
                    m_finished = true;
                }},
                {L"墨大夫在吗？", [this]() { m_currentNode = 1; }},
                {L"没什么事，打扰了", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"师父正在闭关炼丹，已经好几天了。\n你还是别去打扰他比较好...",
            {{L"好的，那我走了", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "san_shu") {
        // 韩三叔 — 原著第1-2章：引路人
        m_dialogueNodes.push_back({
            npcName,
            L"小立啊！到了七玄门要好好听师父的话。\n三叔我能帮你的就这么多了，以后的路要靠你自己走了。",
            {
                {L"三叔，七玄门是什么样的人家？", [this]() { m_currentNode = 1; }},
                {L"我一定不会给三叔丢脸的！", [this]() { m_currentNode = 2; }},
                {L"三叔保重，我去了", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"七玄门啊...那可是咱们方圆数百里数一数二的大门派！\n二百年前由七绝上人创立，曾经雄霸整个镜州呢。\n后来虽然没落了，但在这彩霞山一带，还是没人敢惹的。\n你只要进了内门，每月能领一两多银子，吃喝不愁！",
            {{L"（听起来确实不错...）", [this]() { m_currentNode = 0; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"好！有志气！你爹娘那边我会照看的，你放心去吧。\n记住：做人要老实，遇事要忍让，别和其他人起争执。\n要是真受了欺负...也别太委屈自己。",
            {{L"多谢三叔指点", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "zhang_jun") {
        // 张均 — 冷面师兄（原著第4章）
        m_dialogueNodes.push_back({
            npcName,
            L"（张均面无表情地看着你，眼神冷淡）\n...你就是上次通过测试的那个记名弟子？",
            {
                {L"是的，多谢师兄上次在崖壁上搭救", [this]() { m_currentNode = 1; }},
                {L"师兄有什么事吗？", [this]() { m_currentNode = 2; }},
                {L"...（不知该说什么）", [this]() { m_currentNode = 3; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"（张均微微点头）\n不必记在心上。那是我的职责。\n...你跟着墨大夫好好修炼吧，他医术高明，能学到东西。\n但记住，修仙界不比村里，凡事多留个心眼。",
            {{L"弟子谨记师兄教诲", [this]() { m_finished = true; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"...没什么。只是看你资质尚可，别浪费了。\n七玄门里不是所有人都靠真本事上位的，\n你好自为之。",
            {{L"（他是在暗示舞岩的事？）", [this]() { m_currentNode = 0; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"（张均不再说话，转身离去）\n...好自为之。",
            {{L"...谢谢师兄", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "wu_yan") {
        // 舞岩 — 骄横弟子（原著第3-4章）
        bool afterTest = IsQuestAtLeast("quest_001_start", QuestStatus::Rewarded);
        if (afterTest) {
            // 入门测试后 — 舞岩已经进了七绝堂，更加傲慢
            m_dialogueNodes.push_back({
                npcName,
                L"（舞岩穿着一身崭新的锦缎衣裳，手持一把镶玉长剑）\n哟，这不是那个差点没爬上来的记名弟子吗？\n怎么，还在跟那个老头子学医术呢？",
                {
                    {L"（不想搭理他）", [this]() { m_currentNode = 1; }},
                    {L"你管我跟谁学？", [this]() { m_currentNode = 2; }},
                    {L"舞师兄有何指教？", [this]() { m_currentNode = 3; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（舞岩不屑地哼了一声，转身就走）\n土包子就是土包子，跟你说话都掉价。",
                {{L"...（握紧拳头）", [this]() { m_finished = true; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"呵，嘴还挺硬！我可是进了七绝堂的核心弟子！\n你这种连内门都差点进不来的货色，也配跟我较劲？\n要不是我表姐夫是副门主...咳，我跟你说这些干什么。",
                {{L"（他差点说漏了什么）", [this]() { m_currentNode = 0; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（舞岩得意地扬了扬下巴）\n没什么，就是提醒你一句：以后见到我客气点。\n不然...哼，你知道后果的。",
                {{L"...", [this]() { m_finished = true; }}}
            });
        } else {
            // 初到彩霞山 — 还没测试
            m_dialogueNodes.push_back({
                npcName,
                L"（一个穿着华丽锦衣的少年上下打量着你）\n你就是那个从乡下来的？啧啧...这一身土味。\n明天的测试你肯定过不了，趁早回家种地去吧！",
                {
                    {L"走着瞧", [this]() { m_currentNode = 1; }},
                    {L"...（默默记住这个人）", [this]() { m_finished = true; }},
                    {L"你是谁？", [this]() { m_currentNode = 2; }}
                }
            });
            m_dialogueNodes.push_back({
                npcName,
                L"（舞岩冷笑一声）\n好，我等着看你在炼骨崖上哭爹喊娘的样子！",
                {{L"（此人来者不善）", [this]() { m_finished = true; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"我叫舞岩，我爸在城里开了三家武馆！\n我从小练武，岂是你们这些泥腿子能比的？\n看着吧，明天的第一名肯定是我！",
                {{L"原来只是个武馆少爷", [this]() { m_currentNode = 0; }}}
            });
        }

    } else if (m_npcId == "shuoshuren") {
        // 说书人 — 七玄门背景故事传播者
        m_dialogueNodes.push_back({
            npcName,
            L"（一位须发花白的老人正坐在茶摊边，手摇折扇）\n这位小友，可有兴趣听老夫讲讲七玄门的往事？",
            {
                {L"请老先生赐教", [this]() { m_currentNode = 1; }},
                {L"七玄门的历史？说来听听", [this]() { m_currentNode = 2; }},
                {L"改天再来听", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"话说二百年前，一位自称「七绝上人」的奇人横空出世！\n此人武功盖世，创立了七玄门，曾一度称霸镜州数十年！\n连越国其他大门派都不敢轻视。只可惜...",
            {{L"可惜什么？", [this]() { m_currentNode = 2; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"七绝上人病故后，七玄门就一落千丈了。被其他门派\n联手挤出了镜州首府，百年前才搬到这彩霞山落脚。\n如今的七玄门，虽然还有三四千弟子，但也就是个\n本地小势力了。跟野狼帮你来我往的，好不热闹。",
            {
                {L"野狼帮是什么？", [this]() { m_currentNode = 3; }},
                {L"原来七玄门还有这样的过往...", [this]() { m_finished = true; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"野狼帮啊...那是马贼出身的一伙狠人！被官府招安后\n就成了帮派，但凶狠嗜血的劲头一点没减。论打仗，\n七玄门还真不一定打得过他们。好在野狼帮不会经营，\n论富足还是七玄门这边强得多。",
            {{L"多谢老先生解惑", [this]() { m_finished = true; }}}
        });

    } else if (m_npcId == "shoumen_dizi") {
        // 守门弟子 — 彩霞山门卫
        m_dialogueNodes.push_back({
            npcName,
            L"站住！此地是七玄门重地，闲人不得擅入！\n...哦？你是新来的候选弟子？\n进去吧，沿着山路一直走，岳堂主在山顶等你们。",
            {
                {L"请问师兄，山上有什么规矩？", [this]() { m_currentNode = 1; }},
                {L"多谢师兄", [this]() { m_finished = true; }},
                {L"山顶远不远？", [this]() { m_currentNode = 2; }}
            }
        });
        m_dialogueNodes.push_back({
            npcName,
            L"规矩不多但都很严：不準私自下山，不準在山上斗殴，\n见到长老堂主要行礼。最要紧的一条——别得罪七绝堂\n的人，他们是门主的心尖子。",
            {{L"记下了，多谢师兄", [this]() { m_finished = true; }}}
        });
        m_dialogueNodes.push_back({
            npcName,
            L"说远不远，说近不近。沿着山路走两柱香的工夫就到了。\n明天一早开始选拔测试，今晚好好休息。",
            {{L"好的", [this]() { m_finished = true; }}}
        });

    } else {
        // 默认对话
        m_dialogueNodes.push_back({
            npcName,
            L"（这个人似乎没什么特别的想说）\n\"...你好。\"",
            {{L"离开", [this]() { m_finished = true; }}}
        });
    }
}

void DialogueState::Enter() {
    m_currentNode = 0;
    m_selectedOption = 0;
    m_showingOptions = false;
    m_finished = false;
    m_textTimer = 0.f;
    m_logTimer = 0.f;
    m_lastLog.clear();

    // 更新名称牌
    if (m_npc) {
        m_namePlate.setString(L"— " + Utf8ToWide(m_npc->name)
                             + L" (" + Utf8ToWide(m_npc->title) + L") —");
    } else {
        // 使用 npcId 推断名称（与 BuildDialogueTree 一致）
        std::wstring nameW = L"???";
        if (m_npcId == "mo_dafu")       nameW = L"墨大夫";
        else if (m_npcId == "li_feiyu")  nameW = L"厉飞雨";
        else if (m_npcId == "zhang_tie") nameW = L"张铁";
        else if (m_npcId == "wang_hufa" || m_npcId == "王护法") nameW = L"王护法";
        else if (m_npcId == "yue_tangzhu" || m_npcId == "岳堂主") nameW = L"岳堂主";
        else if (m_npcId == "wan_xiaoshan") nameW = L"万小山";
        else if (m_npcId == "mo_yuzhu")  nameW = L"墨玉珠";
        else if (m_npcId == "chen_qiaoqian") nameW = L"陈巧倩";
        else if (m_npcId == "yaofang_huoji") nameW = L"药房伙计";
        else if (m_npcId == "shanmin")   nameW = L"山民";
        else if (m_npcId == "wuqishang") nameW = L"武器商";
        else if (m_npcId == "danyaoshi") nameW = L"丹药师";
        else if (m_npcId == "zahuoshang")nameW = L"杂货商";
        else if (m_npcId == "shuoshuren") nameW = L"说书人";
        else if (m_npcId == "xinghai_shouhuzhe") nameW = L"星海守护者";
        else if (m_npcId == "shoumen_dizi") nameW = L"守门弟子";
        else if (m_npcId == "san_shu")     nameW = L"韩三叔";
        else if (m_npcId == "zhang_jun")   nameW = L"张均";
        else if (m_npcId == "wu_yan")      nameW = L"舞岩";
        m_namePlate.setString(L"— " + nameW + L" —");
    }

    // Boss战后检测：如果墨大夫已被击败，立即关闭对话让WorldMapState处理BossDefeatDialogue
    if (m_npcId == "mo_dafu") {
        auto* pendingBoss = QuestSystem::Instance().GetPendingBossDefeat();
        if (pendingBoss) {
            GameStateManager::Instance().PopState();
            return;
        }
    }
}

void DialogueState::Update(float dt) {
    m_textTimer += dt;
    if (m_logTimer > 0.f) m_logTimer -= dt;
    if (m_logTimer > 0.f) m_logTimer -= dt;

    if (m_showingOptions) {
        for (int i = 0; i < 4; ++i) {
            bool sel = (i == m_selectedOption);
            m_optionText[i].setFillColor(sel ? sf::Color(255, 220, 100) : sf::Color(170, 175, 180));
        }
    }
}

void DialogueState::HandleInput() {}

void DialogueState::OnKeyPressed(sf::Keyboard::Key key) {
    if (m_finished) {
        if (key == sf::Keyboard::Return || key == sf::Keyboard::Enter || key == sf::Keyboard::Escape) {
            // 返回上一状态
            GameStateManager::Instance().PopState();
        }
        return;
    }

    // 安全保护：对话树为空时，任何键都直接结束
    if (m_dialogueNodes.empty()) {
        m_finished = true;
        return;
    }

    if (m_showingOptions) {
        auto& node = m_dialogueNodes[m_currentNode];
        int optCount = static_cast<int>(node.options.size());

        switch (key) {
            case sf::Keyboard::Up:
                m_selectedOption = (m_selectedOption - 1 + optCount) % optCount;
                break;
            case sf::Keyboard::Down:
                m_selectedOption = (m_selectedOption + 1) % optCount;
                break;
            case sf::Keyboard::Return:
                if (m_selectedOption < optCount && !node.options.empty()) {
                    node.options[m_selectedOption].onSelected();
                }
                break;
            case sf::Keyboard::Escape:
                m_finished = true;
                break;
            default: break;
        }
    } else {
        // 显示选项
        if (key == sf::Keyboard::Return || key == sf::Keyboard::Enter ||
            key == sf::Keyboard::Space) {
            ShowOptions();
        } else if (key == sf::Keyboard::Escape) {
            m_finished = true;
        }
    }
}

void DialogueState::ShowOptions() {
    m_showingOptions = true;
    m_selectedOption = 0;
    m_hintText.setString(L"↑↓ 选择   Enter 确认");
}

// 内部辅助方法：添加日志（用于回调中）
void DialogueState::AddLog(const std::wstring& msg) {
    // 将消息存入 m_lastLog 供 Render 显示
    m_lastLog = msg;
    m_logTimer = 4.f;  // 显示4秒
}

void DialogueState::Render(sf::RenderWindow& window) {
    window.clear(sf::Color(15, 18, 28));

    // === 对话框背景（改进布局：NPC对话区+玩家选项区明确分离）===
    // 布局规划:
    // ┌──────────────────────────────────────┐
    // │ 名称牌 (y=30~52)                      │
    // │ ──分隔线── (y=55)                     │
    // │ NPC对话内容区域 (y=62~320)            │
    // │ （自动换行，限制行数防溢出）           │
    // │ ──分隔线── (选项上方)                 │
    // │ 选项列表 / 提示 (选项区域)            │
    // │ 按 Space/Enter 继续                   │
    // └──────────────────────────────────────┘

    int optCount = 0;
    if (m_currentNode < (int)m_dialogueNodes.size() && m_showingOptions) {
        optCount = (int)m_dialogueNodes[m_currentNode].options.size();
    }

    float boxW = 740.f;
    float boxX = 30.f;
    float boxY = 60.f;
    float contentAreaH = 200.f;     // NPC对话文字区域高度（增大）
    float optAreaH = m_showingOptions ? (optCount * 30.f + 10.f) : 30.f;
    float boxH = 50.f + contentAreaH + optAreaH + 30.f;

    // 对话框背景
    sf::RectangleShape dialogBox(sf::Vector2f(boxW, boxH));
    dialogBox.setFillColor(sf::Color(25, 30, 45, 245));
    dialogBox.setPosition(boxX, boxY);
    dialogBox.setOutlineThickness(2.f);
    dialogBox.setOutlineColor(sf::Color(70, 90, 120));
    window.draw(dialogBox);

    // 名称牌
    m_namePlate.setPosition(boxX + 20.f, boxY + 10.f);
    window.draw(m_namePlate);

    // 分隔线（名字下方）
    sf::RectangleShape sepLine(sf::Vector2f(boxW - 40.f, 1.f));
    sepLine.setFillColor(sf::Color(60, 75, 95));
    sepLine.setPosition(boxX + 20.f, boxY + 38.f);
    window.draw(sepLine);

    // NPC对话内容（自动换行 + 行数限制防溢出）
    if (m_currentNode < (int)m_dialogueNodes.size()) {
        const std::wstring& rawText = m_dialogueNodes[m_currentNode].text;

        // 将原始文本按\n分割，再对每行做自动换行（每行最多36个中文字符）
        int maxCharsPerLine = 36;
        std::vector<std::wstring> wrappedLines;

        // 逐段处理
        size_t pos = 0;
        while (pos < rawText.size()) {
            size_t nlPos = rawText.find(L'\n', pos);
            std::wstring segment = (nlPos != std::wstring::npos)
                ? rawText.substr(pos, nlPos - pos)
                : rawText.substr(pos);

            // 对这段文本做自动换行
            std::wstring curLine;
            for (wchar_t ch : segment) {
                curLine += ch;
                if ((int)curLine.size() >= maxCharsPerLine) {
                    // 在标点处断行
                    size_t bp = curLine.find_last_of(L" ，。、！？：；）】》");
                    if (bp != std::wstring::npos && bp > 2) {
                        wrappedLines.push_back(curLine.substr(0, bp + 1));
                        curLine = curLine.substr(bp + 1);
                    } else {
                        wrappedLines.push_back(curLine);
                        curLine.clear();
                    }
                }
            }
            if (!curLine.empty()) wrappedLines.push_back(curLine);

            if (nlPos == std::wstring::npos) break;
            pos = nlPos + 1;
        }

        // 限制行数：内容区域最多容纳的行数
        float lineH = 22.f;
        float maxLines = (contentAreaH - 10.f) / lineH;
        int maxLinesInt = std::max(1, (int)maxLines);
        if ((int)wrappedLines.size() > maxLinesInt) {
            wrappedLines.resize(maxLinesInt);
            if (!wrappedLines.empty()) wrappedLines.back() += L"...";
        }

        // 区分NPC对话和玩家对话（以"「"或"「"开头的为玩家对话）
        sf::Text dlgText;
        dlgText.setFont(m_font);
        dlgText.setCharacterSize(16);
        dlgText.setLineSpacing(3.f);

        float textY = boxY + 48.f;
        float maxTextY = boxY + 38.f + contentAreaH - 5.f;

        for (const auto& ln : wrappedLines) {
            if (textY + lineH > maxTextY) break;

            // 检测是否为玩家台词（以"「"开头的引号内容）
            bool isPlayerLine = false;
            if (ln.find(L"「") != std::wstring::npos || ln.find(L"「") != std::wstring::npos) {
                isPlayerLine = true;
            }

            dlgText.setString(ln);
            if (isPlayerLine) {
                dlgText.setFillColor(sf::Color(120, 220, 255));  // 玩家台词用蓝色
            } else {
                dlgText.setFillColor(sf::Color(240, 240, 230));  // NPC台词用白色
            }
            dlgText.setPosition(boxX + 25.f, textY);
            window.draw(dlgText);
            textY += lineH;
        }
    }

    // 选项分隔线
    float optSepY = boxY + 38.f + contentAreaH + 8.f;
    sf::RectangleShape optSepLine(sf::Vector2f(boxW - 40.f, 1.f));
    optSepLine.setFillColor(sf::Color(50, 65, 85));
    optSepLine.setPosition(boxX + 20.f, optSepY);
    window.draw(optSepLine);

    // 选项列表（动态位置，限制4个）
    float optStartY = optSepY + 8.f;
    if (m_showingOptions && m_currentNode < (int)m_dialogueNodes.size()) {
        const auto& opts = m_dialogueNodes[m_currentNode].options;
        for (size_t i = 0; i < opts.size() && i < 4; ++i) {
            std::wstring prefix = ((int)i == m_selectedOption) ? L"▶ " : L"  ";
            // 选项文本也做截断防溢出
            std::wstring optStr = opts[i].text;
            if ((int)optStr.size() > 30) {
                optStr = optStr.substr(0, 28) + L"...";
            }
            m_optionText[i].setString(prefix + optStr);
            m_optionText[i].setPosition(boxX + 40.f, optStartY + i * 30.f);
            window.draw(m_optionText[i]);
        }
    } else {
        m_hintText.setString(L"按 Space/Enter 继续...");
    }

    // 操作提示（对话框右下角内侧）
    m_hintText.setPosition(boxX + boxW - 200.f, boxY + boxH - 25.f);
    window.draw(m_hintText);

    // 获得物品/功法的提示（对话框上方显示）
    if (m_logTimer > 0.f && !m_lastLog.empty()) {
        m_logText.setString(L"✦ " + m_lastLog + L" ✦");
        float alpha = std::min(1.f, m_logTimer / 1.f);
        m_logText.setFillColor(sf::Color(
            static_cast<sf::Uint8>(100 + 155 * alpha),
            static_cast<sf::Uint8>(255 * alpha),
            static_cast<sf::Uint8>(100 + 100 * alpha),
            static_cast<sf::Uint8>(255 * alpha)
        ));
        m_logText.setPosition(240.f, 20.f);
        window.draw(m_logText);
    }
}
