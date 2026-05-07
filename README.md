# 凡人修仙传 RPG

基于忘语同名小说《凡人修仙传》改编的 2D 像素风仙侠角色扮演游戏。

---

## 游戏简介

你扮演山村少年**韩立**，从青牛镇出发，拜入七玄门，踏上漫漫修仙之路。在墨大夫的指导下修炼长春功，探索后山寻找神秘小瓶，调查炼骨崖的阴谋，击退野狼帮的入侵……一步步揭开更大的修仙世界。

游戏包含完整的 **境界修炼体系、回合制战斗、主线剧情任务、开放式地图探索和 NPC 对话系统**，所有视觉资源均由程序化生成，无需外部图片资源。

---

## 技术栈

| 技术 | 说明 |
|---|---|
| **语言** | C++17 |
| **图形引擎** | SFML 2.6.1（Graphics / Window / System / Audio） |
| **配置格式** | JSON（nlohmann/json 3.11.3） |
| **构建工具** | CMake ≥ 3.14 |
| **平台** | Windows（MSVC） |
| **编码** | UTF-8 |

---

## 快速开始

### 构建

```bash
git clone <repo-url>
cd XianxiaCombat
mkdir build && cd build
cmake ..
cmake --build .
```

构建完成后会在 `build/` 目录生成 `XianxiaCombat.exe`，`config/` 和 `songs/` 目录会自动复制到输出目录。

### 运行

直接运行 `XianxiaCombat.exe`。游戏窗口大小为 **800×600**，锁定 60fps。

---

## 操作说明

| 按键 | 功能 |
|---|---|
| `W A S D` / 方向键 | 移动角色 |
| `E` | 与 NPC 对话 |
| `F` | 互动（开启宝箱等） |
| `Space` | 修炼（打开修炼界面快捷键） |
| `C` | 修炼界面内进行修炼（积累经验） |
| `U` | 修炼界面内升级功法 |
| `B` | 修炼界面内尝试突破境界 |
| `Q` | 打开任务界面 |
| `I` | 打开背包 |
| `Esc` | 打开设置菜单 / 返回上一层 |

**战斗中**

| 按键 | 功能 |
|---|---|
| `↑ ↓` 选择 | 切换菜单选项 |
| `Enter` | 确认选择 |
| `Esc` | 返回上级菜单 |

---

## 游戏玩法

### 境界修炼

游戏的核心成长系统，共设 **9 大境界**：

练气（1-13层）→ 筑基 → 结丹 → 元婴 → 化神 → 炼虚 → 合体 → 大乘 → 渡劫

**灵根品质**：无灵根 → 伪灵根（五/四/三灵根）→ 真灵根（双/单灵根）→ 天灵根 → 异灵根

**修炼操作**：
- **按 C 修炼**：积累境界经验，效率与已学功法和境界相关
- **按 B 突破**：经验满后消耗突破丹，突破到下一层/境界，获得功法点数
- **按 U 升级功法**：消耗对应境界的功法点数，提升已学功法等级，解锁新技能

### 战斗系统

回合制战斗，玩家与敌人轮流行动。

**玩家行动**：
- **攻击**：普通攻击，伤害 = max(1, 攻击力 - 敌人防御/2)
- **技能**：从已学功法中动态构建技能列表，消耗灵力施展
- **道具**：使用背包中的丹药恢复或增益
- **逃跑**：成功率与双方速度差相关，范围 10%~90%

**敌人 AI**：
- 普通敌人随机选择攻击/防御/技能
- Boss（墨大夫）具有三阶段AI，根据血量比例切换战术

**技能体系**（功法等级解锁）：

| 功法 | 解锁技能 |
|---|---|
| 长春功 | 木灵疗愈（Lv.5）/ 春回大地（Lv.5）/ 木灵护体（Lv.10） |
| 眨眼剑法 | 万剑归宗（Lv.4） |
| 青元剑诀 | 青元剑域 |
| 罗烟步 | 烟云身法（Lv.3）/ 瞬步（Lv.5） |
| 大衍决 | 神识探查 / 幻术 / 神识冲击 |

### 任务系统

覆盖原著关键剧情的 **11 个主线任务**：

| # | 任务 | 关键目标 |
|---|---|---|
| 1 | 初到青牛镇 | 与三叔对话 → 去七玄门 → 学长春功 |
| 2 | 勤修苦练 | 突破练气二层 + 长春功 Lv.2 |
| 3 | 初试锋芒 | 击败 3 个敌人 |
| 4 | 坊市见闻 | 与武器商 / 丹药师对话 |
| 5 | 突破瓶颈 | 突破成功 + 长春功 Lv.5 |
| 6 | 神秘小瓶 | 后山探索 → 获得神秘小瓶 |
| 7 | 墨大夫的异样 | 与张铁对话 → 调查炼骨崖 |
| 8 | 野狼帮入侵 | 与王护法对话 → 击败敌人 |
| 9 | 墨大夫的真相 | Boss 战：墨大夫 |
| 10 | 新的开始 | 前往黄枫谷 → 击败弟子 |
| 11 | 血色禁地 | 进入禁地 → 击败妖兽 → 长春功 Lv.7 |

任务状态流转：`未解锁 → 进行中 → 可领奖 → 已完成`

### 地图探索

共 **11 张地图**，通过传送门（Door 瓦片）连接：

| 地图 | 说明 | 是否有敌人 |
|---|---|---|
| 青牛镇 | 初始地图 | 否 |
| 七玄门外门 | 中央枢纽，连接各区域 | 否 |
| 七玄门后山 | 练级区，神秘小瓶位置 | 是 |
| 神手谷 | 墨大夫驻地 | 否 |
| 炼骨崖 | 最终 Boss 战地点 | 否 |
| 彩霞山 | 七玄门主峰 | 是 |
| 嘉州城 | 城镇区域 | 是 |
| 嘉州城市场 | 安全交易区 | 否 |
| 黄枫谷 | 筑基期加入的门派 | 是 |
| 血色禁地 | 高难度副本 | 是 |
| 乱星海 | 高级区域 | 是 |

地图元素包括：墙壁、路径、地面、地板、传送门、水域、树木、草丛（遇敌）、宝箱、栅栏、NPC 等。

### 背包与装备

- **背包**：40 格上限，物品可堆叠
- **装备槽**：武器 / 防具 / 饰品
- **物品类型**：材料、丹药、宝物、武器、防具、饰品、功法秘籍、杂物
- **装备品质**：普通（白）→ 优秀（绿）→ 稀有（蓝）→ 史诗（紫）→ 传说（金）
- **特殊效果**：吸血、反击、闪避、暴击

### 商店系统

6 种商店类型：武器店、丹药店、功法店、装备店、杂货店、综合店。支持购买和出售模式。

### 存档系统

3 个存档槽位，保存内容：
- 玩家属性（HP/MP/境界/灵石）
- 已学功法（含等级）与功法点数
- 背包物品
- 任务进度
- 装备状态
- 地图宝箱开启状态
- 当前位置

自动存档：退出游戏时、战斗结束后。

---

## 项目架构

### 目录结构

```
XianxiaCombat/
├── main.cpp                  # 程序入口，主循环
├── CMakeLists.txt            # 构建配置
├── config/                   # JSON 配置文件
│   ├── items.json            # 物品数据（17种）
│   ├── techniques.json       # 功法数据（5门）
│   ├── npcs.json             # NPC 数据（21个）
│   └── cultivation.json      # 境界数据（22级）
├── songs/                    # 音频资源
├── docs/                     # 文档
│
├── 框架层
│   ├── GameState.h/cpp       # 状态机基类 + 状态管理器
│   ├── GameSession.h/cpp     # 会话管理 + 存档/读档
│   ├── GameCallbacks.h/cpp   # 全局回调（状态切换）
│   ├── ConfigManager.h/cpp   # JSON 配置加载器
│   └── AudioManager.h/cpp    # 音频管理
│
├── 数据模型
│   ├── Player.h/cpp          # 玩家数据
│   ├── Enemy.h/cpp           # 敌人数据 + AI
│   ├── CultivationSystem.h/cpp  # 境界系统
│   ├── ItemSystem.h/cpp      # 物品 + 背包
│   ├── EquipmentSystem.h/cpp # 装备系统
│   └── QuestSystem.h/cpp     # 任务系统（含旁白）
│
├── 渲染
│   ├── TileSet.h/cpp         # 程序化瓦片纹理
│   ├── TileMap.h/cpp         # 瓦片地图系统
│   ├── PlayerSprite.h/cpp    # 角色精灵 + 行走动画
│   ├── Camera.h/cpp          # 摄像机跟随
│   └── MapEnemy.h/cpp        # 地图敌人 AI
│
└── UI 状态（游戏界面）
    ├── MainMenuState.h/cpp       # 主菜单
    ├── StoryIntroState.h/cpp     # 开场剧情
    ├── WorldMapState.h/cpp       # 世界地图（核心）
    ├── CombatState.h/cpp         # 战斗界面
    ├── DialogueState.h/cpp       # 对话界面
    ├── CultivationUIState.h/cpp  # 修炼界面
    ├── InventoryState.h/cpp      # 背包界面
    ├── ShopState.h/cpp           # 商店界面
    ├── QuestUIState.h/cpp        # 任务界面
    ├── SettingsState.h/cpp       # 设置/存档
    └── SimpleMapState.h/cpp      # 旧版地图（兼容）
```

### 架构设计

游戏采用 **状态机模式** 管理各个界面，所有 UI 界面继承自 `GameState` 基类，由 `GameStateManager`（单例）通过栈结构统一管理：

```
GameStateManager (栈)
├── MainMenuState
├── WorldMapState       ← 核心，地图探索
├── CombatState         ← 战斗中
├── DialogueState       ← 对话中
├── QuestUIState        ← 任务界面（叠加在地图之上）
├── InventoryState      ← 背包界面
├── CultivationUIState  ← 修炼界面
├── ShopState           ← 商店界面
└── SettingsState       ← 设置界面
```

**渲染流程**：从栈底到栈顶依次渲染（上层覆盖下层），支持半透明叠加层。

**跨状态数据共享**：通过 `GameSession`（单例）持有 `Player` 和 `InventorySystem`，所有状态均可访问。

### 数据驱动

所有游戏数据（物品、功法、NPC、境界）以 JSON 格式存储在 `config/` 目录中，运行时由 `ConfigManager` 加载解析。修改 JSON 即可调整游戏数值，无需重新编译核心逻辑。

---

## 配置修改

### 物品（config/items.json）

```json
{
  "items": [
    {
      "id": "hp_potion_small",
      "name": "小回春丹",
      "type": "Medicine",
      "description": "恢复50点生命",
      "stackable": 99,
      "value": 5,
      "effectType": "RestoreHp",
      "effectValue": 50
    }
  ]
}
```

### 功法（config/techniques.json）

```json
{
  "techniques": [
    {
      "id": "changchun",
      "name": "长春功",
      "type": "功法",
      "element": "木",
      "maxLevel": 10,
      "cultivationReq": "Qi_1",
      "effects": { "hpPerLevel": 15, "mpPerLevel": 5, "atkPerLevel": 3 },
      "unlocksSkill": [
        { "level": 5, "skillName": "木灵疗愈", "skillDesc": "恢复自身30%生命" }
      ]
    }
  ]
}
```

### NPC（config/npcs.json）

```json
{
  "npcs": [
    {
      "id": "han_li",
      "name": "韩立",
      "title": "七玄门弟子",
      "location": "神手谷",
      "cultivation": "练气期",
      "relation": 0
    }
  ]
}
```

### 境界（config/cultivation.json）

```json
{
  "realms": [
    { "id": "Qi_1",  "name": "练气一层",    "tier": 1, "subStage": 1,
      "hpBonus": 0, "mpBonus": 0, "atkBonus": 0, "defBonus": 0,
      "speedBonus": 0, "spiritBonus": 0, "lifespan": 0 }
  ]
}
```

---

## 开发状态

### 已实现功能

- [x] 地图探索（11 张地图，瓦片渲染，传送门）
- [x] 玩家移动（4 方向行走动画）
- [x] 摄像机平滑跟随
- [x] 回合制战斗（攻击/技能/道具/逃跑）
- [x] Boss 战（墨大夫 3 阶段 AI）
- [x] 境界修炼系统（练气期完整）
- [x] 功法系统（学习/升级/技能解锁）
- [x] 5 门功法及配套技能
- [x] 物品/背包系统（40 格）
- [x] 装备系统（3 槽位，24 件装备）
- [x] 11 个主线任务（覆盖原著剧情）
- [x] 任务旁白/剧情演出
- [x] NPC 对话系统（条件树，20+ NPC）
- [x] 商店系统（6 种类型）
- [x] 地图敌人 AI（巡逻/追踪/碰撞）
- [x] 存档/读档（3 槽位）
- [x] 主菜单/设置界面
- [x] 开场剧情
- [x] BGM 播放
- [x] 程序化像素纹理（无外部图片）

### 待实现

- [ ] 完整的筑基期及以上的境界内容
- [ ] 更多 BOSS 战（野狼帮主等）
- [ ] 支线任务系统
- [ ] 炼丹/炼器系统
- [ ] 灵兽系统
- [ ] 宗门贡献系统
- [ ] 更多地图（天南、乱星海、大晋等）
- [ ] 音效系统（非 BGM 的短音效）

---

## 参考资料

- 原著：《凡人修仙传》——忘语
- 游戏引擎：[SFML](https://www.sfml-dev.org/) 2.6.1
- JSON 库：[nlohmann/json](https://github.com/nlohmann/json) 3.11.3
