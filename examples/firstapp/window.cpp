#include "window.hpp"
#include "abcgOpenGLWindow.hpp"

void Window::onCreate() {
    auto const &windowSettings(getWindowSettings());
    fmt::print("Initial window size: {}x{}\n", windowSettings.width, windowSettings.height);
}

void Window::onPaint() {
    abcg::glClearColor(m_clearColor.at(0), m_clearColor.at(1), m_clearColor.at(2), m_clearColor.at(3));

    abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onPaintUI() {
    abcg::OpenGLWindow::onPaintUI();

    {
        ImGui::Begin("Hello, First App!");

        auto const &windowSettings(getWindowSettings());
        ImGui::Text("Currente window size: %dx%d (in windowed mode)", windowSettings.width, windowSettings.height);

        static float f{};
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

        ImGui::ColorEdit3("clear color", m_clearColor.data());

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
}