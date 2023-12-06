#include "model.hpp"
#include "core.h"

#include <SDL_shape.h>
#include <cmath>
#include <filesystem>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    auto const h2{std::hash<glm::vec3>()(vertex.normal)};
    auto const h3{std::hash<glm::vec2>()(vertex.texCoord)};
    return abcg::hashCombine(h1, h2, h3);
  }
};

void Model::createBuffers(GLuint program) {
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
  m_normalMatrixLoc = abcg::glGetUniformLocation(program, "normalMatrix");

  m_KaLoc = abcg::glGetUniformLocation(program, "Ka");
  m_KdLoc = abcg::glGetUniformLocation(program, "Kd");
  m_KsLoc = abcg::glGetUniformLocation(program, "Ks");
  m_shininessLoc = abcg::glGetUniformLocation(program, "shininess");
}

void Model::loadDiffuseTexture(std::string_view path) {
  if (!std::filesystem::exists(path))
    return;

  abcg::glDeleteTextures(1, &m_diffuseTexture);
  m_diffuseTexture = abcg::loadOpenGLTexture({.path = path});
}

void Model::loadObj(std::string_view path, GLuint program, bool standardize) {
  auto const basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};
  auto const &materials{reader.GetMaterials()};

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;
  m_hasTexCoords = false;

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
      glm::vec3 position{attrib.vertices.at(startIndex + 0),
                         attrib.vertices.at(startIndex + 1),
                         attrib.vertices.at(startIndex + 2)};

      // Normal
      glm::vec3 normal{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        auto const normalStartIndex{3 * index.normal_index};
        normal = {attrib.normals.at(normalStartIndex + 0),
                  attrib.normals.at(normalStartIndex + 1),
                  attrib.normals.at(normalStartIndex + 2)};
      }

      // Texture coordinates
      glm::vec2 texCoord{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        auto const texCoordsStartIndex{2 * index.texcoord_index};
        texCoord = {attrib.texcoords.at(texCoordsStartIndex + 0),
                    attrib.texcoords.at(texCoordsStartIndex + 1)};
      }

      Vertex const vertex{
          .position = position, .normal = normal, .texCoord = texCoord};

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

  // Use properties of first material, if available
  if (!materials.empty()) {
    auto const &mat{materials.at(0)}; // First material
    m_Ka = {mat.ambient[0], mat.ambient[1], mat.ambient[2], 1};
    m_Kd = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1};
    m_Ks = {mat.specular[0], mat.specular[1], mat.specular[2], 1};
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);
  } else {
    // Default values
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 25.0f;
  }

  if (standardize) {
    Model::standardize();
  }

  if (!m_hasNormals) {
    computeNormals();
  }

  createBuffers(program);
  setupVAO(program);
}

void Model::setupVAO(GLuint program) {
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

  auto const normalAttribute{abcg::glGetAttribLocation(program, "inNormal")};
  if (normalAttribute >= 0) {
    abcg::glEnableVertexAttribArray(normalAttribute);
    auto const offset{offsetof(Vertex, normal)};
    abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  auto const texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    auto const offset{offsetof(Vertex, texCoord)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  // End of binding
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}

void Model::computeNormals() {
  // Clear previous vertex normals
  for (auto &vertex : m_vertices) {
    vertex.normal = glm::vec3(0.0f);
  }

  // Compute face normals
  for (auto const offset : iter::range(0UL, m_indices.size(), 3UL)) {
    // Get face vertices
    auto &a{m_vertices.at(m_indices.at(offset + 0))};
    auto &b{m_vertices.at(m_indices.at(offset + 1))};
    auto &c{m_vertices.at(m_indices.at(offset + 2))};

    // Compute normal
    auto const edge1{b.position - a.position};
    auto const edge2{c.position - b.position};
    auto const normal{glm::cross(edge1, edge2)};

    // Accumulate on vertices
    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  // Normalize
  for (auto &vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Model::render(glm::mat4 modelMatrix, glm::mat4 viewMatrix) {
  abcg::glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

  auto const modelViewMatrix{glm::mat3(viewMatrix * modelMatrix)};
  auto const normalMatrix{glm::inverseTranspose(modelViewMatrix)};

  abcg::glUniformMatrix3fv(m_normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

  // Set uniform variables for the current model
  abcg::glUniform4fv(m_KaLoc, 1, &m_Ka.x);
  abcg::glUniform4fv(m_KdLoc, 1, &m_Kd.x);
  abcg::glUniform4fv(m_KsLoc, 1, &m_Ks.x);
  abcg::glUniform1f(m_shininessLoc, m_shininess);

  abcg::glBindVertexArray(m_VAO);

  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

  // Set minification and magnification parameters
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Set texture wrapping parameters
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  auto const numIndices{m_indices.size()};

  abcg::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);
}

void Model::standardize() {
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

void Model::update(float deltaTime) {
  if (satellite_of) {
    updateSatellite(deltaTime);
  }

  // rotation
  if (!m_collision) {
    auto angle = m_rotation_speed * deltaTime;
    rotation_angle += angle;
  }

  computeModelMatrix();
}

void Model::updateSatellite(float deltaTime) {
  double GRAVITATIONAL_CONST = 0.0005;

  double xPosition = position.x;
  double yPosition = position.y;
  double xPlanet = satellite_of->position.x;
  double yPlanet = satellite_of->position.y;
  double planetMass = satellite_of->mass;

  double dist = glm::length(glm::distance(position, satellite_of->position));

  double massProduct = mass * planetMass;
  double distProduct = dist * dist;

  double gravitationalForce = GRAVITATIONAL_CONST * massProduct / distProduct;

  double cos = (xPlanet - position.x) / dist;
  double sin = (yPlanet - position.y) / dist;
  
  glm::vec3 forca{0.0f};
  forca.x = gravitationalForce * cos;
  forca.y = gravitationalForce * sin;

  m_acceleration.x = forca.x / mass;
  m_acceleration.y = forca.y / mass;

  m_speed.x = m_speed.x + m_acceleration.x;
  m_speed.y = m_speed.y + m_acceleration.y;

  position += m_speed * deltaTime;

  double newDist = glm::length(glm::distance(position, satellite_of->position));
  if (newDist < satellite_of->radius + radius) {
    position.x = xPosition;
    position.y = yPosition;

    m_speed.x = 0;
    m_speed.y = 0;
  }
}

void Model::computeModelMatrix() {
  m_modelMatrix = glm::mat4(1.0f);

  m_modelMatrix = glm::translate(m_modelMatrix, position);
  m_modelMatrix = glm::rotate(m_modelMatrix, rotation_angle, m_axis);
  m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(scale));
}

void Model::destroy() const {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Model::setHorizontalSpeed(double horizontalSpeed) {
  m_speed.x = horizontalSpeed;
}