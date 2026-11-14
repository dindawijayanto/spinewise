#include "screens/SpineLabScreen.h"

#include "core/App.h"
#include "ui/CommonUI.h"

#include <glad/gl.h>
#include <imgui.h>

void SpineLabScreen::onEnter()
{
    // TODO: load resource (mesh, shader) yang dibutuhkan screen ini
}

void SpineLabScreen::onExit()
{
}

void SpineLabScreen::update(float /*dt*/)
{
}

void SpineLabScreen::render()
{
    // Placeholder: warna latar berbeda per screen supaya perpindahan terlihat
    glClearColor(0.14f, 0.12f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SpineLabScreen::drawUI()
{
    ui::drawTopBar(m_app, name());
}
