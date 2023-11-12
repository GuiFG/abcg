# ATIVIDADE 01 - Aplicação interativa com gráficos 2D

**Nome**: Guilherme Ferreira Galdino
**RA**: 11201811063


## Descrição

A aplicação consiste em replicar o jogo *Connect 4*, em que é jogado por duas pessoas utilizando um tabuleiro com 6 linhas e 7 colunas. O objetivo do jogo é conectar quatro peças em sequência, seja na vertical, horizontal ou diagonal. Os jogadores se revezam colocando uma peça de sua cor em uma das colunas do tabuleiro. A peça cai para o espaço mais baixo disponível na coluna escolhida, semelhante ao tabuleiro real. Se todas as colunas estiverem cheias e nenhum jogador conseguir formar uma conexão de quatro peças, o jogo é considerado um empate.

## Implementação

O diretório do projeto ```abcg/examples/connect4``` possui a seguinte estrutura:

```
connect4
│   circle.cpp
|   circle.hpp
|   CMakeLists.txt
|   gamedata.hpp
|   main.cpp
|   window.cpp
|   window.hpp
│
└───assets
    │   circle.frag
    │   circle.vert
    └   Inconsolata-Medium.ttf

```

* ```circle.cpp``` e ```circle.hpp```: Código fonte responsável pelas *peças* (círculos) do jogo.

* ```gamedata.hpp```: Configuração do jogo, como tamanho do tabuleiro e estados jogo.

* ```window.cpp``` e ```window.hpp```: Código responsável pela atualizações na tela e iterativdade.

* ```main.cpp```: Responsável pela execução da aplicação e tratamento de erro.

O subdiretório ```assets``` contém os seguintes arquivos:

* ```Inconsolata-Medium.ttf```: fonte Inconsolata utilizada na mensagem “Red Wins!”, "YellowWins" e "Draw".
* ```circle.vert``` e ```circle.frag```: código-fonte do vertex shader e fragment shader utilizados para renderizar os círculos.

#### Código

Foram utilizados 3 objetos (VBOs de posições e cores) para formar as *peças* do jogo. Um para o círculo vazio, outro para o círculo vermelho e outro para o círculo amarelo. A cada atualização do quadro na tela, é atualizado as posições dos objetos de acordo com o estado do tabuleiro.

A configuração do jogo está definida no arquivo ```gamedata.hpp```:

```cpp
enum class State { RedTurn, YellowTurn, RedWin, YellowWin, Draw, CircleDrop, CircleDropping, CircleDropped };

struct GameData {
  bool m_RedTurn{true};
  static int const m_N{4};
  static int const m_boardWidth{7}; 
  static int const m_boardHeigth{6};
  std::array<char, m_boardWidth * m_boardHeigth> m_board{}; // '\0', 'R' or 'Y'
  State m_state{State::RedTurn};

  std::array<int, 2> m_lastIndex {0, 0};
};
```

Aqui são definidos os estados o jogo.

1. ```RedTurn```: Estado que a vez da jogada é do jogador com a peça (círculo) vermelho.

2. ```YellowTurn```: Estado que a vez da jogada é do jogador com a peça (círculo) amarelo.

3. ```RedWin```: Estado que indica que o jogador vermelho venceu.

4. ```YellowWin```: Estado que indica que o jogador amarelho venceu.

5. ```Draw```: Estado que mostra empate da partida.

6. ```CircleDrop```: Estado que ativa a animação de queda da peça.

7. ```CircleDropping```: Estado indicando o momento em que a peça está caindo.

8. ```CircleDropped```: Estado que indica o momento que a peça atingiu a posição estipulada.

Na sequência é definido a estrutura dos dados do jogo. Nela temos:
* A quantidade de linhas e colunas no tabuleiro. 
* A sequência de símbolos representando o estado do tabuleiro. 
* A quantidade de sequência de peças que são necessárias para se linhar (no caso são 4 peças). 
* Uma variável (m_lastIndex) que armazena o indíce no tabuleiro estipulado para a peça no momento do clique, por conta dos estados de queda da peça (CircleDrop, CircleDropping e CircleDropped).

As funções definidas em ```circle``` são:

```cpp
void create(GLuint program, glm::vec4 color);
void update(const GameData &gameData, const glm::vec2 &mousePosition, float scale, float DeltaTime);
void paint(glm::vec2 translation, float scale);
void paint(float scale);
void destroy();
bool isDropping();
```

* ```create```: Cria o VBO de posição e de cor responsável por formar uma polígono regular de 100 lados, obtendo uma forma semelhante ao um círculo. Cria o VAO passando o VBO para o vertex shader.

* ```update```: Atuliza a posição do VBO de acordo com o estado atual do jogo. 

* ```paint```: Duas funções para desenhar o VBO na tela. Uma especificando a posição exata e outra obtendo a posição atualizada na função update. São utilizadas variáveis uniformes para alterar os pontos do VBO tanto na escala quanto na translação.

* ```destroy```: Deleta os VBOS e o VAO do buffer.

* ```isDropping```: Função para verificar a posição vertical durante o estado *CircleDropping* do círculo. Caso ultrapasse a posição estipulada do círculo no tabuleiro, a função retorna falso, indicando o fim da queda.


As funções definidas em ```window``` são:
```cpp
void onEvent(SDL_Event const &event) override;
void onCreate() override;
void onUpdate() override;
void onPaint() override;
void onPaintUI() override;
void onResize(glm::ivec2 const &size) override;
void onDestroy() override;
```

* ```onEvent```: Captura os cliques do jogador feito na tela, alterando para o estado de *peça* caindo (CircleDrop).

* ```onCreate```: Inicializa fonte *Inconsolata*, shader e os 3 VBOs (círculos).

* ```onUpdate```: Obtém os valores da posição da peça atual para atualizar a iterativadade do mouse com o círculo. Obtém a posição do clique e qual parte do tabuleiro a peça terá que ir. Altera o estado para *CircleDropping* permitindo a animação da gravidade da peça. Verifica a condição final do jogo, após a peça chegar na posição atribuída. Caso jogo esteja finalizado, espera 5 segundos para reiniciar a partida.

* ```onPaint```: Limpa a tela pintando o fundo de azul (cor do tabuleiro). Desenha os círculos conforme o estado atual do tabuleiro. Desenho o círculo do jogador atual na parte superior da tela.

* ```onPaintUI```: No final da partida, mostra um texto no centro da tela indicando o jogador vencedor ou um empate.

* ```onResize``` e ```onDestroy```: onResize atualiza o tamanho do viewPort caso a janela seja redimensionada. E onDestroy, apaga os VBOs, VAO e o program da memória.

Além dessas, foram definidas as seguintes funções:

```cpp
void setupModel();
glm::vec2 getMousePositionViewPort();
glm::vec2 getCirclePositionViewPort(glm::vec2 index);
int getNextRowBoard(int col);
void checkEndCondition();
void restart();
```


* ```setupModel```: Cria os três círculos que serão utilizados no jogo, um de cada cor (branco, vermelho e amarelo), da classe ```circle.hpp```.

* ```getMousePositionViewPort```: Converte a posição do mouse no momento de um clique para a posição nas coordenadas do gsl [-1, 1].

* ```getCirclePositionViewPort```: Converte um indíce do tabuleiro (matriz) para um valor no sistema de coordenadas do gsl [-1, 1].

* ```getNextRowBoard```: Obtém o indíce disponível da linha do tabuleiro em uma dada coluna.

* ```checkEndCondition```: Verifica a condição do jogo. Faz uma varredura no tabuleiro, tanto horizontal, vertical e na diagonal. Para cada varredura, busca sequência de 4 caracteres. Caso sejam iguais, tem-se um vencedor. Caso contrário e não tem mais posições vazia, altera o estado do jogo para empate (Draw).
 
* ```restart```: Retorna o estado do jogo para o início e atualiza o tabuleiro preenchendo todas as posições como vazio.








