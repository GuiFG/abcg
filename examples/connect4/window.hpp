#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"
#include "circle.hpp"
#include "gamedata.hpp"

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
  glm::ivec2 m_viewportSize{};
  float m_scale;

  GLuint m_program{};
  Circle m_circle;
  Circle m_circleRed;
  Circle m_circleYellow;
  glm::ivec2 m_mousePosition;
  GameData m_gameData;

  ImFont *m_font{};
  abcg::Timer m_restartWaitTimer;
 
  void setupModel();
  glm::vec2 getMousePositionViewPort();
  glm::vec2 getCirclePositionViewPort(glm::vec2 index);
  int getNextRowBoard(int col);
  void checkEndCondition();
  void restart();
};

#endif