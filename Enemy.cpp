#pragma execution_character_set("utf-8")
#include "Enemy.h"
#include <cstdlib>

static int RandInt(int l, int r) { return l + rand() % (r - l + 1); }

Enemy::Enemy(const std::wstring& name, int hp, int mp, int atk, int def, EnemyType type)
    : m_name(name), m_maxHp(hp), m_hp(hp), m_maxMp(mp), m_mp(mp)
    , m_baseAtk(atk), m_baseDef(def), m_type(type) {}

void Enemy::TakeDamage(int dmg) {
    int actual = std::max(1, dmg - m_baseDef / 2);
    m_hp -= actual;
    if (m_hp < 0) m_hp = 0;
}

int Enemy::ChooseAction() {
    // 高境界敌人更聪明：有概率使用技能
    int maxRoll = (m_major > MajorRealm::Qi) ? 10 : 6;
    int roll = RandInt(0, maxRoll);
    if (roll < 5) return 0;   // 普通攻击
    if (roll < 8) return 1;   // 防御
    return 2;                   // 技能
}

int Enemy::GetAttack() const {
    int mult = 1;
    if (m_major == MajorRealm::ZhuJi) mult = 2;
    else if (m_major == MajorRealm::JieDan) mult = 3;
    else if (m_major >= MajorRealm::YuanYing) mult = 5;
    return m_baseAtk * mult;
}

int Enemy::GetDefense() const { return m_baseDef; }
int Enemy::GetSpeed()   const { return 5 + static_cast<int>(m_major) * 3; }
