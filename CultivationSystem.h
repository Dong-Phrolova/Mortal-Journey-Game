#pragma execution_character_set("utf-8")
#pragma once
#include <string>
#include <vector>
#include <cstdint>

// 修炼大境界（与凡人修仙传一致：练气→筑基→结丹→元婴→化神→炼虚→合体→大乘）
enum class MajorRealm : uint8_t {
    Qi,          // 练气期（1-13层，无子境界）
    ZhuJi,       // 筑基期（初/中/后/大圆满）
    JieDan,      // 结丹期
    YuanYing,    // 元婴期
    HuaShen,     // 化神期
    LianXu,      // 炼虚期
    HeTi,        // 合体期
    DaCheng,     // 大乘期
    DuJie,       // 渡劫期（状态，非独立境界）
    COUNT
};

// 子境界（每个大境界下的细分，练气期除外）
enum class SubStage : uint8_t {
    None,        // 练气期无子境界
    Early,       // 初期
    Mid,         // 中期
    Late,        // 后期
    GrandPerfection // 大圆满
};

// 五行灵根类型
enum class SpiritRoot : uint8_t {
    None,        // 无灵根（凡人）
    FiveElement, // 五行杂灵根
    FourElement, // 四灵根
    ThreeElement,// 三灵根
    DualElement,  // 双灵根
    SingleElement,// 单灵根
    HeavenSpirit,// 天灵根
    Mutated      // 变异灵根
};

// 元素属性
enum class Element : uint8_t {
    None, Metal, Wood, Water, Fire, Earth, Wind, Thunder, Ice
};

// 修炼境界完整描述（单一境界节点）
struct CultivationNode {
    std::string id;
    std::string name;           // 显示名称，如"练气三层"
    MajorRealm major;
    int qiLayer;                // 练气期层数（1-13），非练气期填0
    SubStage subStage;
    int hpBonus;
    int mpBonus;
    int atkBonus;
    int defBonus;
    int speedBonus;
    int spiritBonus;            // 神识加成
    int lifespan;               // 额外寿元（年），0=同凡人
    std::string description;
};

// 前向声明（PlayerCulti 的方法需要）
class CultivationSystem;

// 玩家修炼状态（运行时数据，可存档）
struct PlayerCulti {
    MajorRealm major = MajorRealm::Qi;
    int qiLayer = 1;              // 练气期层数 1~13
    SubStage subStage = SubStage::None; // 筑基及以上才有子境界
    SpiritRoot spiritRoot = SpiritRoot::ThreeElement;
    int cultivationExp = 0;        // 当前境界修炼进度
    int breakthroughReq = 100;     // 突破所需灵力

    // 计算当前境界总战力加成
    int GetTotalHpBonus(const CultivationSystem& sys) const;
    int GetTotalMpBonus(const CultivationSystem& sys) const;
    int GetTotalAtkBonus(const CultivationSystem& sys) const;
    int GetTotalDefBonus(const CultivationSystem& sys) const;
    int GetTotalSpeedBonus(const CultivationSystem& sys) const;
    int GetTotalSpiritBonus(const CultivationSystem& sys) const;
};

// ============ 修炼系统 ============
class CultivationSystem {
public:
    CultivationSystem();

    // 境界查询
    static int GetMajorRealmOrder(MajorRealm r);
    static std::string MajorRealmName(MajorRealm r);
    static std::string SubStageName(SubStage s);
    static std::string SpiritRootName(SpiritRoot r);

    // 节点查询
    const CultivationNode* GetNode(const std::string& id) const;
    const CultivationNode* GetNode(MajorRealm major, int qiLayer, SubStage sub) const;
    const CultivationNode* GetNextNode(const std::string& currentId) const;

    // 境界判断
    static bool CanBreakThrough(MajorRealm from, SubStage fromSub,
                                MajorRealm to,   SubStage toSub);

    // 获取当前境界的完整显示名（string / wstring 两版）
    static std::string  FullDisplayName(MajorRealm major, int qiLayer, SubStage sub);
    static std::wstring FullDisplayNameW(MajorRealm major, int qiLayer, SubStage sub);
    static std::wstring MajorRealmNameW(MajorRealm r);
    static std::wstring SubStageNameW(SubStage s);
    static std::wstring SpiritRootNameW(SpiritRoot r);

    // 辅助：根据修炼状态生成任务系统用的等级字符串
    // 格式："qi_3"（练气3层），"zhuji_early"（筑基初期）
    static std::string GetLevelString(const PlayerCulti& culti);

    // 所有境界节点（按修炼顺序排列）
    const std::vector<CultivationNode>& GetAllNodes() const { return m_nodes; }

private:
    void BuildNodes();
    std::vector<CultivationNode> m_nodes;
};

