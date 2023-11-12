#include "window.hpp"
#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <random>

void Window::onEvent(SDL_Event const &event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? -1.0f : 1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 2.0f);
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

  m_model.loadObj(assetsPath + "sphere.obj");
  m_model.setupVAO(m_program);

  m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
  m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(0.5f));

  m_trianglesToDraw = m_model.getNumTriangles();

  // Setup stars
  for (auto &sphere : m_spheres) {
    std::uniform_real_distribution<float> distPosXY(-1.0f, 1.0f);
    std::uniform_real_distribution<float> distPosZ(-1.5f, 2.0f);
    sphere.m_position =
        glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine),
                  distPosZ(m_randomEngine));
  }
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(glm::radians(30.0f) * deltaTime);
  m_modelMatrix = glm::rotate(m_modelMatrix, m_angle, m_rotationAxis);

  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f + m_zoom),
                  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
  auto const aspect{gsl::narrow<float>(m_viewportSize.x) /
                    gsl::narrow<float>(m_viewportSize.y)};
  m_projMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 5.0f);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  auto const modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  auto const colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  // Set uniform variables for the current model
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White

  // render earth
  m_model.render(m_trianglesToDraw);
  
  // Render each sphere
  for (auto &sphere : m_spheres) {
    // Compute model matrix of the current star
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, sphere.m_position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

    abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // White

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

    m_model.render();
  }

  abcg::glUseProgram(0);
}

void Window::onPaintUI() { abcg::OpenGLWindow::onPaintUI(); }

void Window::onResize(glm::ivec2 const &size) { m_viewportSize = size; }

void Window::onDestroy() {
  m_model.destroy();
  abcg::glDeleteProgram(m_program);
}