#pragma once

#include "core/Screen.h"
#include "core/ScreenId.h"

#include <array>
#include <memory>

struct GLFWwindow;
struct ImGuiContext;

class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    int run();

    void switchTo(ScreenId id);
    void quit();

    GLFWwindow* window() const { return m_window; }
    ImGuiContext* imguiContext() const;
    int framebufferWidth() const { return m_fbWidth; }
    int framebufferHeight() const { return m_fbHeight; }

private:
    bool init();
    void shutdown();
    void applyPendingScreen();

    GLFWwindow* m_window = nullptr;
    int m_fbWidth = 0;
    int m_fbHeight = 0;

    std::array<std::unique_ptr<Screen>, static_cast<size_t>(ScreenId::Count)> m_screens;
    Screen* m_current = nullptr;
    ScreenId m_pending = ScreenId::Menu;
    bool m_hasPending = false;
};