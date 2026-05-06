#pragma execution_character_set("utf-8")
#include "Player.h"
#include "CultivationSystem.h"
#include "ConfigManager.h"
#include "EquipmentSystem.h"
#include <algorithm>
#include <cmath>

static CultivationSystem g_cultivationSys;

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
    m_culti.breakthroughReq = 100;
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

bool Player::CanBreakThrough() const {
    return m_culti.cultivationExp >= m_culti.breakthroughReq;
}

bool Player::BreakThrough() {
    if (!CanBreakThrough()) return false;
    const auto* curNode = g_cultivationSys.GetNode(
        m_culti.major, m_culti.qiLayer, m_culti.subStage);
    if (!curNode) return false;
    const auto* next = g_cultivationSys.GetNextNode(curNode->id);
    if (!next) return false;

    m_culti.major    = next->major;
    m_culti.qiLayer  = next->qiLayer;
    m_culti.subStage = next->subStage;
    m_culti.cultivationExp = 0;
    m_culti.breakthroughReq = static_cast<int>(m_culti.breakthroughReq * 1.5f);

    // 突破后回满
    m_hp = GetMaxHp();
    m_mp = GetMaxMp();
    return true;
}

std::wstring Player::GetRealmName() const {
    return g_cultivationSys.FullDisplayNameW(m_culti.major, m_culti.qiLayer, m_culti.subStage);
}
