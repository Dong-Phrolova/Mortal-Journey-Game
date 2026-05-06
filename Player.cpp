#pragma execution_character_set("utf-8")
#include "Player.h"
#include "CultivationSystem.h"
#include "ConfigManager.h"
#include "EquipmentSystem.h"
#include "GameSession.h"
#include <algorithm>
#include <cmath>

static CultivationSystem g_cultivationSys;

// ============================================================
//  功法境界工具：从 cultivationReq 提取境界
// ============================================================
// "qi_1" → "qi", "zhuji_early" → "zhuji"
std::string Player::GetRealmFromTechnique(const std::string& cultivationReq) {
    size_t pos = cultivationReq.find('_');
    if (pos == std::string::npos) return cultivationReq;
    return cultivationReq.substr(0, pos);
}

Player::Player(const std::wstring& name,
                 int baseHp, int baseMp,
                 int baseAtk, int baseDef)
    : m_name(name)
    , m_baseHp(baseHp), m_baseMp(baseMp)
    , m_baseAtk(baseAtk), m_baseDef(baseDef)
    , m_hp(baseHp), m_mp(baseMp)
    , m_gold(50)   // 初始灵石
{
    // 默认练气一层
    m_culti.major     = MajorRealm::Qi;
    m_culti.qiLayer   = 1;
    m_culti.subStage  = SubStage::None;
    m_culti.spiritRoot     = SpiritRoot::ThreeElement;
    m_culti.cultivationExp = 0;
    m_culti.breakthroughReq = 150;  // Qi 1→2 需要150经验
}

int Player::GetMaxHp()  const { return std::max(1, CalcHp());  }
int Player::GetMaxMp()  const { return std::max(0, CalcMp());  }
int Player::GetAttack()  const { return std::max(1, CalcAtk()); }
int Player::GetDefense() const { return std::max(0, CalcDef()); }
int Player::GetSpeed()   const { return std::max(1, CalcSpeed()); }
int Player::GetSpirit()  const { return std::max(0, CalcSpirit());}

int Player::CalcHp() const {
    int v = m_baseHp + m_culti.GetTotalHpBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->hpPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalHp();
    return v;
}

int Player::CalcMp() const {
    int v = m_baseMp + m_culti.GetTotalMpBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->mpPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalMp();
    return v;
}

int Player::CalcAtk() const {
    int v = m_baseAtk + m_culti.GetTotalAtkBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->atkPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalAtk();
    return v;
}

int Player::CalcDef() const {
    int v = m_baseDef + m_culti.GetTotalDefBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->defPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalDef();
    return v;
}

int Player::CalcSpeed() const {
    int v = 10 + m_culti.GetTotalSpeedBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->speedPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalSpd();
    return v;
}

int Player::CalcSpirit() const {
    int v = 20 + m_culti.GetTotalSpiritBonus(g_cultivationSys);
    for (const auto& lt : m_learned) {
        const auto* t = ConfigManager::Instance().GetTechnique(lt.techniqueId);
        if (t) v += t->spiritPerLevel * lt.level;
    }
    v += EquipmentSystem::Instance().TotalSpirit();
    return v;
}

bool Player::LearnTechnique(const std::string& techniqueId) {
    if (GetLearned(techniqueId)) return false; // 已学会
    const auto* t = ConfigManager::Instance().GetTechnique(techniqueId);
    if (!t) return false;
    m_learned.push_back({techniqueId, 1});
    return true;
}

bool Player::UpgradeTechnique(const std::string& techniqueId) {
    for (auto& lt : m_learned) {
        if (lt.techniqueId == techniqueId) {
            const auto* t = ConfigManager::Instance().GetTechnique(techniqueId);
            if (!t || lt.level >= t->maxLevel) return false;
            // 消耗功法点数：功法所属境界需有至少1点
            std::string realm = GetRealmFromTechnique(t->cultivationReq);
            auto it = m_techPoints.find(realm);
            if (it == m_techPoints.end() || it->second < 1) return false;
            it->second--;
            lt.level++;
            return true;
        }
    }
    return false;
}

const LearnedTechnique* Player::GetLearned(const std::string& techniqueId) const {
    for (const auto& lt : m_learned)
        if (lt.techniqueId == techniqueId) return &lt;
    return nullptr;
}

// ============================================================
//  突破经验需求表
// ============================================================
static int GetBreakthroughReq(MajorRealm major, int qiLayer, SubStage subStage) {
    if (major == MajorRealm::Qi) {
        // 练气期 1→2 到 12→13
        static const int qiReqs[] = {150, 200, 260, 330, 400, 480, 570, 670, 780, 900, 1050, 1200, 1500};
        int idx = qiLayer - 1;
        if (idx >= 0 && idx < 13) return qiReqs[idx];
        return 1500;
    }
    if (major == MajorRealm::ZhuJi) {
        static const int zhujiReqs[] = {2200, 3000, 4000, 6000};
        int idx = (int)subStage;
        if (idx >= 0 && idx < 4) return zhujiReqs[idx];
        return 6000;
    }
    return 8000;  // 更高境界，以后填充
}

// ============================================================
//  突破丹映射
// ============================================================
std::string Player::GetRequiredBreakthroughPill() const {
    if (m_culti.major == MajorRealm::Qi) {
        if (m_culti.qiLayer < 13) return "qi_breakthrough_pill";
        return "zhuji_breakthrough_pill";  // Qi 13→筑基
    }
    if (m_culti.major == MajorRealm::ZhuJi) {
        if (m_culti.subStage < SubStage::GrandPerfection) return "zhuji_breakthrough_pill";
        return "jiedan_breakthrough_pill";  // 筑基圆满→结丹
    }
    return "jiedan_breakthrough_pill";
}

int Player::GetBreakthroughPillCount() const {
    return 1;  // 每次突破消耗1颗
}

bool Player::ConsumeBreakthroughPill() {
    std::string pillId = GetRequiredBreakthroughPill();
    auto& inv = GameSession::Instance().GetInventory();
    if (!inv.HasItem(pillId, 1)) return false;
    inv.RemoveItem(pillId, 1);
    return true;
}

// ============================================================
//  功法点数系统
// ============================================================
void Player::AddTechniquePoints(const std::string& realm, int count) {
    m_techPoints[realm] += count;
}

int Player::GetTechniquePoints(const std::string& realm) const {
    auto it = m_techPoints.find(realm);
    return (it != m_techPoints.end()) ? it->second : 0;
}

bool Player::CanBreakThrough() const {
    return m_culti.cultivationExp >= m_culti.breakthroughReq;
}

bool Player::BreakThrough() {
    if (!CanBreakThrough()) return false;

    // 检查是否有对应的突破丹（Qi 1→2 教程豁免，不需要丹药）
    bool needsPill = !(m_culti.major == MajorRealm::Qi && m_culti.qiLayer == 1);
    if (needsPill && !ConsumeBreakthroughPill()) return false;

    const auto* curNode = g_cultivationSys.GetNode(
        m_culti.major, m_culti.qiLayer, m_culti.subStage);
    if (!curNode) return false;
    const auto* next = g_cultivationSys.GetNextNode(curNode->id);
    if (!next) return false;

    // 消耗超出部分的经验，保留给下一层
    int excess = m_culti.cultivationExp - m_culti.breakthroughReq;
    m_culti.cultivationExp = std::max(0, excess);

    m_culti.major    = next->major;
    m_culti.qiLayer  = next->qiLayer;
    m_culti.subStage = next->subStage;

    // 查表获取下一层突破需求
    m_culti.breakthroughReq = GetBreakthroughReq(m_culti.major, m_culti.qiLayer, m_culti.subStage);

    // 突破后送2点对应境界的功法点数
    if (m_culti.major == MajorRealm::Qi) {
        AddTechniquePoints("qi", 2);
    } else if (m_culti.major == MajorRealm::ZhuJi) {
        AddTechniquePoints("zhuji", 2);
    } else if (m_culti.major == MajorRealm::JieDan) {
        AddTechniquePoints("jindan", 2);
    }

    // 突破后回满
    m_hp = GetMaxHp();
    m_mp = GetMaxMp();
    return true;
}

std::wstring Player::GetRealmName() const {
    return g_cultivationSys.FullDisplayNameW(m_culti.major, m_culti.qiLayer, m_culti.subStage);
}
