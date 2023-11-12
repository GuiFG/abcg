#ifndef SPHERE_HPP_
#define SPHERE_HPP_

#include "abcgOpenGL.hpp"

struct Vertex {
  glm::vec3 position{};

  friend bool operator==(Vertex const &, Vertex const &) = default;
};

class Sphere {
public:
  void loadObj(std::string_view path, GLuint program, bool standardize = true);
  void render(glm::mat4 modelMatrix, glm::vec4 pColor) const;
  void destroy() const;
  void update(float rot_speed, float trans_speed);
  void computeModelMatrix();

  [[nodiscard]] int getNumTriangles() const {
    return gsl::narrow<int>(m_indices.size()) / 3;
  }

  [[nodiscard]] glm::mat4 getModelMatrix() const {
    return m_modelMatrix;
  }

  Sphere *satellite_of;
  float scale{1.0f};
  float orbit_radius{};
  float mass{1.0f};
  float speed_x{0.5f};
  glm::vec3 position{0.0f};
  glm::vec4 color{1.0f};
  bool z_index = true;
  float translation_reduce;

private:
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  float m_translation_angle{0.0f};
  float m_translation_speed{0.1f};
  float m_rotation_angle{0.0f};
  float m_rotation_speed{1.0f};
  glm::vec2 m_speed{0.5f, 0.0f};

  glm::mat4 m_modelMatrix{1.0f};

  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};

  bool m_collision {false};
  float m_distance {0.0f};

  void createBuffers(GLuint program);
  void setupVAO(GLuint program);
  void standardize();
  void updateSpeed();
  float getDistance(float x1, float y1, float x2, float y2);
};

#endif