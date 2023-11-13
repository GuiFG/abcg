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

  [[nodiscard]] glm::mat4 getModelMatrix() const {
    return m_modelMatrix;
  }

  Sphere *satellite_of;
  float scale{1.0f};
  float orbit_radius{};
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

  void computeModelMatrix();
  void createBuffers(GLuint program);
  void setupVAO(GLuint program);
  void standardize();
};

#endif