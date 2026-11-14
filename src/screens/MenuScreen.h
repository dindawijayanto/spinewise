#pragma once

#include "core/Screen.h"

// Owner: Dinda (UI & Integration)
// Navigasi ke screen lain, disclaimer, About & Credits.
class MenuScreen : public Screen
{
public:
    using Screen::Screen;
    const char* name() const override { return "Menu"; }
    void drawUI() override;
};
