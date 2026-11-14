#include "screens/PostureLabScreen.h"

#include "core/App.h"
#include "ui/CommonUI.h"

#include <glad/gl.h>
#include <imgui.h>

void PostureLabScreen::onEnter()
{
    // TODO: load resource (mesh, shader) yang dibutuhkan screen ini
}

void PostureLabScreen::onExit()
{
}

void PostureLabScreen::update(float /*dt*/)
{
}

void PostureLabScreen::render()
{
    // Placeholder: warna latar berbeda per screen supaya perpindahan terlihat
    glClearColor(0.12f, 0.17f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostureLabScreen::drawUI()
{
    ui::drawTopBar(m_app, name());
}
