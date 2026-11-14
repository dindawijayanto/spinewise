#pragma once

#include "core/Screen.h"

// Owner: Diny (Spine) + Dealova (Camera/Render)
// Panel input SpineParams, spine prosedural, deformasi C/S, mode comparison.
class SpineLabScreen : public Screen
{
public:
    using Screen::Screen;
    const char* name() const override { return "Spine Lab"; }

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;
};
