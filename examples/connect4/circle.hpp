#ifndef CIRCLE_HPP_
#define CIRCLE_HPP_

#include <array>
#include <random>

#include "abcgOpenGL.hpp"
#include "gamedata.hpp"

class Circle {
public:
  void create(GLuint program, glm::vec4 color);
  void update(const GameData &gameData, const glm::vec2 &mousePosition, float scale, float DeltaTime);
  void paint(glm::vec2 translation, float scale);
  void paint(float scale);
  void destroy();
  bool isDropping();

private:
  GLuint m_program{};
  GLuint m_VAO{};
  GLuint m_VBOPositions{};
  GLuint m_VBOColors{};

  GLint m_scaleLoc{};
  GLint m_translationLoc{};

  glm::vec2 m_translation{};

  static int const m_sides { 100 };
public:
  glm::vec2 m_lastPosition{};
};

#endif