#include "screens/SkeletonExplorerScreen.h"

#include "core/App.h"
#include "ui/CommonUI.h"

#include <glad/gl.h>
#include <imgui.h>

void SkeletonExplorerScreen::onEnter()
{
    // TODO: load resource (mesh, shader) yang dibutuhkan screen ini
}

void SkeletonExplorerScreen::onExit()
{
}

void SkeletonExplorerScreen::update(float /*dt*/)
{
}

void SkeletonExplorerScreen::render()
{
    // Placeholder: warna latar berbeda per screen supaya perpindahan terlihat
    glClearColor(0.12f, 0.14f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SkeletonExplorerScreen::drawUI()
{
    ui::drawTopBar(m_app, name());
}
