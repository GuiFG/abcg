#include "window.hpp"
#include "gamedata.hpp"
#include "imgui.h"
#include <glm/fwd.hpp>

void Window::onEvent(SDL_Event const &event) {
    // Mouse events
    if (event.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&m_mousePosition.x, &m_mousePosition.y);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT 
        && m_gameData.m_state != State::CircleDropping
        && m_gameData.m_state != State::RedWin
        && m_gameData.m_state != State::YellowWin) {
            m_gameData.m_state = State::CircleDrop;
        }
    }
}

void Window::onCreate() {
    auto const assetsPath{abcg::Application::getAssetsPath()};

    // Load a new font
    auto const filename{assetsPath + "Inconsolata-Medium.ttf"};
    m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
    if (m_font == nullptr) {
        throw abcg::RuntimeError("Cannot load font file");
    }

    m_program = abcg::createOpenGLProgram({{.source = assetsPath + "circle.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "circle.frag",
                                  .stage = abcg::ShaderStage::Fragment}});
}

void Window::onUpdate() {
    auto const deltaTime{gsl::narrow_cast<float>(getDeltaTime())};

    if (m_gameData.m_state == State::Draw ||
        m_gameData.m_state == State::YellowWin || 
        m_gameData.m_state == State::RedWin) {
            if (m_restartWaitTimer.elapsed() > 5) restart();
        return;
    }

    if (m_gameData.m_state == State::CircleDrop) {
        auto const idxCol = (int) (m_mousePosition.x / ((float) m_viewportSize.x / m_gameData.m_boardWidth));
        auto const idxRow = getNextRowBoard(idxCol);

        if (idxRow >= 0) {
            m_gameData.m_lastIndex = {idxRow, idxCol};
            auto const circlePosition = getCirclePositionViewPort({idxCol, idxRow});

            if (m_gameData.m_RedTurn) {
                m_circleRed.m_lastPosition = circlePosition;
            } else {
                m_circleYellow.m_lastPosition = circlePosition; 
            }

            m_gameData.m_state = State::CircleDropping;
        } else {
            m_gameData.m_state = m_gameData.m_RedTurn ? State::RedTurn : State::YellowTurn;
        }
    }

    if (m_gameData.m_state == State::CircleDropping) {
        bool isDropping = m_gameData.m_RedTurn ? m_circleRed.isDropping() : m_circleYellow.isDropping();

        if (!isDropping)
            m_gameData.m_state = State::CircleDropped;
    }

    if (m_gameData.m_state == State::CircleDropped) {
        auto const idx = m_gameData.m_boardWidth * m_gameData.m_lastIndex[0] + m_gameData.m_lastIndex[1];
        m_gameData.m_board.at(idx) = m_gameData.m_RedTurn ? 'R' : 'Y';
        m_gameData.m_RedTurn = !m_gameData.m_RedTurn;
        m_gameData.m_state = m_gameData.m_RedTurn ? State::RedTurn : State::YellowTurn;
        checkEndCondition();
    }

    auto const mousePosition = getMousePositionViewPort();
    m_circleRed.update(m_gameData, mousePosition, m_scale, deltaTime);
    m_circleYellow.update(m_gameData, mousePosition, m_scale, deltaTime);
}

void Window::onPaint() {
    abcg::glClearColor(0, 0, 255, 1);
    abcg::glClear(GL_COLOR_BUFFER_BIT);

    setupModel();

    abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

    for (auto const i : iter::range(m_gameData.m_boardHeigth)) {
        for (auto const j: iter::range(m_gameData.m_boardWidth)) {
            int idx = m_gameData.m_boardWidth * i + j;
            auto const player = m_gameData.m_board.at(idx);
            auto const circlePosition = getCirclePositionViewPort({j, i});
            if (player == 'R') {
                m_circleRed.paint(circlePosition, m_scale);
            } else if (player == 'Y') {
                m_circleYellow.paint(circlePosition, m_scale);
            } else {
                m_circle.paint(circlePosition, m_scale);
            }
        }
    }

    if (m_gameData.m_RedTurn)
        m_circleRed.paint(m_scale);
    else 
        m_circleYellow.paint(m_scale);
}


void Window::onPaintUI() {
    auto const size{ImVec2(350, 85)};
    auto const position{ImVec2((m_viewportSize.x - size.x) / 2.0f,
                                (m_viewportSize.y - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags const flags{ImGuiWindowFlags_NoBackground |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    ImVec4 fontColor {0.0f, 0.0f, 0.0f, 1.0f};

    if (m_gameData.m_state == State::RedWin) {
        ImGui::TextColored(fontColor, " Red Wins!");
    } else if (m_gameData.m_state == State::YellowWin) {
        ImGui::TextColored(fontColor, "Yellow Wins!");
    } else if (m_gameData.m_state == State::Draw) {
        ImGui::TextColored(fontColor, "    Draw! ");
    }

    ImGui::PopFont();
    ImGui::End();
}

void Window::setupModel() {
    m_circle.create(m_program, {1.0f,1.0f,1.0f,1.0f});
    m_circleRed.create(m_program, {1.0, 0, 0, 1});
    m_circleYellow.create(m_program, {1.0, 1.0, 0, 1});
}

void Window::onResize(glm::ivec2 const &size) {
    m_viewportSize = size;
    m_scale = (float) (0.9f * m_viewportSize.x / m_gameData.m_boardWidth) / m_viewportSize.x;

    abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
    m_circle.destroy();
    m_circleRed.destroy();
    m_circleYellow.destroy();
    abcg::glDeleteProgram(m_program);
}

glm::vec2 Window::getMousePositionViewPort() {
    glm::vec2 mousePosition;

    mousePosition.x = ((float) m_mousePosition.x / m_viewportSize.x) * 2 - 1;
    mousePosition.y = 1 - m_scale;

    return mousePosition;
}

glm::vec2 Window::getCirclePositionViewPort(glm::vec2 index) {
    auto const diameter { m_scale * 2 };
    auto const startHorizontal { -1 + m_scale };
    auto const startVertical { 1 - (3 * m_scale) };

    glm::vec2 circlePosition;
    circlePosition.x = startHorizontal + (diameter + 0.03f) * index.x;
    circlePosition.y = startVertical - (diameter + 0.03f) * index.y;

    return circlePosition;
}

int Window::getNextRowBoard(int col) {
    for (auto const i : iter::range(m_gameData.m_boardHeigth - 1, -1, -1)) {
        auto const symbol = m_gameData.m_board[m_gameData.m_boardWidth * i + col];
        if (symbol == '\0') {
            return i;
        }
    }

    return -1;
}

void Window::checkEndCondition() {
    if (m_gameData.m_state != State::RedTurn &&
        m_gameData.m_state != State::YellowTurn) {
        return;
    }

    auto allRedsOrYellows{[&](std::string_view str) {
        if (str == std::string(m_gameData.m_N, 'R')) {
            m_gameData.m_state = State::RedWin;
            return true;
        }
        if (str == std::string(m_gameData.m_N, 'Y')) {
            m_gameData.m_state = State::YellowWin;
            return true;
        }

        return false;
    }};

    // Check rows
    for (auto const i : iter::range(m_gameData.m_boardHeigth)) {
        for (auto const j : iter::range(m_gameData.m_boardWidth - m_gameData.m_N + 1)) {
            std::string concatenation;
            for (auto const k: iter::range(m_gameData.m_N)) {
                concatenation += m_gameData.m_board.at(i * m_gameData.m_boardWidth + j + k);
            }

            if (allRedsOrYellows(concatenation)) {
                fmt::print("FIM: LINHA\n");
                m_restartWaitTimer.restart();
                return;
            }
        }
    }

    // Check columns
    for (auto const j : iter::range(m_gameData.m_boardWidth)) {
        for (auto const i : iter::range(m_gameData.m_boardHeigth - m_gameData.m_N + 1)) {
            std::string concatenation;
            for (auto const k: iter::range(m_gameData.m_N)) {
                concatenation += m_gameData.m_board.at((i + k) * m_gameData.m_boardWidth + j);
            }

            if (allRedsOrYellows(concatenation)) {
                fmt::print("FIM: COLUNA\n");
                m_restartWaitTimer.restart();
                return;
            }   
        }
    }

    // Check main diagonal 
    for (auto const i: iter::range(m_gameData.m_boardHeigth - m_gameData.m_N + 1)) {
        for (auto const j: iter::range(m_gameData.m_boardWidth - m_gameData.m_N + 1)) {
            std::string concatenation;
            for (auto const k: iter::range(m_gameData.m_N)) {
                concatenation += m_gameData.m_board.at((i+k) * m_gameData.m_boardWidth + (j+k));
            }

            if (allRedsOrYellows(concatenation)) {
                fmt::print("FIM: DIAGONAL\n");
                m_restartWaitTimer.restart();
                return;
            }
        }
    }
    
    // Check inverse diagonal 
    for (auto const i: iter::range(m_gameData.m_boardHeigth - m_gameData.m_N + 1)) {
        for (auto const j: iter::range(m_gameData.m_boardWidth -1, m_gameData.m_boardWidth - m_gameData.m_N - 1, -1)) {
            std::string concatenation;
            for (auto const k: iter::range(m_gameData.m_N)) {
                concatenation += m_gameData.m_board.at((i+k) * m_gameData.m_boardWidth + (j-k));
            }

            if (allRedsOrYellows(concatenation)) {
                fmt::print("FIM: INVERSE DIAGONAL\n");
                m_restartWaitTimer.restart();
                return;
            }
        }
    }

    // Check draw
    if (std::find(m_gameData.m_board.begin(), m_gameData.m_board.end(), '\0') == m_gameData.m_board.end()) {
        m_gameData.m_state = State::Draw;
        m_restartWaitTimer.restart();
        fmt::print("DRAW\n");
    }
}

void Window::restart() {
    m_gameData.m_state = State::RedTurn;
    m_gameData.m_RedTurn = true;
    m_gameData.m_board.fill('\0');
}