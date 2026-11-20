#include "Config.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

void to_json(ordered_json& j, const Config& config) {
    j = ordered_json{
        {"custom_args", config.custom_args},
        {"doom_game_path", config.doom_game_path},
        {"renderer", config.renderer},
        {"xp_multiplier_enabled", config.xp_multiplier_enabled},
        {"xp_multiplier", config.xp_multiplier},
        {"unlock_all_enabled", config.unlock_all_enabled},
        {"snapmap_enabled", config.snapmap_enabled},
        {"skip_intro_enabled", config.skip_intro_enabled},
        {"net_peerTimeoutInSeconds", config.net_peerTimeoutInSeconds},
        {"net_peerTimeoutInSeconds_Lobby", config.net_peerTimeoutInSeconds_Lobby},
        {"net_connectTimeoutInSeconds", config.net_connectTimeoutInSeconds},
        {"net_waitOnPeerToLoadTimer_Seconds", config.net_waitOnPeerToLoadTimer_Seconds},
        {"net_forceStartMatchTimeLimit", config.net_forceStartMatchTimeLimit},
        {"net_peerTimeoutDuringLoadMS", config.net_peerTimeoutDuringLoadMS},

        {"baseColor",
         {config.baseColor.x, config.baseColor.y, config.baseColor.z, config.baseColor.w}},
        {"baseHover",
         {config.baseHover.x, config.baseHover.y, config.baseHover.z, config.baseHover.w}},
        {"baseActive",
         {config.baseActive.x, config.baseActive.y, config.baseActive.z, config.baseActive.w}}};
}

void from_json(const json& j, Config& config) {
    if (j.contains("custom_args"))
        config.custom_args = j["custom_args"];
    if (j.contains("doom_game_path"))
        config.doom_game_path = j["doom_game_path"];
    if (j.contains("renderer"))
        config.renderer = j["renderer"];
    if (config.renderer < 0 || config.renderer > 1)
        config.renderer = 0;
    if (j.contains("xp_multiplier"))
        config.xp_multiplier = j["xp_multiplier"].get<float>();
    if (config.xp_multiplier < 0.0f)
        config.xp_multiplier = 0.0f;
    if (j.contains("xp_multiplier_enabled"))
        config.xp_multiplier_enabled = j["xp_multiplier_enabled"].get<bool>();
    else if (j.contains("xp_multiplier"))
        config.xp_multiplier_enabled = (config.xp_multiplier != 1.0f);
    if (j.contains("unlock_all_enabled"))
        config.unlock_all_enabled = j["unlock_all_enabled"].get<bool>();
    if (j.contains("snapmap_enabled"))
        config.snapmap_enabled = j["snapmap_enabled"].get<bool>();
    if (j.contains("skip_intro_enabled"))
        config.skip_intro_enabled = j["skip_intro_enabled"].get<bool>();
    if (j.contains("net_peerTimeoutInSeconds"))
        config.net_peerTimeoutInSeconds = j["net_peerTimeoutInSeconds"].get<int>();
    if (j.contains("net_peerTimeoutInSeconds_Lobby"))
        config.net_peerTimeoutInSeconds_Lobby = j["net_peerTimeoutInSeconds_Lobby"].get<int>();
    if (j.contains("net_connectTimeoutInSeconds"))
        config.net_connectTimeoutInSeconds = j["net_connectTimeoutInSeconds"].get<int>();
    if (j.contains("net_waitOnPeerToLoadTimer_Seconds"))
        config.net_waitOnPeerToLoadTimer_Seconds =
            j["net_waitOnPeerToLoadTimer_Seconds"].get<int>();
    if (j.contains("net_forceStartMatchTimeLimit"))
        config.net_forceStartMatchTimeLimit = j["net_forceStartMatchTimeLimit"].get<int>();
    if (j.contains("net_peerTimeoutDuringLoadMS"))
        config.net_peerTimeoutDuringLoadMS = j["net_peerTimeoutDuringLoadMS"].get<int>();

    if (j.contains("baseColor")) {
        auto arr = j["baseColor"];
        if (arr.size() == 4)
            config.baseColor = ImVec4(arr[0], arr[1], arr[2], arr[3]);
    }
    if (j.contains("baseHover")) {
        auto arr = j["baseHover"];
        if (arr.size() == 4)
            config.baseHover = ImVec4(arr[0], arr[1], arr[2], arr[3]);
    }
    if (j.contains("baseActive")) {
        auto arr = j["baseActive"];
        if (arr.size() == 4)
            config.baseActive = ImVec4(arr[0], arr[1], arr[2], arr[3]);
    }
}

Config load_config(const std::string& filepath) {
    Config config;
    std::ifstream file(filepath);
    if (file.is_open()) {
        try {
            json j;
            file >> j;
            from_json(j, config);
        } catch (const json::exception& e) {
            std::cerr << "Error parsing config: " << e.what() << std::endl;
        }
    }
    return config;
}

void save_config(const Config& config, const std::string& filepath) {
    std::ofstream file(filepath);
    if (file.is_open()) {
        ordered_json j;
        to_json(j, config);
        file << j.dump(2);
    }
}
