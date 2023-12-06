#include "window.hpp"
#include <SDL_keycode.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <pthread.h>
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

    if (event.key.keysym.sym == SDLK_RETURN) {
      auto sphere = createSphere(m_horizontal_speed);
      m_satellites.push_back(sphere);
    }
  }

  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP)
      m_verticalSpeed = 1.0f;
    if (event.key.keysym.sym == SDLK_DOWN)
      m_verticalSpeed = -1.0f;
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
    if ((event.key.keysym.sym == SDLK_UP) && m_verticalSpeed > 0)
      m_verticalSpeed = 0.0f;
    if ((event.key.keysym.sym == SDLK_DOWN) && m_verticalSpeed < 0)
      m_verticalSpeed = 0.0f;
  }

  if (event.type == SDL_MOUSEWHEEL) {
    m_camera.mouseScroll(event.wheel.preciseY);
  }
}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  m_program = abcg::createOpenGLProgram(
      {{.source = assetsPath + "shaders/blinnphong.vert",
        .stage = abcg::ShaderStage::Vertex},
       {.source = assetsPath + "shaders/blinnphong.frag",
        .stage = abcg::ShaderStage::Fragment}});


  m_skybox_program =
      abcg::createOpenGLProgram({{.source = assetsPath + "shaders/skybox.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "shaders/skybox.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  m_planet.mass = 100.0f;
  m_planet.radius = 1.0f;
  m_planet.scale = 2.0f;
  m_planet.satellite_of = nullptr;

  m_cannon_model.position = glm::vec3{-0.5f, m_planet.radius + 0.3f, 0.0f};
  m_cannon_model.scale = 0.5f;
  m_cannon_model.rotation_angle = -1.5f;
  m_cannon_model.computeModelMatrix();

  auto satellite = createSphere();
  m_satellites.push_back(satellite);

  m_mappingMode = 3; // "From mesh" option
  m_planet.loadDiffuseTexture(assetsPath + "maps/earth.png");
  m_planet.loadObj(assetsPath + "earth.obj", m_program);
  
  m_cannon_model.loadDiffuseTexture(assetsPath + "maps/cannon.png");
  m_cannon_model.loadObj(assetsPath + "cannon.obj", m_program);

  m_satellite_model.loadDiffuseTexture(assetsPath + "maps/cannonball.png");
  m_satellite_model.loadObj(assetsPath + "cannonball.obj", m_program);

  m_skybox.texture_path = assetsPath + "maps/sky.jpg";
  m_skybox.createBuffers(m_skybox_program);
}

Model Window::createSphere(float horizontalSpeed) {
  Model sphere(1.0f, 0.2f);
  sphere.satellite_of = &m_planet;
  sphere.scale = 0.2f;
  sphere.setHorizontalSpeed(horizontalSpeed);

  std::uniform_real_distribution rd(0.0f, 1.0f);
  sphere.color = {rd(m_randomEngine), rd(m_randomEngine), rd(m_randomEngine),
                  rd(m_randomEngine)};

  if (sphere.satellite_of) {
    // glm::vec3{0.0f, m_cannon_model.position.y + 10.0f, 0.0f};
    sphere.position = glm::vec3{
        0.0f, m_cannon_model.position.y + 0.1f, 0.0f};
  }

  m_satellites_total += 1;

  return sphere;
}

void Window::onUpdate() {
  auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

  m_planet.update(deltaTime);
  for (auto &satellite : m_satellites) {
    satellite.update(deltaTime);
  }

  // Update LookAt camera
  m_camera.dolly(m_dollySpeed * deltaTime);
  m_camera.truck(m_truckSpeed * deltaTime);
  m_camera.pan(m_panSpeed * deltaTime);
  m_camera.vertical(m_verticalSpeed * deltaTime);
}

void Window::onPaint() {
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};
  auto const IaLoc{abcg::glGetUniformLocation(m_program, "Ia")};
  auto const IdLoc{abcg::glGetUniformLocation(m_program, "Id")};
  auto const IsLoc{abcg::glGetUniformLocation(m_program, "Is")};
  
  auto const diffuseTexLoc{abcg::glGetUniformLocation(m_program, "diffuseTex")};
  auto const mappingModeLoc{abcg::glGetUniformLocation(m_program, "mappingMode")};

  // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);

  abcg::glUniform4fv(IaLoc, 1, &m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_Is.x);
 
  auto const lightDirLoc = abcg::glGetUniformLocation(m_program, "lightDirWorldSpace");
  abcg::glUniform4fv(lightDirLoc, 1, &m_lightDir.x);

  m_planet.render(m_planet.getModelMatrix(), m_camera.getViewMatrix());
  m_cannon_model.render(m_cannon_model.getModelMatrix(), m_camera.getViewMatrix());

  for (auto &satellite : m_satellites) {
    m_satellite_model.render(satellite.getModelMatrix(), m_camera.getViewMatrix());
  }

  abcg::glUseProgram(0);

  abcg::glUseProgram(m_skybox_program);

  // Get location of uniform variables
  auto const skyViewMatrixLoc{abcg::glGetUniformLocation(m_skybox_program, "viewMatrix")};
  auto const skyProjMatrixLoc{abcg::glGetUniformLocation(m_skybox_program, "projMatrix")};

  abcg::glUniformMatrix4fv(skyViewMatrixLoc, 1, GL_FALSE, &m_camera.getViewMatrix()[0][0]);
  abcg::glUniformMatrix4fv(skyProjMatrixLoc, 1, GL_FALSE, &m_camera.getProjMatrix()[0][0]);

  auto const skydiffuseTexLoc{abcg::glGetUniformLocation(m_skybox_program, "diffuseTex")};
  
  abcg::glUniform1i(skydiffuseTexLoc, 0);

  m_skybox.render();

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  ImGui::SetNextWindowPos(ImVec2(5, 50));
  ImGui::SetNextWindowSize(ImVec2(350, -1));
  ImGui::SetNextWindowBgAlpha(0.7f);
  ImGui::Begin("Controles de Movimento", nullptr, ImGuiWindowFlags_NoResize);

  {
    ImGui::PushItemWidth(-1);
    ImGui::PushID(1);
    ImGui::SliderFloat("", &horizontal_speed, 0.0f, 100.0f,
                       "%.1fx Velocidade horizontal");
    ImGui::PopID();
    ImGui::PopItemWidth();

    m_horizontal_speed = gsl::narrow_cast<float>(horizontal_speed / 25);
  }


  std::string total = "Quantidade de projeteis: " + std::to_string(m_satellites_total) ;
  ImGui::Text( total.c_str());

  ImGui::End();
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_camera.computeProjectionMatrix(size);
}

void Window::onDestroy() {
  m_planet.destroy();
  m_cannon_model.destroy();
  m_satellite_model.destroy();

  abcg::glDeleteProgram(m_program);
}