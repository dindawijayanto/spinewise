#include "core/App.h"

#include "screens/MenuScreen.h"
#include "screens/PostureLabScreen.h"
#include "screens/SkeletonExplorerScreen.h"
#include "screens/SpineLabScreen.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <string>

namespace
{
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr const char* kWindowTitle = "SPINEWISE";
constexpr const char* kGlslVersion = "#version 330 core";

void glfwErrorCallback(int code, const char* description)
{
    std::fprintf(stderr, "[GLFW] error %d: %s\n", code, description);
}
} // namespace

App::App() = default;

App::~App()
{
    shutdown();
}

bool App::init()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
    {
        std::fprintf(stderr, "Gagal inisialisasi GLFW\n");
        return false;
    }

    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

    m_window = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle, nullptr, nullptr);
    if (!m_window)
    {
        std::fprintf(stderr, "Gagal membuat window. Pastikan GPU mendukung OpenGL 3.3.\n");
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::fprintf(stderr, "Gagal memuat fungsi OpenGL (GLAD)\n");
        return false;
    }
    std::printf("OpenGL %s | %s\n",
                reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // jangan tulis imgui.ini di samping .exe
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(kGlslVersion);

    // Screen
    m_screens[static_cast<size_t>(ScreenId::Menu)] = std::make_unique<MenuScreen>(*this);
    m_screens[static_cast<size_t>(ScreenId::SkeletonExplorer)] = std::make_unique<SkeletonExplorerScreen>(*this);
    m_screens[static_cast<size_t>(ScreenId::SpineLab)] = std::make_unique<SpineLabScreen>(*this);
    m_screens[static_cast<size_t>(ScreenId::PostureLab)] = std::make_unique<PostureLabScreen>(*this);

    switchTo(ScreenId::Menu);
    return true;
}

int App::run()
{
    if (!init())
    {
        shutdown();
        return 1;
    }

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        applyPendingScreen();

        // Jangan render saat window di-minimize (framebuffer 0x0)
        glfwGetFramebufferSize(m_window, &m_fbWidth, &m_fbHeight);
        if (m_fbWidth == 0 || m_fbHeight == 0)
        {
            glfwWaitEvents();
            // Reset waktu supaya dt frame berikutnya tidak berisi seluruh durasi minimize
            lastTime = glfwGetTime();
            continue;
        }

        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - lastTime);
        lastTime = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glViewport(0, 0, m_fbWidth, m_fbHeight);
        glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_current)
        {
            m_current->update(dt);
            m_current->render();
            m_current->drawUI();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    shutdown();
    return 0;
}

void App::switchTo(ScreenId id)
{
    m_pending = id;
    m_hasPending = true;
}

void App::quit()
{
    if (m_window)
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void App::applyPendingScreen()
{
    if (!m_hasPending)
        return;
    m_hasPending = false;

    Screen* next = m_screens[static_cast<size_t>(m_pending)].get();
    if (next == m_current)
        return;

    if (m_current)
        m_current->onExit();
    m_current = next;
    if (m_current)
    {
        m_current->onEnter();
        glfwSetWindowTitle(m_window, (std::string(kWindowTitle) + " - " + m_current->name()).c_str());
    }
}

void App::shutdown()
{
    if (m_current)
    {
        m_current->onExit();
        m_current = nullptr;
    }
    // Screen bisa memegang resource OpenGL, hapus sebelum context dihancurkan
    for (auto& screen : m_screens)
        screen.reset();

    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}
