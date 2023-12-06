#ifndef SKYBOX_HPP_
#define SKYBOX_HPP_

#include "abcgOpenGL.hpp"

struct VertexSky {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec2 texCoord{};
  
  friend bool operator==(VertexSky const &, VertexSky const &) = default;
};

class Skybox {
public:
  void createBuffers(GLuint program);
  void paint();
  void destroy();
  void computeModelMatrix();
  void render() const;
  void generateUVSphere(int stacks, int slices);

  std::vector<VertexSky> m_vertices;
  std::vector<GLuint> m_indices;

  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};

  std::string texture_path{""};
  GLuint m_diffuseTexture{};

  glm::vec4 color{};
  float scale{1.0f};

  glm::vec3 position{0.0f};
  glm::mat4 modelMatrix{1.0f};

  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};
};

#endif