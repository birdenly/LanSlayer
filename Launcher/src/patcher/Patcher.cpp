#include "Patcher.hpp"

#include "Utils.hpp"
#include "xdelta3_wrapper.h"

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

constexpr const char* BACKUP = ".bak";
constexpr const char* PATCHED = ".patched";

namespace {
struct PatchTask {
    fs::path target;
    fs::path patchFile;
};

// Derive each target from the patch file name: DOOMx64.exe.xdelta -> DOOMx64.exe
std::vector<PatchTask> CollectPatchTasks(const fs::path& gameDir, const fs::path& patchesDir) {
    std::vector<PatchTask> tasks;
    std::error_code ec;
    if (!fs::exists(patchesDir) || !fs::is_directory(patchesDir))
        return tasks;

    for (const auto& entry : fs::directory_iterator(patchesDir, ec)) {
        if (ec || !entry.is_regular_file(ec) || entry.path().extension() != ".xdelta")
            continue;

        std::string targetName = entry.path().stem().string();
        if (!targetName.empty())
            tasks.push_back({gameDir / targetName, entry.path()});
    }
    return tasks;
}

bool run_xdelta(const std::string& original, const std::string& patch, const std::string& output,
                std::string& err) {
    std::vector<std::string> params = {"-d", "-f", "-s", original, patch, output};
    int result = xd3_main_exec(params);
    if (result != 0) {
        std::string messages = xd3_messages();
        err =
            "xdelta3 failed: " +
            (messages.empty() ? "Unknown error (code: " + std::to_string(result) + ")" : messages);
        return false;
    }
    return true;
}

bool XdeltaPatch(const fs::path& target, const fs::path& patchFile, std::string& err) {
    if (!fs::exists(target) || !fs::exists(patchFile)) {
        err = !fs::exists(target) ? "Game executable not found" : "Patch file not found";
        return false;
    }

    std::string backupPath = target.string() + BACKUP;
    std::string tempPath = target.string() + PATCHED;

    if (!Utils::FileUtil::copy(target.string(), backupPath, err))
        return false;

    if (!run_xdelta(target.string(), patchFile.string(), tempPath, err)) {
        Utils::FileUtil::remove(backupPath); // Rollback backup on fail
        return false;
    }

    if (!Utils::FileUtil::move_replace(tempPath, target.string(), err)) {
        Utils::FileUtil::remove(backupPath);
        return false;
    }
    return true;
}

bool RestoreFromExtension(const fs::path& target, const std::string& ext, std::string& err) {
    std::string backup = target.string() + ext;
    if (!fs::exists(backup))
        return true;

    if (fs::exists(target))
        Utils::FileUtil::remove(target.string());
    return Utils::FileUtil::move_replace(backup, target.string(), err);
}
} // namespace

bool Patcher::IsDOOMPatched(const std::string& gameDir, const std::string& patchesDir) {
    if (gameDir.empty() || !fs::exists(gameDir))
        return false;

    auto tasks = CollectPatchTasks(gameDir, patchesDir);
    if (tasks.empty())
        return false;

    for (const auto& task : tasks) {
        if (!fs::exists(task.target))
            continue;
        if (!fs::exists(task.target.string() + BACKUP))
            return false;
    }
    return true;
}

bool Patcher::AutoPatchDOOM(const std::string& gameDir, const std::string& patchesDir,
                            std::string& errorMessage) {
    if (gameDir.empty() || !fs::exists(gameDir)) {
        errorMessage = "Game directory not found";
        return false;
    }

    auto tasks = CollectPatchTasks(gameDir, patchesDir);
    if (tasks.empty()) {
        errorMessage = "No .xdelta patch files found";
        return false;
    }

    bool patchedAny = false;
    for (const auto& task : tasks) {
        if (!fs::exists(task.target))
            continue;
        if (fs::exists(task.target.string() + BACKUP)) {
            patchedAny = true; // Already patched, leave it alone
            continue;
        }
        if (!XdeltaPatch(task.target, task.patchFile, errorMessage))
            return false;
        patchedAny = true;
    }

    if (!patchedAny) {
        errorMessage = "No DOOM executables found to patch";
        return false;
    }
    return true;
}

bool Patcher::RestoreDOOM(const std::string& gameDir, const std::string& patchesDir,
                          std::string& errorMessage) {
    if (gameDir.empty() || !fs::exists(gameDir)) {
        errorMessage = "Game directory not found";
        return false;
    }

    bool restoredAny = false;
    for (const auto& task : CollectPatchTasks(gameDir, patchesDir)) {
        if (!fs::exists(task.target.string() + BACKUP))
            continue;
        if (!RestoreFromExtension(task.target, BACKUP, errorMessage))
            return false;
        restoredAny = true;
    }

    if (!restoredAny) {
        errorMessage = "No backups found";
        return false;
    }
    return true;
}

bool Patcher::ApplyPatch(const std::string& originalFile, const std::string& patchFile,
                         const std::string& outputFile, std::string& errorMessage) {
    if (!fs::exists(originalFile)) {
        errorMessage = "Original file not found";
        return false;
    }
    if (!fs::exists(patchFile)) {
        errorMessage = "Patch file not found";
        return false;
    }

    return run_xdelta(originalFile, patchFile, outputFile, errorMessage);
}
