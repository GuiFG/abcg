#include "window.hpp"
#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <random>
#include <variant>

void Window::onEvent(SDL_Event const &event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_w)
      m_dollySpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_s)
      m_dollySpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_panSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_panSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_q)
      m_truckSpeed = -1.0f;
    if (event.key.keysym.sym == SDLK_e)
      m_truckSpeed = 1.0f;
  }

  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP)
      m_pedestalSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_DOWN)
      m_pedestalSpeed = -1.0f;
  }

  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) &&
        m_dollySpeed > 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) &&
        m_dollySpeed < 0)
      m_dollySpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) &&
        m_panSpeed < 0)
      m_panSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_RIGHT ||
         event.key.keysym.sym == SDLK_d) &&
        m_panSpeed > 0)
      m_panSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_q && m_truckSpeed < 0)
      m_truckSpeed = 0.0f;
    if (event.key.keysym.sym == SDLK_e && m_truckSpeed > 0)
      m_truckSpeed = 0.0f;
  }

  if (event.type == SDL_KEYUP) {
    if ((event.key.keysym.sym == SDLK_UP) && m_pedestalSpeed > 0)
      m_pedestalSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN) && m_pedestalSpeed < 0)
      m_pedestalSpeed = 0.0f;
  }

  if (event.type == SDL_MOUSEMOTION) {
    m_camera.mouseMove(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONDOWN &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_camera.mousePress(mousePosition);
  }
  if (event.type == SDL_MOUSEBUTTONUP &&
      event.button.button == SDL_BUTTON_LEFT) {
    m_camera.mouseRelease(mousePosition);
  }

  if (event.type == SDL_MOUSEWHEEL) {
    m_camera.mouseScroll(event.wheel.preciseY);
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  m_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "sphere.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "sphere.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  int i = 0;
  for (auto &sphere : m_spheres) {
    if (i == 0) {
      sphere.satellite_of = nullptr;
      sphere.color = {0.0f, 0.0f, 1.0f, 1.0f};
    } else {
      sphere.satellite_of = &m_spheres[0];
      sphere.scale = 0.3f;
    }

    std::uniform_real_distribution rd(-0.0f, 1.0f);
    sphere.color = {rd(m_randomEngine), rd(m_randomEngine), rd(m_randomEngine),
                    rd(m_randomEngine)};

    std::uniform_real_distribution<float> distRadius(1.0f, 5.0f);
    sphere.orbit_radius = distRadius(m_randomEngine);

    sphere.z_index = sphere.orbit_radius > 3.0f;

    std::uniform_real_distribution<float> distTrans(0.1f, 1.0f);
    sphere.translation_reduce = distTrans(m_randomEngine);

    if (sphere.satellite_of) {
      sphere.position = sphere.satellite_of->position +
                        glm::vec3{sphere.orbit_radius, 0.0f, 0.0f};
    }

    i += 1;
  }

  m_model.loadObj(assetsPath + "sphere.obj", m_program);
}
void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  for (auto &sphere : m_spheres) {
    sphere.update(m_rotation_speed * deltaTime,
                  m_translation_speed * deltaTime);
  }

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
  m_camera.pedestal(m_pedestalSpeed * deltaTime);
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};

  // Set uniform variables for viewMatrix and projMatrix
  // These matrices are used for every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE,
                           &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE,
                           &m_camera.getProjMatrix()[0][0]);

  for (auto &sphere : m_spheres) {
    m_model.render(sphere.getModelMatrix(), sphere.color);
  }

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  ImGui::SetNextWindowPos(ImVec2(5, 100));
  ImGui::SetNextWindowSize(ImVec2(300, -1));
  ImGui::SetNextWindowBgAlpha(0.7f);
  ImGui::Begin("Controles de Movimento", nullptr, ImGuiWindowFlags_NoResize);

  {
    ImGui::PushItemWidth(-1);
    ImGui::PushID(1);
    ImGui::SliderFloat("", &m_translation_speed, 0.0f, 40.0f,
                       "%.1fx Vel. Translação");
    ImGui::PopID();
    ImGui::PopItemWidth();
  }

  ImGui::End();
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() {
  m_model.destroy();
  abcg::glDeleteProgram(m_program);
}