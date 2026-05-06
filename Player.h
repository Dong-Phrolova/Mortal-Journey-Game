#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "CultivationSystem.h"

// 已学会的功法/技能
struct LearnedTechnique {
    std::string techniqueId;
    int level = 1;
};

class Player {
public:
    Player(const std::wstring& name,
            int baseHp, int baseMp,
            int baseAtk, int baseDef);

    // 属性查询（基础 + 境界加成 + 功法加成）
    int GetMaxHp()      const;
    int GetMaxMp()      const;
    int GetAttack()      const;
    int GetDefense()     const;
    int GetSpeed()       const;
    int GetSpirit()      const;  // 神识

    // 当前值
    int GetCurrentHp() const { return m_hp; }
    int GetCurrentMp() const { return m_mp; }
    void SetCurrentHp(int v) { m_hp = v; if (m_hp > GetMaxHp()) m_hp = GetMaxHp(); if (m_hp < 0) m_hp = 0; }
    void SetCurrentMp(int v) { m_mp = v; if (m_mp > GetMaxMp()) m_mp = GetMaxMp(); if (m_mp < 0) m_mp = 0; }

    bool IsAlive() const { return m_hp > 0; }

    // 受到伤害
    void TakeDamage(int damage) { m_hp -= damage; if (m_hp < 0) m_hp = 0; }

    // 使用灵力
    bool UseMp(int cost) { if (m_mp < cost) return false; m_mp -= cost; return true; }

    // 修炼系统
    const PlayerCulti& GetCulti() const { return m_culti; }
    void  SetCulti(const PlayerCulti& c) { m_culti = c; }
    PlayerCulti& GetCultiMutable() { return m_culti; }  // 需要修改时使用

    // 功法
    bool LearnTechnique(const std::string& techniqueId);
    bool UpgradeTechnique(const std::string& techniqueId);
    const std::vector<LearnedTechnique>& GetLearned() const { return m_learned; }
    const LearnedTechnique* GetLearned(const std::string& techniqueId) const;

    // 物品/灵石
    int  GetGold() const { return m_gold; }
    void AddGold(int a) { m_gold += a; }
    bool SpendGold(int a) { if (m_gold < a) return false; m_gold -= a; return true; }

    // 突破
    bool CanBreakThrough() const;
    bool BreakThrough();  // 返回是否成功

    const std::wstring& GetName() const { return m_name; }
    std::wstring GetRealmName() const;
    MajorRealm GetMajorRealm() const { return m_culti.major; }

private:
    int CalcHp()      const;
    int CalcMp()      const;
    int CalcAtk()     const;
    int CalcDef()     const;
    int CalcSpeed()   const;
    int CalcSpirit()  const;

    std::wstring m_name;
    // 基础值
    int m_baseHp, m_baseMp, m_baseAtk, m_baseDef;
    // 当前值
    int m_hp, m_mp;
    // 货币
    int m_gold = 0;
    // 修炼
    PlayerCulti m_culti;
    std::vector<LearnedTechnique> m_learned;
};
