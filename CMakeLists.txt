cmake_minimum_required(VERSION 3.14)
project(XianxiaCombat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ============ SFML ============
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

# ============ nlohmann/json（头文件库）============
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ============ 源文件 ============
set(SRC_FILES
    main.cpp
    Player.cpp
    Enemy.cpp
    Combat.cpp
    CultivationSystem.cpp
    GameState.cpp
    ConfigManager.cpp
    ItemSystem.cpp
    GameSession.cpp
    GameCallbacks.cpp
    AudioManager.cpp
    MainMenuState.cpp
    WorldMapState.cpp
    CombatState.cpp
    DialogueState.cpp
    CultivationUIState.cpp
    InventoryState.cpp
    SettingsState.cpp
    ShopState.cpp
    QuestSystem.cpp
    QuestUIState.cpp
    EquipmentSystem.cpp
    TileSet.cpp
    TileMap.cpp
    PlayerSprite.cpp
    Camera.cpp
    MapEnemy.cpp
    SimpleMapState.cpp
    StoryIntroState.cpp
)

add_executable(XianxiaCombat ${SRC_FILES})

target_link_libraries(XianxiaCombat
    PRIVATE sfml-graphics sfml-window sfml-system sfml-audio
    PRIVATE nlohmann_json
)

# MSVC UTF-8 support
if(MSVC)
    target_compile_options(XianxiaCombat PRIVATE /utf-8)
endif()

# ============ 复制资源到构建目录（Windows）============
if(WIN32)
    # SFML DLL
    add_custom_command(
        TARGET XianxiaCombat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE:sfml-audio>
            $<TARGET_FILE_DIR:XianxiaCombat>
    )
    # OpenAL + 其他 extlibs
    add_custom_command(
        TARGET XianxiaCombat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>
            $<TARGET_FILE_DIR:XianxiaCombat>
        VERBATIM
    )
    # config 目录
    add_custom_command(
        TARGET XianxiaCombat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/config
            $<TARGET_FILE_DIR:XianxiaCombat>/config
        VERBATIM
    )
    # songs 目录
    add_custom_command(
        TARGET XianxiaCombat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/songs
            $<TARGET_FILE_DIR:XianxiaCombat>/songs
        VERBATIM
    )
endif()
