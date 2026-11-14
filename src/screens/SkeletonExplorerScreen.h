#pragma once

#include "core/Screen.h"

// Owner: Rafiq (Anatomy & Animation)
// Render skeleton OBJ, orbit camera, highlight region spine.
class SkeletonExplorerScreen : public Screen
{
public:
    using Screen::Screen;
    const char* name() const override { return "Skeleton Explorer"; }

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;
};
