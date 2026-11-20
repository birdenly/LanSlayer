#pragma once
#include "imgui.h"
#include "imgui_internal.h"

#include <windows.h>

class UI {
public:
    static void DrawUI(HWND hwnd, float dpiScale);
};
