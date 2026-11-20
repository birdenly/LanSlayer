#pragma once
#include <string>

class Patcher {
public:
    static bool IsDOOMPatched(const std::string& gameDir, const std::string& patchesDir);
    static bool AutoPatchDOOM(const std::string& gameDir, const std::string& patchesDir,
                              std::string& errorMessage);
    static bool RestoreDOOM(const std::string& gameDir, const std::string& patchesDir,
                            std::string& errorMessage);

    static bool ApplyPatch(const std::string& originalFile, const std::string& patchFile,
                           const std::string& outputFile, std::string& errorMessage);
};
