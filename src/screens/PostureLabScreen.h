#pragma once

#include "core/Screen.h"

// Owner: Rafiq (Anatomy & Animation)
// Figur prosedural (bahu, pelvis, spine) + animasi keyframe.
class PostureLabScreen : public Screen
{
public:
    using Screen::Screen;
    const char* name() const override { return "Posture Lab"; }

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;
};
