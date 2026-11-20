#include "Launcher.hpp"

#include "Renderer.hpp"
#include "config/Config.hpp"
#include "utils/Utils.hpp"

Config g_config;

int Launcher::Run() {
    if (Utils::Process::handleCLI())
        return 0;

    g_config = load_config(Utils::GetConfigPath());

    Renderer::Run();
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return Launcher::Run();
}