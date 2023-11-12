#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include "abcgOpenGL.hpp"
#include "model.hpp"
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

  Model m_model;
  int m_trianglesToDraw{};

  struct Sphere {
    glm::vec3 m_position{};
  };

  std::array<Sphere, 1> m_spheres;

  glm::vec3 m_rotationAxis{1.0f, 1.0f, 0.0f};
  float m_angle{};
  float m_zoom{1.0f};

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  GLuint m_program{};
};

#endif