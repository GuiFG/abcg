#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "abcgOpenGL.hpp"

struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec2 texCoord{};

  friend bool operator==(Vertex const &, Vertex const &) = default;
};

class Model {
public:
  void loadDiffuseTexture(std::string_view path);
  void loadObj(std::string_view path, GLuint program, bool standardize = true);
  void render(glm::mat4 modelMatrix, glm::mat4 m_viewMatrix);
  void destroy() const;
  void update(float deltaTime);
  void setHorizontalSpeed(double horizontalSpeed);
  void computeModelMatrix();

  [[nodiscard]] glm::mat4 getModelMatrix() const { return m_modelMatrix; }


  [[nodiscard]] glm::vec4 getKa() const { return m_Ka; }
  [[nodiscard]] glm::vec4 getKd() const { return m_Kd; }
  [[nodiscard]] glm::vec4 getKs() const { return m_Ks; }
  [[nodiscard]] float getShininess() const { return m_shininess; }

  Model *satellite_of;
  float scale{1.0f};
  glm::vec3 position{0.0f};
  glm::vec4 color{1.0f};

  double radius;
  double mass;

  float rotation_angle{0.0f};
  float translation_speed{0.0f};

  Model(double pMass, double pRadius) {
    mass = pMass;
    radius = pRadius;
    position = {0.0f, 0.0f, 0.0f};
  }

  Model() {
  }

private:
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  float m_rotation_speed{1.0f};
  glm::mat4 m_rotation{1.0f};

  glm::vec3 m_axis{0.0f, 1.0f, 0.0f};
  glm::vec3 m_position{0.0f};
  glm::vec2 m_acceleration{0.0f};
  glm::vec3 m_speed{0.0f};
  bool m_collision{false};

  glm::mat4 m_modelMatrix{1.0f};
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};

  GLint m_modelMatrixLoc{};
  GLint m_normalMatrixLoc{};
  GLint m_KaLoc{};
  GLint m_KdLoc{};
  GLint m_KsLoc{};
  GLint m_shininessLoc{};

  bool m_hasNormals{false};
  bool m_hasTexCoords{false};

  glm::vec4 m_Ka{};
  glm::vec4 m_Kd{};
  glm::vec4 m_Ks{};
  float m_shininess{};
  GLuint m_diffuseTexture{};

  void computeNormals();
  void createBuffers(GLuint program);
  void setupVAO(GLuint program);
  void standardize();
  void updateSatellite(float deltaTime);
};

#endif