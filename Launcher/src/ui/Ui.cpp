#include "Ui.hpp"

#include "Config.hpp"
#include "Patcher.hpp"
#include "Utils.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

extern Config g_config;
namespace fs = std::filesystem;

static bool DrawDirectoryInput(HWND hwnd, const char* label, std::string& pathRef, float dpiScale,
                               float centerOffset, float fieldWidth) {
    bool pathChanged = false;

    ImGui::SetCursorPosX(centerOffset);
    ImGui::TextUnformatted(label);
    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth - 90 * dpiScale);

    char buf[512];
    strncpy_s(buf, pathRef.c_str(), sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    std::string inputId = std::string("##Path_") + label;
    if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf))) {
        pathRef = buf;
        pathChanged = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Browse", ImVec2(80 * dpiScale, 0))) {
        std::string chosen = Utils::Dialog::BrowseForDirectory(hwnd);
        if (!chosen.empty()) {
            pathRef = chosen;
            pathChanged = true;
        }
    }

    return pathChanged;
}

static void setupStyle(float dpiScale) {
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_Button] = g_config.baseColor;
    style.Colors[ImGuiCol_ButtonHovered] = g_config.baseHover;
    style.Colors[ImGuiCol_ButtonActive] = g_config.baseActive;
    style.Colors[ImGuiCol_CheckMark] = g_config.baseColor;
    style.Colors[ImGuiCol_FrameBgActive] = g_config.baseActive;
    style.Colors[ImGuiCol_FrameBgHovered] = g_config.baseHover;
    style.Colors[ImGuiCol_Header] = g_config.baseColor;
    style.Colors[ImGuiCol_HeaderHovered] = g_config.baseHover;
    style.Colors[ImGuiCol_HeaderActive] = g_config.baseActive;
    style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);

    style.WindowPadding = ImVec2(16 * dpiScale, 10 * dpiScale);
    style.ItemSpacing = ImVec2(10 * dpiScale, 6 * dpiScale);
    style.FramePadding = ImVec2(6 * dpiScale, 4 * dpiScale);
    style.WindowBorderSize = 0.0f;
    style.FrameRounding = 4.0f;
}

static void drawTitleBar(HWND hwnd, float dpiScale) {
    float h = 40.0f * dpiScale;
    ImGui::BeginChild("TitleBar", ImVec2(0, h), false);

    ImGui::SetCursorPos(ImVec2(15 * dpiScale, (h - ImGui::GetFontSize() * 1.2f) * 0.5f));
    ImGui::TextUnformatted("LanSlayer Launcher");

    float btnW = 40 * dpiScale, btnH = 26 * dpiScale, spacing = 12 * dpiScale,
          rightPad = 10 * dpiScale;
    float yOffset = (h - btnH) * 0.5f;
    ImGui::SetCursorPos(
        ImVec2(ImGui::GetWindowWidth() - (btnW * 3) - spacing * 2 - rightPad, yOffset));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1));
    if (ImGui::Button("_", ImVec2(btnW, btnH)))
        ::ShowWindow(hwnd, SW_MINIMIZE);
    ImGui::SameLine(0, spacing);
    if (ImGui::Button(::IsZoomed(hwnd) ? "[ ]" : "[]", ImVec2(btnW, btnH))) {
        if (::IsZoomed(hwnd))
            ::ShowWindow(hwnd, SW_RESTORE);
        else
            ::ShowWindow(hwnd, SW_MAXIMIZE);
    }
    ImGui::SameLine(0, spacing);
    if (ImGui::Button("X", ImVec2(btnW, btnH)))
        ::PostMessage(hwnd, WM_CLOSE, 0, 0);
    ImGui::PopStyleColor();

    if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (!ImGui::IsAnyItemActive()) {
            ::ReleaseCapture();
            ::SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
    }
    ImGui::EndChild();
}

static void drawTabBar(int& currentTab, float dpiScale) {
    ImGui::BeginChild("TabBar", ImVec2(0, 45.0f * dpiScale), false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::SetCursorPos(ImVec2(15.0f * dpiScale, 6.0f * dpiScale));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.0f * dpiScale, 8.0f * dpiScale));

    float tabWidth = 130.0f * dpiScale;
    ImVec4 activeColor = g_config.baseColor;
    ImVec4 inactiveColor = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);

    const char* tabs[] = {"Launcher", "Patcher", "Misc"};
    for (int i = 0; i < 3; ++i) {
        if (i > 0)
            ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, currentTab == i ? activeColor : inactiveColor);
        if (ImGui::Button(tabs[i], ImVec2(tabWidth, 0)))
            currentTab = i;
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static void drawStatusMessage(float dpiScale) {
    if (Utils::UI::Status::GetCurrent().text.empty() || Utils::UI::Status::GetCurrent().isExpired())
        return;

    float safeWidth = ImGui::GetContentRegionAvail().x;
    float centerOffset = (safeWidth - 400 * dpiScale) / 2.0f;
    ImGui::Dummy(ImVec2(0, 5 * dpiScale));

    auto elapsed = std::chrono::steady_clock::now() - Utils::UI::Status::GetCurrent().showTime;
    float elapsedSec =
        std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

    float alpha = 1.0f;
    if (elapsedSec > Utils::UI::Status::GetCurrent().duration - 0.5f) {
        alpha = (Utils::UI::Status::GetCurrent().duration - elapsedSec) * 2.0f;
    }

    ImVec4 textColor = Utils::UI::Status::GetCurrent().isError ? ImVec4(1.0f, 0.4f, 0.4f, alpha)
                                                               : ImVec4(0.4f, 1.0f, 0.4f, alpha);

    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    float textWidth = ImGui::CalcTextSize(Utils::UI::Status::GetCurrent().text.c_str()).x;
    float padding = (400 * dpiScale - textWidth) / 2.0f;

    ImGui::SetCursorPosX(centerOffset + padding);
    ImGui::Text("%s", Utils::UI::Status::GetCurrent().text.c_str());
    ImGui::PopStyleColor();
}

static void drawLauncherTab(HWND hwnd, float dpiScale) {
    float safeWidth = ImGui::GetContentRegionAvail().x;
    float fieldWidth = safeWidth * 0.85f;
    float centerOffset = (safeWidth - fieldWidth) / 2.0f;

    if (DrawDirectoryInput(hwnd, "Game Directory:", g_config.doom_game_path, dpiScale, centerOffset,
                           fieldWidth))
        save_config(g_config, Utils::GetConfigPath());

    ImGui::Dummy(ImVec2(0, 10 * dpiScale));

    ImGui::SetCursorPosX(centerOffset);
    ImGui::TextUnformatted("Renderer:");
    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth);
    const char* renderers[] = {"OpenGL", "Vulkan"};
    if (ImGui::Combo("##Renderer", &g_config.renderer, renderers, IM_ARRAYSIZE(renderers)))
        save_config(g_config, Utils::GetConfigPath());

    ImGui::Dummy(ImVec2(0, 15 * dpiScale));

    ImGui::SetCursorPosX(centerOffset);
    if (ImGui::Checkbox("XP Multiplier", &g_config.xp_multiplier_enabled))
        save_config(g_config, Utils::GetConfigPath());
    ImGui::SameLine();
    if (ImGui::Checkbox("UnlockAll", &g_config.unlock_all_enabled))
        save_config(g_config, Utils::GetConfigPath());
    ImGui::SameLine();
    if (ImGui::Checkbox("SnapMap", &g_config.snapmap_enabled))
        save_config(g_config, Utils::GetConfigPath());
    ImGui::SameLine();
    if (ImGui::Checkbox("Skip Intro", &g_config.skip_intro_enabled))
        save_config(g_config, Utils::GetConfigPath());
    ImGui::SameLine(0.0f, 10 * dpiScale);
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Skips startup videos");

    ImGui::BeginDisabled(!g_config.xp_multiplier_enabled);
    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth);
    if (ImGui::InputFloat("##XPMultiplier", &g_config.xp_multiplier, 0.1f, 1.0f, "%.2f")) {
        if (g_config.xp_multiplier < 0.0f)
            g_config.xp_multiplier = 0.0f;
        save_config(g_config, Utils::GetConfigPath());
    }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 10 * dpiScale));

    if (Utils::UI::CenteredInput("Custom Arguments:", g_config.custom_args, centerOffset,
                                 fieldWidth))
        save_config(g_config, Utils::GetConfigPath());

    ImGui::Dummy(ImVec2(0, 15 * dpiScale));

    ImGui::SetCursorPosX(centerOffset);
    ImGui::TextUnformatted("Network Timeouts:");
    ImGui::SameLine(0.0f, 10 * dpiScale);
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Amount of time it takes for various things to timeout");

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Peer Timeout", &g_config.net_peerTimeoutInSeconds)) {
        if (g_config.net_peerTimeoutInSeconds < 0)
            g_config.net_peerTimeoutInSeconds = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Peer Timeout Lobby", &g_config.net_peerTimeoutInSeconds_Lobby)) {
        if (g_config.net_peerTimeoutInSeconds_Lobby < 0)
            g_config.net_peerTimeoutInSeconds_Lobby = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Connect Timeout", &g_config.net_connectTimeoutInSeconds)) {
        if (g_config.net_connectTimeoutInSeconds < 0)
            g_config.net_connectTimeoutInSeconds = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Wait On Peer To Load Timer",
                        &g_config.net_waitOnPeerToLoadTimer_Seconds)) {
        if (g_config.net_waitOnPeerToLoadTimer_Seconds < 0)
            g_config.net_waitOnPeerToLoadTimer_Seconds = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Force Start Match Time Limit", &g_config.net_forceStartMatchTimeLimit)) {
        if (g_config.net_forceStartMatchTimeLimit < 0)
            g_config.net_forceStartMatchTimeLimit = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth / 2.0f);
    if (ImGui::InputInt("Peer Timeout During Load (ms)", &g_config.net_peerTimeoutDuringLoadMS)) {
        if (g_config.net_peerTimeoutDuringLoadMS < 0)
            g_config.net_peerTimeoutDuringLoadMS = 0;
        save_config(g_config, Utils::GetConfigPath());
    }

    ImGui::Dummy(ImVec2(0, 20 * dpiScale));

    if (Utils::UI::CenteredButton("Launch Game", 200 * dpiScale, dpiScale)) {
        std::error_code ec;
        fs::path gamePath(g_config.doom_game_path);
        fs::path gameDir = fs::is_directory(gamePath, ec) ? gamePath : gamePath.parent_path();
        const char* exeName = (g_config.renderer == 1) ? "DOOMx64vk.exe" : "DOOMx64.exe";
        fs::path exePath = gameDir / exeName;

        if (g_config.doom_game_path.empty() || !fs::is_directory(gameDir, ec)) {
            Utils::UI::Status::Show("Set a valid game directory first!", true);
        } else if (!fs::exists(exePath, ec)) {
            Utils::UI::Status::Show(std::string("Executable not found: ") + exeName, true);
        } else {
            std::string err;
            std::string args = Utils::Process::BuildArgs(g_config);
            if (Utils::Process::LaunchExe(exePath.string(), args, err))
                Utils::UI::Status::Show("Launching " + std::string(exeName) + "\nArgs: " + args);
            else
                Utils::UI::Status::Show("Failed to launch: " + err, true);
        }
    }

    drawStatusMessage(dpiScale);
}

static void drawPatcherTab(HWND hwnd, float dpiScale) {
    float safeWidth = ImGui::GetContentRegionAvail().x;
    float fieldWidth = safeWidth * 0.85f;
    float centerOffset = (safeWidth - fieldWidth) / 2.0f;

    if (DrawDirectoryInput(hwnd, "Game Directory:", g_config.doom_game_path, dpiScale, centerOffset,
                           fieldWidth))
        save_config(g_config, Utils::GetConfigPath());

    ImGui::Dummy(ImVec2(0, 8 * dpiScale));
    ImGui::SetCursorPosX(centerOffset);
    ImGui::TextDisabled("Patches all executables in the selected folder with a matching .xdelta in "
                        "patches/DOOM/xdelta/");
    ImGui::Dummy(ImVec2(0, 12 * dpiScale));

    std::error_code ec;
    fs::path gamePath(g_config.doom_game_path);
    fs::path gameDir = fs::is_directory(gamePath, ec) ? gamePath : gamePath.parent_path();
    bool hasDir = !g_config.doom_game_path.empty() && fs::is_directory(gameDir, ec);

    fs::path patchesDir(Utils::GetDOOMPatchesDir());
    bool hasPatches = false;
    if (fs::is_directory(patchesDir, ec)) {
        for (const auto& entry : fs::directory_iterator(patchesDir, ec)) {
            if (ec)
                break;
            if (entry.is_regular_file(ec) && entry.path().extension() == ".xdelta") {
                hasPatches = true;
                break;
            }
        }
    }

    std::string patchesDirStr = patchesDir.string();
    std::string gameDirStr = gameDir.string();
    bool isPatched = Patcher::IsDOOMPatched(gameDirStr, patchesDirStr);

    if (!hasPatches) {
        ImGui::SetCursorPosX(centerOffset);
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1),
                           "No .xdelta patches found in patches/DOOM/xdelta/");
    }

    if (isPatched) {
        ImGui::SetCursorPosX(centerOffset);
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "DOOM is already patched");

        if (Utils::UI::CenteredButton("RESTORE ORIGINAL DOOM", 200 * dpiScale, dpiScale)) {
            std::string err;
            if (Patcher::RestoreDOOM(gameDirStr, patchesDirStr, err))
                Utils::UI::Status::Show("DOOM executables restored to original");
            else
                Utils::UI::Status::Show("Error: " + err, true);
        }
    } else {
        if (!hasDir) {
            ImGui::SetCursorPosX(centerOffset);
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Set a valid DOOM game directory first!");
        } else {
            ImGui::SetCursorPosX(centerOffset);
            ImGui::Text("One click patch for all DOOM executables");
        }

        ImGui::BeginDisabled(!hasDir || !hasPatches);
        if (Utils::UI::CenteredButton("Auto Patch DOOM", 200 * dpiScale, dpiScale)) {
            std::string err;
            if (Patcher::AutoPatchDOOM(gameDirStr, patchesDirStr, err))
                Utils::UI::Status::Show("DOOM executables patched successfully!");
            else
                Utils::UI::Status::Show("Error: " + err, true);
        }
        ImGui::EndDisabled();
    }

    drawStatusMessage(dpiScale);
}

static void drawMiscTab(HWND hwnd, float dpiScale) {
    float safeWidth = ImGui::GetContentRegionAvail().x;

    ImGui::Dummy(ImVec2(0, 30 * dpiScale));
    ImGui::SetWindowFontScale(1.3f);
    ImGui::PushStyleColor(ImGuiCol_Text, g_config.baseColor);
    Utils::UI::CenteredText("Credits", safeWidth);
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 15 * dpiScale));

    const std::vector<std::string> credits = {"birden", "twig", "blueballoon"};
    float nameSpacing = 25.0f * dpiScale;
    float totalCreditsWidth = 0.0f;

    for (const auto& name : credits)
        totalCreditsWidth += ImGui::CalcTextSize(name.c_str()).x;
    if (credits.size() > 1)
        totalCreditsWidth += (nameSpacing * (credits.size() - 1));

    ImGui::SetCursorPosX((safeWidth - totalCreditsWidth) * 0.5f);
    for (size_t i = 0; i < credits.size(); i++) {
        if (i > 0)
            ImGui::SameLine(0, nameSpacing);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), credits[i].c_str());
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::Dummy(ImVec2(0, 40 * dpiScale));

    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 20 * dpiScale));

    ImGui::SetWindowFontScale(1.3f);
    ImGui::PushStyleColor(ImGuiCol_Text, g_config.baseColor);
    Utils::UI::CenteredText("UI Theme", safeWidth);
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Dummy(ImVec2(0, 15 * dpiScale));

    static const char* presetNames[] = {"Dark Green", "Red",    "Purple", "Blue",
                                        "Yellow",     "Orange", "Aqua"};
    static const char* displayNames[] = {"Dark Green", "Red",    "Purple", "Blue",
                                         "Yellow",     "Orange", "Aqua",   "Custom"};
    static int currentPreset = 0;

    float comboWidth = 200 * dpiScale;
    ImGui::SetCursorPosX((safeWidth - comboWidth) * 0.5f);
    ImGui::SetNextItemWidth(comboWidth);

    const char* previewValue =
        (currentPreset >= 0 && currentPreset < 7) ? displayNames[currentPreset] : displayNames[7];

    if (ImGui::BeginCombo("##Presets", previewValue)) {
        for (int i = 0; i < 7; i++) {
            bool isSelected = (currentPreset == i);
            if (ImGui::Selectable(presetNames[i], isSelected)) {
                currentPreset = i;
                switch (currentPreset) {
                case 0:
                    g_config.baseColor = ImVec4(0.0196f, 0.2431f, 0.1059f, 1.0f);
                    g_config.baseHover = ImVec4(0.0510f, 0.1922f, 0.0902f, 1.0f);
                    g_config.baseActive = ImVec4(0.0353f, 0.2706f, 0.1020f, 1.0f);
                    break;
                case 1:
                    g_config.baseColor = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
                    g_config.baseHover = ImVec4(0.7f, 0.1f, 0.1f, 1.0f);
                    g_config.baseActive = ImVec4(0.6f, 0.1f, 0.1f, 1.0f);
                    break;
                case 2:
                    g_config.baseColor = ImVec4(0.6f, 0.0f, 0.8f, 1.0f);
                    g_config.baseHover = ImVec4(0.5f, 0.0f, 0.7f, 1.0f);
                    g_config.baseActive = ImVec4(0.4f, 0.0f, 0.6f, 1.0f);
                    break;
                case 3:
                    g_config.baseColor = ImVec4(0.0f, 0.5f, 0.9f, 1.0f);
                    g_config.baseHover = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);
                    g_config.baseActive = ImVec4(0.0f, 0.3f, 0.7f, 1.0f);
                    break;
                case 4:
                    g_config.baseColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                    g_config.baseHover = ImVec4(0.9f, 0.7f, 0.0f, 1.0f);
                    g_config.baseActive = ImVec4(0.8f, 0.6f, 0.0f, 1.0f);
                    break;
                case 5:
                    g_config.baseColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
                    g_config.baseHover = ImVec4(0.9f, 0.4f, 0.0f, 1.0f);
                    g_config.baseActive = ImVec4(0.8f, 0.3f, 0.0f, 1.0f);
                    break;
                case 6:
                    g_config.baseColor = ImVec4(0.0f, 0.75f, 0.8f, 1.0f);
                    g_config.baseHover = ImVec4(0.0f, 0.65f, 0.7f, 1.0f);
                    g_config.baseActive = ImVec4(0.0f, 0.55f, 0.6f, 1.0f);
                    break;
                }

                setupStyle(dpiScale);
                save_config(g_config, Utils::GetConfigPath());
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Dummy(ImVec2(0, 25 * dpiScale));

    float pickerSize = 50.0f * dpiScale, spacing = 40.0f * dpiScale, textGap = 4.0f * dpiScale;
    float totalBlockWidth = (pickerSize * 3) + (spacing * 2);
    float startX = (safeWidth - totalBlockWidth) * 0.5f;
    float startY = ImGui::GetCursorPosY();

    auto DrawCenteredItem = [&](int index, const char* label, ImVec4& color, const char* id) {
        ImGui::PushID(id);
        ImGui::SetCursorPos(ImVec2(startX + (index * (pickerSize + spacing)), startY));
        ImGui::SetNextItemWidth(pickerSize);
        if (ImGui::ColorEdit3("", (float*)&color,
                              ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
            currentPreset = -1;
            setupStyle(dpiScale);
            save_config(g_config, Utils::GetConfigPath());
        }

        ImVec2 boxPos = ImGui::GetItemRectMin();
        ImVec2 boxSize = ImGui::GetItemRectSize();
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImVec2 textPos(boxPos.x + (boxSize.x * 0.5f) - (textSize.x * 0.5f),
                       boxPos.y - textSize.y - 6.0f * dpiScale);

        ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(180, 180, 180, 255), label);
        ImGui::PopID();
    };

    DrawCenteredItem(0, "Base", g_config.baseColor, "BaseColor");
    DrawCenteredItem(1, "Hover", g_config.baseHover, "HoverColor");
    DrawCenteredItem(2, "Active", g_config.baseActive, "ActiveColor");

    ImGui::SetCursorPosY(startY + ImGui::GetTextLineHeight() + textGap + ImGui::GetFrameHeight());
    ImGui::Dummy(ImVec2(0, 50 * dpiScale));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

#ifdef GIT_HASH
#ifdef GIT_DATE
    Utils::UI::CenteredText("Launcher Version: " + std::string(GIT_HASH) + " (" + GIT_DATE + ")",
                            safeWidth);
#else
    Utils::UI::CenteredText(GIT_HASH, safeWidth);
#endif
#else
    Utils::UI::CenteredText("unknown", safeWidth);
#endif

    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 10 * dpiScale));

    if (Utils::UI::CenteredButton("Source Code", 220 * dpiScale, dpiScale)) {
        ShellExecuteA(hwnd, "open", "https://github.com/birdenly/LanSlayer", NULL, NULL,
                      SW_SHOWNORMAL);
    }

    drawStatusMessage(dpiScale);
}

void UI::DrawUI(HWND hwnd, float dpiScale) {
    static int currentTab = 0;
    setupStyle(dpiScale);

    RECT rc;
    GetClientRect(hwnd, &rc);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)(rc.right - rc.left), (float)(rc.bottom - rc.top)));

    ImGui::Begin("Garden Gate", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);

    drawTitleBar(hwnd, dpiScale);
    drawTabBar(currentTab, dpiScale);

    ImGui::BeginChild("ContentArea", ImVec2(0, ImGui::GetContentRegionAvail().y), false);

    switch (currentTab) {
    case 0:
        drawLauncherTab(hwnd, dpiScale);
        break;
    case 1:
        drawPatcherTab(hwnd, dpiScale);
        break;
    case 2:
        drawMiscTab(hwnd, dpiScale);
        break;
    }

    ImGui::EndChild();
    ImGui::End();
}
