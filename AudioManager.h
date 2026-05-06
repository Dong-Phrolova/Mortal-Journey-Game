#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <vector>

// ============ 全局音频管理器（单例）============
class AudioManager {
public:
    static AudioManager& Instance();

    // 背景音乐
    void PlayBGM(const std::string& filepath);
    void StopBGM();
    void PauseBGM();
    void ResumeBGM();

    // 音量控制 (0~100)
    void   SetBGMVolume(float v);
    float  GetBGMVolume() const { return m_bgmVolume; }

    void   SetSFXVolume(float v);
    float  GetSFXVolume() const { return m_sfxVolume; }

    // 是否正在播放
    bool IsBGMPlaying() const;

    // 保存/加载音量设置（存 audio.cfg）
    void SaveSettings(const std::string& path = "audio.cfg") const;
    void LoadSettings(const std::string& path = "audio.cfg");

private:
    AudioManager();
    sf::Music m_music;
    float m_bgmVolume = 60.f;
    float m_sfxVolume = 80.f;
    std::string m_currentBGM;
};
