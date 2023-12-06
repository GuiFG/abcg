#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "skybox.hpp"
#include <array>
#include <random>

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;
  Model createSphere(float horizontalSpeed = 0.0f);

private:
  std::default_random_engine m_randomEngine;
  glm::ivec2 m_viewportSize{};

  Model m_planet;
  Model m_cannon_model;
  Model m_satellite_model;
  Skybox m_skybox;

  Camera m_camera;
  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};
  float m_verticalSpeed{};

  float m_horizontal_speed{0.0f};
  float horizontal_speed{0.0f};
  int m_satellites_total{0};

  std::vector<Model> m_satellites;
  
  GLuint m_program{};
  GLuint m_skybox_program{};

  // Mapping mode
  // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
  int m_mappingMode{};

  // Light and material properties
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
};

#endif