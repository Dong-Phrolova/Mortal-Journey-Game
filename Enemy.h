#pragma once
#include <string>
#include <vector>
#include "CultivationSystem.h"

// 敌人类型
enum class EnemyType { Mortal, Qi, ZhuJi, JieDan, YuanYing, YaoBeast, MoXiu };

// 单个敌人数据（运行时）
class Enemy {
public:
    Enemy() = default;
    Enemy(const std::wstring& name, int hp, int mp, int atk, int def,
          EnemyType type = EnemyType::Mortal);

    void TakeDamage(int dmg);
    bool IsAlive() const { return m_hp > 0; }

    int  ChooseAction();  // 0=攻击 1=防御 2=技能
    int  GetAttack() const;
    int  GetDefense() const;
    int  GetSpeed() const;

    // AI：根据修炼境界决定行为概率
    void SetCultivation(MajorRealm m, int qiLayer = 0, SubStage sub = SubStage::None) {
        m_major = m; m_qiLayer = qiLayer; m_subStage = sub;
    }

    const std::wstring& GetName() const { return m_name; }
    int  GetHp() const { return m_hp; }
    int  GetMaxHp() const { return m_maxHp; }
    EnemyType GetType() const { return m_type; }

private:
    std::wstring m_name;
    int m_maxHp, m_hp;
    int m_maxMp, m_mp;
    int m_baseAtk, m_baseDef;
    EnemyType m_type;
    MajorRealm m_major = MajorRealm::Qi;
    int m_qiLayer = 1;
    SubStage m_subStage = SubStage::None;
};

// 敌人模板（从 JSON 加载）
struct EnemyTemplate {
    std::string id;
    std::string name;
    EnemyType type;
    std::string realmReq;   // 对应境界 id
    int baseHp;
    int baseMp;
    int baseAtk;
    int baseDef;
    int expReward;
    int goldReward;
    std::vector<std::string> dropTable; // 可能掉落的道具 id
};
