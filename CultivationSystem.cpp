#pragma execution_character_set("utf-8")
#include "CultivationSystem.h"
#include <stdexcept>
#include <sstream>

// 大境界排序序号
int CultivationSystem::GetMajorRealmOrder(MajorRealm r) {
    return static_cast<int>(r);
}

std::string CultivationSystem::MajorRealmName(MajorRealm r) {
    switch (r) {
        case MajorRealm::Qi:       return "练气期";
        case MajorRealm::ZhuJi:    return "筑基期";
        case MajorRealm::JieDan:   return "结丹期";
        case MajorRealm::YuanYing: return "元婴期";
        case MajorRealm::HuaShen:  return "化神期";
        case MajorRealm::LianXu:   return "炼虚期";
        case MajorRealm::HeTi:     return "合体期";
        case MajorRealm::DaCheng:  return "大乘期";
        case MajorRealm::DuJie:    return "渡劫期";
        default: return "未知";
    }
}

std::string CultivationSystem::SubStageName(SubStage s) {
    switch (s) {
        case SubStage::None:          return "";
        case SubStage::Early:         return "初期";
        case SubStage::Mid:           return "中期";
        case SubStage::Late:          return "后期";
        case SubStage::GrandPerfection: return "大圆满";
        default: return "";
    }
}

std::string CultivationSystem::SpiritRootName(SpiritRoot r) {
    switch (r) {
        case SpiritRoot::None:           return "无灵根";
        case SpiritRoot::FiveElement:     return "五行杂灵根";
        case SpiritRoot::FourElement:     return "四灵根";
        case SpiritRoot::ThreeElement:    return "三灵根";
        case SpiritRoot::DualElement:     return "双灵根";
        case SpiritRoot::SingleElement:   return "单灵根";
        case SpiritRoot::HeavenSpirit:   return "天灵根";
        case SpiritRoot::Mutated:        return "变异灵根";
        default: return "未知";
    }
}

std::string CultivationSystem::FullDisplayName(MajorRealm major, int qiLayer, SubStage sub) {
    std::ostringstream oss;
    if (major == MajorRealm::Qi) {
        oss << "练气" << qiLayer << "层";
    } else {
        oss << MajorRealmName(major);
        std::string subName = SubStageName(sub);
        if (!subName.empty()) oss << subName;
    }
    return oss.str();
}

std::wstring CultivationSystem::MajorRealmNameW(MajorRealm r) {
    switch (r) {
        case MajorRealm::Qi:       return L"练气期";
        case MajorRealm::ZhuJi:    return L"筑基期";
        case MajorRealm::JieDan:   return L"结丹期";
        case MajorRealm::YuanYing: return L"元婴期";
        case MajorRealm::HuaShen:  return L"化神期";
        case MajorRealm::LianXu:   return L"炼虚期";
        case MajorRealm::HeTi:     return L"合体期";
        case MajorRealm::DaCheng:  return L"大乘期";
        case MajorRealm::DuJie:    return L"渡劫期";
        default: return L"未知";
    }
}

std::wstring CultivationSystem::SubStageNameW(SubStage s) {
    switch (s) {
        case SubStage::None:           return L"";
        case SubStage::Early:          return L"初期";
        case SubStage::Mid:            return L"中期";
        case SubStage::Late:           return L"后期";
        case SubStage::GrandPerfection:return L"大圆满";
        default: return L"";
    }
}

std::wstring CultivationSystem::SpiritRootNameW(SpiritRoot r) {
    switch (r) {
        case SpiritRoot::None:           return L"无灵根";
        case SpiritRoot::FiveElement:     return L"五行杂灵根";
        case SpiritRoot::FourElement:     return L"四灵根";
        case SpiritRoot::ThreeElement:    return L"三灵根";
        case SpiritRoot::DualElement:     return L"双灵根";
        case SpiritRoot::SingleElement:   return L"单灵根";
        case SpiritRoot::HeavenSpirit:   return L"天灵根";
        case SpiritRoot::Mutated:        return L"变异灵根";
        default: return L"未知";
    }
}

std::wstring CultivationSystem::FullDisplayNameW(MajorRealm major, int qiLayer, SubStage sub) {
    if (major == MajorRealm::Qi) {
        return L"练气" + std::to_wstring(qiLayer) + L"层";
    } else {
        std::wstring s = MajorRealmNameW(major);
        std::wstring subName = SubStageNameW(sub);
        if (!subName.empty()) s += subName;
        return s;
    }
}

std::string CultivationSystem::GetLevelString(const PlayerCulti& culti) {
    std::string s;
    switch (culti.major) {
        case MajorRealm::Qi:
            s = "qi_" + std::to_string(culti.qiLayer);
            break;
        case MajorRealm::ZhuJi:
            s = "zhuji_";
            switch (culti.subStage) {
                case SubStage::Early:         s += "early"; break;
                case SubStage::Mid:           s += "mid"; break;
                case SubStage::Late:          s += "late"; break;
                case SubStage::GrandPerfection: s += "peak"; break;
                default: s += "early"; break;
            }
            break;
        case MajorRealm::JieDan:
            s = "jiedan_";
            switch (culti.subStage) {
                case SubStage::Early:         s += "early"; break;
                case SubStage::Mid:           s += "mid"; break;
                case SubStage::Late:          s += "late"; break;
                case SubStage::GrandPerfection: s += "peak"; break;
                default: s += "early"; break;
            }
            break;
        default:
            // 其他大境界，简单处理
            s = "qi_1";  // 默认
            break;
    }
    return s;
}

bool CultivationSystem::CanBreakThrough(MajorRealm from, SubStage fromSub,
                                        MajorRealm to,   SubStage toSub) {
    int fromOrder = GetMajorRealmOrder(from);
    int toOrder   = GetMajorRealmOrder(to);
    if (fromOrder == toOrder) {
        // 同大境界只能升子境界
        return static_cast<int>(toSub) == static_cast<int>(fromSub) + 1;
    }
    // 升大境界
    return toOrder == fromOrder + 1;
}

CultivationSystem::CultivationSystem() {
    BuildNodes();
}

void CultivationSystem::BuildNodes() {
    // 练气期：1~13层（无子境界）
    m_nodes.push_back({"qi_1",   "练气一层",  MajorRealm::Qi, 1, SubStage::None,   0,   0,  0,  0,  0,  0, 0, "初入修仙，灵根未定"});
    m_nodes.push_back({"qi_2",   "练气二层",  MajorRealm::Qi, 2, SubStage::None,   5,   3,  2,  1,  1,  2, 0, "灵气初聚"});
    m_nodes.push_back({"qi_3",   "练气三层",  MajorRealm::Qi, 3, SubStage::None,  10,   6,  4,  2,  2,  4, 0, "可修简单法术"});
    m_nodes.push_back({"qi_4",   "练气四层",  MajorRealm::Qi, 4, SubStage::None,  15,   9,  6,  3,  3,  6, 0, "可御使低阶法器"});
    m_nodes.push_back({"qi_5",   "练气五层",  MajorRealm::Qi, 5, SubStage::None,  20,  12,  8,  4,  4,  8, 0, ""});
    m_nodes.push_back({"qi_6",   "练气六层",  MajorRealm::Qi, 6, SubStage::None,  26,  16, 10,  5,  5, 10, 0, ""});
    m_nodes.push_back({"qi_7",   "练气七层",  MajorRealm::Qi, 7, SubStage::None,  32,  20, 13,  6,  6, 13, 0, ""});
    m_nodes.push_back({"qi_8",   "练气八层",  MajorRealm::Qi, 8, SubStage::None,  38,  24, 16,  8,  7, 16, 0, ""});
    m_nodes.push_back({"qi_9",   "练气九层",  MajorRealm::Qi, 9, SubStage::None,  45,  28, 19, 10,  8, 19, 0, ""});
    m_nodes.push_back({"qi_10",  "练气十层",  MajorRealm::Qi,10, SubStage::None,  52,  33, 22, 12, 10, 22, 0, "练气顶峰，可尝试筑基"});
    m_nodes.push_back({"qi_11",  "练气十一层",MajorRealm::Qi,11, SubStage::None,  60,  38, 26, 14, 12, 26, 0, ""});
    m_nodes.push_back({"qi_12",  "练气十二层",MajorRealm::Qi,12, SubStage::None,  68,  43, 30, 16, 14, 30, 0, ""});
    m_nodes.push_back({"qi_13",  "练气十三层",MajorRealm::Qi,13, SubStage::None,  77,  49, 35, 18, 16, 35, 0, "圆满之境，筑基瓶颈已近"});

    // 筑基期
    m_nodes.push_back({"zhuji_early", "筑基初期",  MajorRealm::ZhuJi, 0, SubStage::Early,         120, 80,  50, 30, 20, 50, 200, "寿元两百，可御剑飞行"});
    m_nodes.push_back({"zhuji_mid",   "筑基中期",  MajorRealm::ZhuJi, 0, SubStage::Mid,           160,110,  65, 40, 25, 65, 200, ""});
    m_nodes.push_back({"zhuji_late",  "筑基后期",  MajorRealm::ZhuJi, 0, SubStage::Late,          200,140,  80, 50, 30, 80, 200, ""});
    m_nodes.push_back({"zhuji_peak",  "筑基大圆满",MajorRealm::ZhuJi, 0, SubStage::GrandPerfection,250,180, 100, 60, 35,100, 200, "可尝试凝结金丹"});

    // 结丹期
    m_nodes.push_back({"jindan_early","结丹初期",  MajorRealm::JieDan,0, SubStage::Early,         400,300, 150, 80, 50,150, 500, "结成金丹，寿元五百，可炼本命法宝"});
    m_nodes.push_back({"jindan_mid",  "结丹中期",  MajorRealm::JieDan,0, SubStage::Mid,           500,380, 190,100, 60,190, 500, ""});
    m_nodes.push_back({"jindan_late", "结丹后期",  MajorRealm::JieDan,0, SubStage::Late,          600,460, 230,120, 70,230, 500, ""});
    m_nodes.push_back({"jindan_peak", "结丹大圆满",MajorRealm::JieDan,0, SubStage::GrandPerfection,700,550, 280,140, 80,280, 500, "假婴之境"});

    // 元婴期（先定义到元婴初期，后续可扩展）
    m_nodes.push_back({"yuanying_early","元婴初期",MajorRealm::YuanYing,0,SubStage::Early,       1000,800,400,200,100,400,1000,"丹破婴生，大道初成，元婴不灭可夺舍"});
}

const CultivationNode* CultivationSystem::GetNode(const std::string& id) const {
    for (const auto& n : m_nodes) if (n.id == id) return &n;
    return nullptr;
}

const CultivationNode* CultivationSystem::GetNode(MajorRealm major, int qiLayer, SubStage sub) const {
    for (const auto& n : m_nodes) {
        if (n.major == major && n.qiLayer == qiLayer && n.subStage == sub)
            return &n;
    }
    return nullptr;
}

const CultivationNode* CultivationSystem::GetNextNode(const std::string& currentId) const {
    for (size_t i = 0; i + 1 < m_nodes.size(); ++i) {
        if (m_nodes[i].id == currentId)
            return &m_nodes[i + 1];
    }
    return nullptr;
}

// ============ PlayerCulti 成员实现 ============
int PlayerCulti::GetTotalHpBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->hpBonus : 0;
}
int PlayerCulti::GetTotalMpBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->mpBonus : 0;
}
int PlayerCulti::GetTotalAtkBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->atkBonus : 0;
}
int PlayerCulti::GetTotalDefBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->defBonus : 0;
}
int PlayerCulti::GetTotalSpeedBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->speedBonus : 0;
}
int PlayerCulti::GetTotalSpiritBonus(const CultivationSystem& sys) const {
    const CultivationNode* node = sys.GetNode(major, qiLayer, subStage);
    return node ? node->spiritBonus : 0;
}
