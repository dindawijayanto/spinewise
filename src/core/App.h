#pragma once

#include "core/Screen.h"
#include "core/ScreenId.h"

#include <array>
#include <memory>

struct GLFWwindow;

class App
{
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    // Jalankan aplikasi. Return 0 kalau sukses (dipakai sebagai exit code).
    int run();

    // Pindah screen. Perpindahan dilakukan di awal frame berikutnya
    // supaya aman dipanggil dari dalam update()/drawUI().
    void switchTo(ScreenId id);
    void quit();

    GLFWwindow* window() const { return m_window; }
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
