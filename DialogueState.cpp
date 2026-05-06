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
                    OnStartCombat("mortal_thug");
                    m_finished = true;
                }}, {L"师父饶命！", [this]() { m_currentNode = 3; }}}
            });
            m_dialogueNodes.push_back({
                npcName,
                L"求饶是没用的。我筹划了这么久，岂会因你一句话\n就放弃？来吧，让我看看你这段时间的修炼成果！",
                {{L"（只能一战了！）", [this]() {
                    extern void OnStartCombat(const std::string& enemyId);
                    OnStartCombat("mortal_thug");
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
        m_namePlate.setString(L"— " + nameW + L" —");
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

    // 对话框背景
    sf::RectangleShape dialogBox(sf::Vector2f(740.f, 280.f));
    dialogBox.setFillColor(sf::Color(25, 30, 45, 245));
    dialogBox.setPosition(30.f, 100.f);
    dialogBox.setOutlineThickness(2.f);
    dialogBox.setOutlineColor(sf::Color(70, 90, 120));
    window.draw(dialogBox);

    // 名称牌
    m_namePlate.setPosition(50.f, 110.f);
    window.draw(m_namePlate);

    // 分隔线
    sf::RectangleShape sepLine(sf::Vector2f(700.f, 1.f));
    sepLine.setFillColor(sf::Color(60, 75, 95));
    sepLine.setPosition(50.f, 142.f);
    window.draw(sepLine);

    // 对话内容
    if (m_currentNode < (int)m_dialogueNodes.size()) {
        m_dialogueText.setString(m_dialogueNodes[m_currentNode].text);
        m_dialogueText.setPosition(50.f, 155.f);
        // 支持自动换行
        m_dialogueText.setLineSpacing(4.f);
        window.draw(m_dialogueText);
    }

    // 选项列表
    if (m_showingOptions && m_currentNode < (int)m_dialogueNodes.size()) {
        const auto& opts = m_dialogueNodes[m_currentNode].options;
        for (size_t i = 0; i < opts.size() && i < 4; ++i) {
            std::wstring prefix = ((int)i == m_selectedOption) ? L"▶ " : L"  ";
            m_optionText[i].setString(prefix + opts[i].text);
            window.draw(m_optionText[i]);
        }
    } else {
        m_hintText.setString(L"按 Space/Enter 继续...");
    }

    m_hintText.setPosition(550.f, 558.f);
    window.draw(m_hintText);

    // 获得物品/功法的提示（对话框上方显示）
    if (m_logTimer > 0.f && !m_lastLog.empty()) {
        m_logText.setString(L"✦ " + m_lastLog + L" ✦");
        float alpha = std::min(1.f, m_logTimer / 1.f);  // 渐隐效果
        m_logText.setFillColor(sf::Color(
            static_cast<sf::Uint8>(100 + 155 * alpha),
            static_cast<sf::Uint8>(255 * alpha),
            static_cast<sf::Uint8>(100 + 100 * alpha),
            static_cast<sf::Uint8>(255 * alpha)
        ));
        window.draw(m_logText);
    }
}
