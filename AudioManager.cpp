#pragma execution_character_set("utf-8")
#include "AudioManager.h"
#include <fstream>
#include <algorithm>

AudioManager::AudioManager() {
    LoadSettings();
}

AudioManager& AudioManager::Instance() {
    static AudioManager inst;
    return inst;
}

void AudioManager::PlayBGM(const std::string& filepath) {
    if (filepath == m_currentBGM && IsBGMPlaying()) return;

    if (!m_music.openFromFile(filepath)) return;

    m_currentBGM = filepath;
    m_music.setLoop(true);
    m_music.setVolume(m_bgmVolume);
    m_music.play();
}

void AudioManager::StopBGM() {
    m_music.stop();
}

void AudioManager::PauseBGM() {
    m_music.pause();
}

void AudioManager::ResumeBGM() {
    if (m_music.getStatus() == sf::Music::Paused)
        m_music.play();
}

void AudioManager::SetBGMVolume(float v) {
    m_bgmVolume = std::max(0.f, std::min(100.f, v));
    m_music.setVolume(m_bgmVolume);
}

void AudioManager::SetSFXVolume(float v) {
    m_sfxVolume = std::max(0.f, std::min(100.f, v));
}

bool AudioManager::IsBGMPlaying() const {
    return m_music.getStatus() == sf::Music::Playing;
}

void AudioManager::SaveSettings(const std::string& path) const {
    std::ofstream f(path);
    if (f.is_open()) {
        f << "bgm=" << m_bgmVolume << "\n";
        f << "sfx=" << m_sfxVolume << "\n";
    }
}

void AudioManager::LoadSettings(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.substr(0, 4) == "bgm=") {
            try { m_bgmVolume = std::stof(line.substr(4)); } catch (...) {}
        } else if (line.substr(0, 4) == "sfx=") {
            try { m_sfxVolume = std::stof(line.substr(4)); } catch (...) {}
        }
    }
    // 立即同步到已有音乐（如果有）
    m_music.setVolume(m_bgmVolume);
}
