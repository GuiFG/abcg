#ifndef SPHERE_HPP_
#define SPHERE_HPP_

#include "abcgOpenGL.hpp"
#include "orbit.hpp"

struct Vertex {
  glm::vec3 position{};

  friend bool operator==(Vertex const &, Vertex const &) = default;
};

class Sphere {
public:
  void loadObj(std::string_view path, GLuint program, bool standardize = true);
  void render() const;
  void destroy() const;
  void update(float rot_speed, float trans_speed);
  void computeModelMatrix();

  [[nodiscard]] int getNumTriangles() const {
    return gsl::narrow<int>(m_indices.size()) / 3;
  }

  Sphere *satellite_of;

private:
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  float scale{1.0f};

  Orbit m_orbit;

  glm::vec3 position{0.0f};
  float m_orbit_radius{};
  float m_translation_angle{0.0f};
  float m_translation_speed{0.0f};
  float m_rotation_angle{0.0f};
  float m_rotation_speed{1.0f};
  glm::vec4 m_color{1.0f};

  glm::mat4 m_modelMatrix{1.0f};

  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};

  void createBuffers(GLuint program);
  void setupVAO(GLuint program);
  void standardize();
};

#endif