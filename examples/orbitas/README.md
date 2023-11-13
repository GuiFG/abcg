# ATIVIDADE 02 - Aplicação gráfica 3D com animações

**Nome**: Guilherme Ferreira Galdino
**RA**: 11201811063


## Descrição

A aplicação consiste em mostrar alguns satélites orbitando um corpo central, em que cada satélite possui sua própria translação e distância de órbita. Além disso, é possível analisar todos os movimentos ao mudar a posição da camêra com o teclado e observar que cada corpo possui rotação em seu próprio eixo.

## Implementação

O diretório do projeto ```abcg/examples/orbitas``` possui a seguinte estrutura:

```
orbitas
│   camera.cpp
|   camera.hpp
|   CMakeLists.txt
|   main.cpp
|   sphere.cpp
|   sphere.hpp
|   window.cpp
|   window.hpp
│
└───assets
    │   sphere.frag
    │   sphere.vert
    └   sphere.obj

```

* ```camera.cpp``` e ```camera.hpp```: Código fonte responsável pela câmera.

* ```sphere.cpp``` e ```sphere.hpp```: Código para gerar o VBO, EBO e VAO para as esferas e renderizar cada uma de acordo com o model matrix.

* ```window.cpp``` e ```window.hpp```: Código responsável pela atualizações na tela e animações.

* ```main.cpp```: Responsável pela execução da aplicação e tratamento de erro.

O subdiretório ```assets``` contém os seguintes arquivos:

* ```sphere.obj```: arquivo obj para gerar a formato da esfera em 3D.
* ```sphere.vert``` e ```sphere.frag```: código-fonte do vertex shader e fragment shader utilizados para renderizar as esferas.

#### Código

Foi utlizado um modelo de esfera que cria o VBO, EBO e VAO do objeto esfera. Um array de esferas controla a matriz modelo de cada uma, que será renderizada na tela utilizando o modelo carregado inicialmente. Assim, cada esfera poderá ser personaliza em relação a sua órbita e velocidade utilizando o mesmo VBO. Além disso, foi utilizado interatividade com o teclado e mouse para controlar a visão da câmera, permitindo visualizar a movimentação dos objetos em todos os ângulos.

Modelo esfera utilizado para criar os objetos da tela: 

```cpp

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

  float m_distance {0.0f};

  void computeModelMatrix();
  void createBuffers(GLuint program);
  void setupVAO(GLuint program);
  void standardize();
  void updateSpeed();
  float getDistance(float x1, float y1, float x2, float y2);
};
```

1. ```loadObj```: Carrega o arquivo obj, obtendo os verifices e indices. Criar o VBO, EBO e o VAO. 

2. ```render```: Renderiza o modelo da esfera de acordo com a matriz modelo e a cor do objeto.

3. ```destroy```: Deleta os buffers VBO, EBO e VAO.

4. ```update```: Atualiza a velocidade de translação e a velocidade de rotação da esfera.

5. ```getModelMatrix```:  Recupera a matriz modelo da esfera para renderizar na janela personalizado.

6. ```getViewMatrix``` e ```getProjMatrix```: Recupera a matriz de visão e a matriz de projeção.

7. ```satellite_of```: Referência para "esfera pai". A esfera ficará em órbita em relação a ela. Caso se ja nulo, não está em órbita a nenhuma outra esfera.

8. ```scale```: Escala de cada esfera. Determina o tamanho da esfera.

9. ```orbit_radius```: Raio de órbita com a "esfera pai".

10. ```position```: Posição da esfera no espaço do mundo.

11. ```color```: Cor da esfera a ser renderizado.

12. ```z_index```: Varia se a translação será no eixo z. Caso contrário, será feita no eixo y.

13. ```translation_reduce```: Porcentagem relativa a velocidade total de translação, permite que cada esfera tenha uma velocidade de translação diferente.


Janela da aplicação, responsável pela interatividade:

```cpp
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
  std::default_random_engine m_randomEngine;
  glm::ivec2 m_viewportSize{};

  Sphere m_model;

  Camera m_camera;
  float m_dollySpeed{};
  float m_truckSpeed{};
  float m_panSpeed{};
  float m_verticalSpeed{};

  float m_rotation_speed{1.0f};
  float m_translation_speed{2.0f};

  std::array<Sphere, 6> m_spheres;

  GLuint m_program{};
};
```

1. ```onEvent```: Captura dos eventos de teclado e mouse.

2. ```onCreate```: Obtém o modelo no assets e cria os objetos.

3. ```onUpdate```:  Atualiza a velocidade de translação e rotação das esferas. Além disso, atualiza a posição da camêra de acordo com a interatividade do usuário.

4. ```onPaint```: Renderiza as esferas na janela.

5. ```onPaintUI``:  Possui slider para controlar a velocidade de translação das esferas.

6. ```onResize```: Atualiza o tamanho do viewport ao modificar o tamanho da janela.

7. ```m_randomEngine```: Engine para randomizar o raio de órbita e as cores das esferas.

8. ```m_model```: Modelo de esfera compartilhado pelos objetos. Cria o VBO, EBO e VAO das esferas.

8. ```m_camera```: Camera utilizada na janela para controlar a visão do espectador.

8. ```m_dollySpeed```, ```m_truckSpeed```, ```m_panSpeed``` e ```m_verticalSpeed```: Velocidade dos movimentos da câmera.

8. ```m_rotation_speed``` e ```m_translation_speed```: Velocidade de rotação e translação das esferas.

8. ```m_spheres```: Array das esferas a serem mostradas na tela. A primeira é a maior e a central. As outras são satélites que orbitam a esfera central.

8. ```m_model```: Modelo de esfera compartilhado pelos objetos. Cria o VBO, EBO e VAO das esferas.

8. ```m_program```: Program shader responsável pela transformações dos vértices dos objetos no pipeline gráfico;


Câmera para a visualização do mundo 3D:
```cpp
class Camera {
public:
  void computeViewMatrix();
  void computeProjectionMatrix(glm::vec2 const &size);

  void resizeViewport(glm::ivec2 const &size);
  void mouseScroll(float scroll); 

  void dolly(float speed);
  void truck(float speed);
  void pan(float speed);
  void pedestal(float speed);

  glm::mat4 const &getViewMatrix() const { return m_viewMatrix; }
  glm::mat4 const &getProjMatrix() const { return m_projMatrix; }

private:
  glm::vec3 m_eye{0.0f, 0.0f, 7.0f}; // Camera position
  glm::vec3 m_at{0.0f, 0.0f, 0.0f};  // Look-at point
  glm::vec3 m_up{0.0f, 1.0f, 0.0f};  // "up" direction

  // Matrix to change from world space to camera space
  glm::mat4 m_viewMatrix;

  // Matrix to change from camera space to clip space
  glm::mat4 m_projMatrix;

  glm::ivec2 m_viewportSize{};

  glm::vec3 m_lastPosition{};

  glm::vec3 m_axis{1.0f}; // axis for rotating camera when mouse is dragged
  glm::mat4 m_rotation{1.0f}; // angle for rotating camera when mouse is dragged 

  [[nodiscard]] glm::vec3 project(glm::vec2 const &mousePosition) const;
};
```

1. ```computeViewMatrix```: Calcular a matriz de visão da camêra, de acordo com os vetores eye, at, up.

2. ```computeProjectionMatrix```: Calcular a matriz de projeção em perspectiva.

3. ```resizeViewport```: Modifica o tamanho do viewport na camêra no onResize da janela.

4. ```mouseScroll```: Realiza o efeito de zoom no scroll do mouse.

5. ```dolly```, ```truck```, ```pan``` e ```vertical```:  Modifica a visão da camêra de acordo a interatividade com o teclado.

6. ```getViewMatrix``` e ```getProjMatrix```: Recupera a matriz de visão e a matriz de projeção.

7. ```m_eye```, ```m_at```, ```m_up```: Vetores para calcular a matriz visão da câmera.

