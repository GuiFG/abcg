#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"
#include "sphere.hpp"
#include "camera.hpp"
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

private:
  std::default_random_engine m_randomEngine;
  glm::ivec2 m_viewportSize{};

  Sphere m_model;

  Camera m_camera;
  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};
  float m_verticalSpeed{};

  float m_rotation_speed{1.0f};
  float m_translation_speed{2.0f};

  std::array<Sphere, 6> m_spheres;
  

  GLuint m_program{};
};

#endif