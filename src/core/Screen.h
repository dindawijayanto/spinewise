#pragma once

#include "core/ScreenId.h"

class App;

// Basis untuk semua screen. Setiap frame App memanggil:
//   update(dt) -> render() -> drawUI()
// render() untuk OpenGL, drawUI() untuk panel ImGui.
class Screen
{
public:
    explicit Screen(App& app) : m_app(app) {}
    virtual ~Screen() = default;

    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    virtual const char* name() const = 0;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void update(float /*dt*/) {}
    virtual void render() {}
    virtual void drawUI() {}

protected:
    App& m_app;
};
