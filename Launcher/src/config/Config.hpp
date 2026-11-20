#pragma once
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>

struct Config {
    std::string custom_args;
    std::string doom_game_path;
    int renderer =
        0; // 0 = OpenGL (DOOMx64.exe +r_renderAPI 0), 1 = Vulkan (DOOMx64vk.exe +r_renderAPI 1)
    bool xp_multiplier_enabled = false;
    float xp_multiplier = 1.0f;
    bool unlock_all_enabled = false;
    bool snapmap_enabled = false;
    bool skip_intro_enabled = true;

    int net_peerTimeoutInSeconds = 120;
    int net_peerTimeoutInSeconds_Lobby = 120;
    int net_connectTimeoutInSeconds = 120;
    int net_waitOnPeerToLoadTimer_Seconds = 120;
    int net_forceStartMatchTimeLimit = 120;
    int net_peerTimeoutDuringLoadMS = 120000;

    ImVec4 baseColor = ImVec4(0.0196f, 0.2431f, 0.1059f, 1.0f);
    ImVec4 baseHover = ImVec4(0.0510f, 0.1922f, 0.0902f, 1.0f);
    ImVec4 baseActive = ImVec4(0.0353f, 0.2706f, 0.1020f, 1.0f);
};

Config load_config(const std::string& file);
void save_config(const Config& cfg, const std::string& file);
