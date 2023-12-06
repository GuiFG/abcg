#include "skybox.hpp"
#include <filesystem>


void Skybox::createBuffers(GLuint program){
  generateUVSphere(36,36);
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

  // Release previous VAO
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);

  // Bind EBO and VBO
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  // Bind vertex attributes
  auto const positionAttribute{abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(VertexSky), nullptr);
  }

  auto const texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    auto const offset{offsetof(VertexSky, texCoord)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(VertexSky),
                                reinterpret_cast<void *>(offset));
  }

  // End of binding
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);

  if (std::filesystem::exists(texture_path)){
    abcg::glDeleteTextures(1, &m_diffuseTexture);
    m_diffuseTexture = abcg::loadOpenGLTexture({.path = texture_path});
  }
}

void Skybox::destroy(){
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Skybox::generateUVSphere(int stacks, int sectors){
  // geneterates a UV sphere with radius 1.0f
  float radius = 50.0f;
  float sectorAngle, stackAngle, x, y, z, xz, u, v;
  float sectorStep = 2 * M_PI / sectors;
  float stackStep = M_PI / stacks;

  for(auto i : iter::range(stacks+1)){
    stackAngle = M_PI / 2 - i * stackStep;
    xz = std::cos(stackAngle);
    y = std::sin(stackAngle) * radius;

    for(auto j: iter::range(sectors+1)){
      sectorAngle = j * sectorStep;

      x = xz * std::sin(sectorAngle) * radius;
      z = xz * std::cos(sectorAngle) * radius;


      u = (float) j / sectors;
      v = 1.0f - (float) i / stacks;

      VertexSky const vertex{.position = {x, y, z}, .normal = {x, y, z}, .texCoord = {u,v}};

      m_vertices.push_back(vertex);

    }
  }
  // now we create the indices for the triangles
  int k1, k2;
  for(int i = 0; i < stacks; ++i){
    k1 = i * (sectors + 1);     // beginning of current stack
    k2 = k1 + sectors + 1;      // beginning of next stack

    for(int j = 0; j < sectors; ++j, ++k1, ++k2)
    {
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if(i != 0)
      {
          m_indices.push_back(k1);
          m_indices.push_back(k2);
          m_indices.push_back(k1 + 1);
      }

      // k1+1 => k2 => k2+1
      if(i != (stacks-1))
      {
          m_indices.push_back(k1 + 1);
          m_indices.push_back(k2);
          m_indices.push_back(k2 + 1);
      }
    }
  }


}

void Skybox::render() const {
  abcg::glBindVertexArray(m_VAO);

  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

  abcg::glEnable(GL_CULL_FACE);
  abcg::glFrontFace(GL_CW);
  abcg::glDepthFunc(GL_LEQUAL);

  // Draw Skybox triangles
  abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
  abcg::glFrontFace(GL_CCW);

  abcg::glDepthFunc(GL_LESS);
  abcg::glDisable(GL_CULL_FACE);


  abcg::glBindVertexArray(0);
}


