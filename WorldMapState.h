#pragma once
#include "GameState.h"
#include "TileMap.h"
#include "PlayerSprite.h"
#include "Camera.h"
#include "MapEnemy.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>

// Note: MapData is defined in TileMap.h
// using GameMapDef = MapData; // Use MapData directly

class WorldMapState : public GameState {
public:
    WorldMapState();
    ~WorldMapState() override = default;

    void Enter() override;
    void Exit() override {}
    void Update(float dt) override;
    void HandleInput() override {}
    void Render(sf::RenderWindow& window) override;

    void OnKeyPressed(sf::Keyboard::Key key);

    // 切换到指定地图
    void SwitchToMap(const std::string& mapId, int spawnX = -1, int spawnY = -1);

private:
    // 地图数据
    std::map<std::string, MapData> m_maps;
    MapData* m_currentMap = nullptr;

    // 核心对象
    TileMap m_tileMap;
    PlayerSprite m_player;
    Camera m_camera;
    MapEnemySystem m_enemySystem;  // 地图敌人系统

    // UI 字体
    sf::Font m_font;

    // 玩家在地图上的瓦片坐标
    int m_playerTileX = 1;
    int m_playerTileY = 1;

    // 移动节流：防止按住键连续移动
    float m_moveCooldown = 0.f;
    const float MOVE_INTERVAL = 0.18f; // 每次移动间隔（秒）

    // 遇敌冷却
    float m_encounterCooldown = 0.f;

    // 对话框相关
    bool m_showMapName = true;
    float m_mapNameTimer = 0.f;

    // 初始化所有地图
    void InitAllMaps();
    void BuildQixuanmen();   // 七玄门·外门
    void BuildJiazhou();     // 嘉州城
    void BuildHuangfengu();  // 黄枫谷·山门
    void BuildXuese();       // 血色禁地·入口
    void BuildQixuanmenBack(); // 七玄门·后山
    void BuildJiazhouMarket();  // 嘉州城·市场
    void BuildLuanshenghai();  // 乱星海
    void BuildCaixiaMountain(); // 彩霞山（七玄门主峰）
    void BuildLianguCliff();    // 炼骨崖（采药场景）
    void BuildShenshuValley();  // 神手谷（墨大夫隐居地）
    void BuildQingniuTown();    // 青牛镇

    // 处理玩家移动
    void TryMovePlayer(int dx, int dy);

    // 检查并处理特殊格子
    void HandleSpecialTile();

    // 触发遇敌
    void TryEncounter();

    // 渲染 HUD（顶部状态栏+底部提示）
    void RenderHUD(sf::RenderWindow& window);
    void RenderNPCNames(sf::RenderWindow& window);
    void RenderMapEnemies(sf::RenderWindow& window, float camOffsetX, float camOffsetY);

    // 交互系统：选项菜单 + 对话
    enum class InteractState { None, ShowOptions, InDialogue, ShowNarration, BossDefeatDialogue };
    InteractState m_interactState = InteractState::None;
    int m_nearbyNPCId = -1;
    int m_selectedOption = 0;
    std::vector<std::wstring> m_dialogueLines;
    size_t m_dialogueLineIndex = 0;

    void RenderInteractionUI(sf::RenderWindow& window);
    void ProcessInteractionInput(sf::Keyboard::Key key);
    void StartNPCOptions(int npcId);
    void StartNPCDialogue(int npcId);
    void StartNPCDuel(int npcId);

    // 浮动提示文字（用于宝箱获得物品等）
    void ShowFloatingText(const std::wstring& msg);
    sf::Text m_floatText;
    std::wstring m_floatMsg;
    float m_floatTimer = 0.f;

    // 引路提示系统：闪烁黄色感叹号
    float m_guideAnimTimer = 0.f;  // 闪烁动画计时
    void RenderQuestGuides(sf::RenderWindow& window);
    void RemoveMoDafuNPC();  // 击败墨大夫后移除所有地图上的墨大夫NPC
    struct GuideMarker {
        int tileX;
        int tileY;
        enum Type { NPC, Door } type;
    };
    std::vector<GuideMarker> m_currentGuideMarkers;

    // 持久化的已开启宝箱集合（格式："mapId_x_y"）
    std::set<std::string> m_openedChests;
};
