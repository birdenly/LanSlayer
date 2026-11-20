#include "Utils.hpp"

#include "Patcher.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string>
#include <vector>
#include <windows.h>

namespace fs = std::filesystem;

namespace Utils {
namespace Dialog {
std::string BrowseForDirectory(HWND hwnd) {
    std::string result;

    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool shouldUninit = (hrInit == S_OK || hrInit == S_FALSE);

    IFileOpenDialog* pDialog = nullptr;
    HRESULT hr =
        CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
    if (SUCCEEDED(hr)) {
        DWORD options = 0;
        if (SUCCEEDED(pDialog->GetOptions(&options))) {
            pDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM |
                                FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR);
        }

        pDialog->SetTitle(L"Select Game Directory");

        wchar_t cwd[MAX_PATH];
        DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
        if (cwdLen > 0 && cwdLen < MAX_PATH) {
            IShellItem* psiFolder = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(cwd, nullptr, IID_PPV_ARGS(&psiFolder)))) {
                pDialog->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }

        if (SUCCEEDED(pDialog->Show(hwnd))) {
            IShellItem* pItem = nullptr;
            if (SUCCEEDED(pDialog->GetResult(&pItem))) {
                PWSTR pathW = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pathW)) && pathW) {
                    int size =
                        WideCharToMultiByte(CP_ACP, 0, pathW, -1, nullptr, 0, nullptr, nullptr);
                    if (size > 0) {
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_ACP, 0, pathW, -1, result.data(), size, nullptr,
                                            nullptr);
                    }
                    CoTaskMemFree(pathW);
                }
                pItem->Release();
            }
        }

        pDialog->Release();
    }

    if (shouldUninit)
        CoUninitialize();

    return result;
}
} // namespace Dialog

namespace UI {
bool CenteredInput(const char* label, std::string& value, float centerOffset, float fieldWidth,
                   ImGuiInputTextFlags flags) {
    ImGui::SetCursorPosX(centerOffset);
    ImGui::TextUnformatted(label);
    ImGui::SetCursorPosX(centerOffset);
    ImGui::SetNextItemWidth(fieldWidth);

    char buf[1025];
    strncpy_s(buf, value.c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    bool changed = false;
    if (ImGui::InputText(std::string("##").append(label).c_str(), buf, sizeof(buf), flags)) {
        value = buf;
        changed = true;
    }
    return changed;
}

void CenteredText(const std::string& text, float windowWidth) {
    float textWidth = ImGui::CalcTextSize(text.c_str()).x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::TextUnformatted(text.c_str());
}

bool CenteredButton(const char* label, float width, float dpiScale) {
    float safeWidth = ImGui::GetContentRegionAvail().x;
    float offset = (safeWidth - width) / 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    return ImGui::Button(label, ImVec2(width, 38.0f * dpiScale));
}

namespace Status {
static StatusMessage currentStatus;

void Show(const std::string& text, bool isError, float duration) {
    currentStatus = StatusMessage(text, isError, duration);
}

const StatusMessage& GetCurrent() {
    return currentStatus;
}
} // namespace Status
} // namespace UI

namespace Process {
std::string BuildArgs(const Config& cfg) {
    std::string args;
    args += (cfg.renderer == 1) ? "+r_renderAPI 1" : "+r_renderAPI 0";

    if (cfg.snapmap_enabled) {
        if (!args.empty())
            args += ' ';
        args += "+com_gameMode 1 +com_gameType 1";
    }
    if (cfg.xp_multiplier_enabled) {
        if (!args.empty())
            args += ' ';
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%g", cfg.xp_multiplier);
        args += "+metrics_xpMultiplier ";
        args += buf;
    }
    if (cfg.skip_intro_enabled) {
        if (!args.empty())
            args += ' ';
        args += "+com_skipIntroVideo 1";
    }
    if (cfg.unlock_all_enabled) {
        if (!args.empty())
            args += ' ';
        args += "+dev_allowAllUnlocks 1";
    }

    static const char* defaultArgs[] = {
        "+net_dedicatedServerGate_Enable 0",
        "+net_peerTimeoutDisabled 1",
        "+com_logFile 2",
        "+menu_allowPrivateMatch 1",
        "+menu_startMatchAlwaysAllowed 1",
        "+gc_minPlayersInPrivateMatch 1",
        "+metrics_allowStatsAndXPInPrivateMatch 1",
        "+hud_showXPGainsInPrivateMatch 1",
    };

    for (const char* arg : defaultArgs) {
        if (!args.empty())
            args += ' ';
        args += arg;
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "+net_peerTimeoutInSeconds %d", cfg.net_peerTimeoutInSeconds);
    if (!args.empty())
        args += ' ';
    args += buf;

    std::snprintf(buf, sizeof(buf), "+net_peerTimeoutInSeconds_Lobby %d",
                  cfg.net_peerTimeoutInSeconds_Lobby);
    if (!args.empty())
        args += ' ';
    args += buf;

    std::snprintf(buf, sizeof(buf), "+net_connectTimeoutInSeconds %d",
                  cfg.net_connectTimeoutInSeconds);
    if (!args.empty())
        args += ' ';
    args += buf;

    std::snprintf(buf, sizeof(buf), "+net_waitOnPeerToLoadTimer_Seconds %d",
                  cfg.net_waitOnPeerToLoadTimer_Seconds);
    if (!args.empty())
        args += ' ';
    args += buf;

    std::snprintf(buf, sizeof(buf), "+net_forceStartMatchTimeLimit %d",
                  cfg.net_forceStartMatchTimeLimit);
    if (!args.empty())
        args += ' ';
    args += buf;

    std::snprintf(buf, sizeof(buf), "+net_peerTimeoutDuringLoadMS %d",
                  cfg.net_peerTimeoutDuringLoadMS);
    if (!args.empty())
        args += ' ';
    args += buf;

    if (!cfg.custom_args.empty()) {
        if (!args.empty())
            args += ' ';
        args += cfg.custom_args;
    }
    return args;
}

bool LaunchExe(const std::string& exePath, const std::string& args, std::string& errorMessage) {
    if (exePath.empty() || !fs::exists(exePath)) {
        errorMessage = "Executable not found";
        return false;
    }

    std::string cmdLine = "\"" + exePath + "\"";
    if (!args.empty())
        cmdLine += " " + args;

    std::vector<char> cmdBuffer(cmdLine.begin(), cmdLine.end());
    cmdBuffer.push_back('\0');

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {};

    BOOL ok = CreateProcessA(nullptr, cmdBuffer.data(), nullptr, nullptr, FALSE, 0, nullptr,
                             fs::path(exePath).parent_path().string().c_str(), &si, &pi);

    if (!ok) {
        errorMessage = "CreateProcess failed. Error: " + std::to_string(GetLastError());
        return false;
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

static std::vector<std::string> GetCommandLineArgs() {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;

    if (argvW) {
        for (int i = 0; i < argc; ++i) {
            int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
            std::string arg(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &arg[0], size, nullptr, nullptr);
            args.push_back(arg);
        }
        LocalFree(argvW);
    }
    return args;
}

bool handleCLI() {
    auto args = GetCommandLineArgs();
    if (args.size() < 3 || args[1] != "-patch")
        return false;

    std::ofstream log(GetLogPath(), std::ios::app);
    auto logMsg = [&](const std::string& msg) { log << msg << std::endl; };

    Config cfg = load_config(GetConfigPath());

    for (size_t i = 2; i < args.size(); ++i) {
        if (args[i] != "doom" && args[i] != "-all")
            continue;

        std::string gamePath = cfg.doom_game_path;
        if (i + 1 < args.size() && !args[i + 1].empty() && args[i + 1][0] != '-')
            gamePath = args[++i];

        if (gamePath.empty()) {
            logMsg("FAILED: no DOOM path provided or configured");
            continue;
        }

        fs::path gameDir =
            fs::is_directory(gamePath) ? fs::path(gamePath) : fs::path(gamePath).parent_path();
        logMsg("Patching DOOM in " + gameDir.string() + "...");
        std::string err;
        bool ok = Patcher::AutoPatchDOOM(gameDir.string(), GetDOOMPatchesDir(), err);
        logMsg(ok ? "SUCCESS" : "FAILED: " + err);
    }

    logMsg("========================================");
    return true;
}
} // namespace Process

namespace FileUtil {
bool copy(const std::string& src, const std::string& dst, std::string& err) {
    if (!::CopyFileA(src.c_str(), dst.c_str(), FALSE)) {
        err = "Failed to copy " + src + " to " + dst;
        return false;
    }
    return true;
}

bool move(const std::string& src, const std::string& dst, std::string& err) {
    if (!::MoveFileA(src.c_str(), dst.c_str())) {
        err = "Failed to move " + src + " to " + dst;
        return false;
    }
    return true;
}

bool move_replace(const std::string& src, const std::string& dst, std::string& err) {
    if (!::MoveFileExA(src.c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        err = "Failed to replace " + dst + " with " + src;
        return false;
    }
    return true;
}

void remove(const std::string& path) {
    ::DeleteFileA(path.c_str());
}
} // namespace FileUtil

std::string GetDOOMPatchesDir() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return (fs::path(path).parent_path() / "patches" / "DOOM" / "xdelta").string();
}

std::string GetConfigPath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    fs::path exePath(path);
    return (exePath.parent_path() / "config.json").string();
}

std::string GetLogPath() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    fs::path exePath(path);
    return (exePath.parent_path() / "LanSlayer_Launcher.log").string();
}
} // namespace Utils
