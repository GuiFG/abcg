#include "sphere.hpp"

#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};

void Sphere::createBuffers(GLuint program) {
  // Delete previous buffers
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);

  // VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER,
                     sizeof(m_vertices.at(0)) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices.at(0)) * m_indices.size(),
                     m_indices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  m_modelMatrixLoc = abcg::glGetUniformLocation(program, "modelMatrix");
  m_colorLoc = abcg::glGetUniformLocation(program, "color");
}

void Sphere::loadObj(std::string_view path, GLuint program, bool standardize) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load Sphere {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load Sphere {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  // Loop over shapes
  for (auto const &shape : shapes) {
    // Loop over indices
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      // Access to vertex
      auto const index{shape.mesh.indices.at(offset)};

      // Vertex position
      auto const startIndex{3 * index.vertex_index};
      auto const vx{attrib.vertices.at(startIndex + 0)};
      auto const vy{attrib.vertices.at(startIndex + 1)};
      auto const vz{attrib.vertices.at(startIndex + 2)};

      Vertex const vertex{.position = {vx, vy, vz}};

      // If hash doesn't contain this vertex
      if (!hash.contains(vertex)) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  if (standardize) {
    Sphere::standardize();
  }

  createBuffers(program);
  setupVAO(program);

  if (satellite_of) {
    position = satellite_of->position + glm::vec3{orbit_radius, 0.0f, 0.0f};
  }
}

void Sphere::setupVAO(GLuint program) {
  // Release previous VAO
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }

  // End of binding
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}

void Sphere::render() const {
  abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &m_modelMatrix[0][0]);
  abcg::glUniform4fv(m_colorLoc, 1, &color[0]);

  abcg::glBindVertexArray(m_VAO);

  auto const numIndices{m_indices.size()};

  abcg::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);
}

void Sphere::standardize() {
  // Center to origin and normalize largest bound to [-1, 1]

  // Get bounds
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (auto const &vertex : m_vertices) {
    max = glm::max(max, vertex.position);
    min = glm::min(min, vertex.position);
  }

  // Center and scale
  auto const center{(min + max) / 2.0f};
  auto const scaling{2.0f / glm::length(max - min)};
  for (auto &vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

void Sphere::update(float rot_speed, float trans_speed) {
  if (satellite_of) {
    auto angle = m_translation_speed * trans_speed;
    m_translation_angle += angle;
    position.x =
        satellite_of->position.x + cos(m_translation_angle) * orbit_radius;
    position.z =
        satellite_of->position.z - sin(m_translation_angle) * orbit_radius;
  }

  // rotation
  auto angle = m_rotation_speed * rot_speed;
  m_rotation_angle += angle;

  computeModelMatrix();
}

void Sphere::updateSpeed() {
  const double constante_gravitacional = 0.0005;
  m_distance = getDistance(position.x, position.y, satellite_of->position.x,
                               satellite_of->position.y);
  if (m_distance < 1) {
    m_distance = 1;
  }

  auto produtoMassas = satellite_of->mass * mass;
  auto produtoDistancia = m_distance * m_distance;

  auto forcaGravitacional =
      constante_gravitacional * produtoMassas / produtoDistancia;

  auto cos = (satellite_of->position.x - position.x) / m_distance;
  auto sin = (satellite_of->position.y - position.y) / m_distance;

  glm::vec2 forca;
  forca.x = forcaGravitacional * cos;
  forca.y = forcaGravitacional * sin;

  glm::vec2 aceleracao;
  aceleracao.x = forca.x / mass;
  aceleracao.y = forca.y / mass;

  m_speed.x = m_speed.x + aceleracao.x;
  m_speed.y = m_speed.y + aceleracao.y;
}

float Sphere::getDistance(float x1, float y1, float x2, float y2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void Sphere::computeModelMatrix() {
  m_modelMatrix = glm::mat4(1.0f);
  m_modelMatrix = glm::translate(m_modelMatrix, position);
  m_modelMatrix =
      glm::rotate(m_modelMatrix, m_rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
  m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(scale));
}

void Sphere::destroy() const {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}