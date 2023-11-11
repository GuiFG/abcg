#include "circle.hpp"
#include "gamedata.hpp"

void Circle::create(GLuint program, glm::vec4 color) {
    destroy();

    m_program = program;

    // Get location of uniforms in the program
    m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");
    m_scaleLoc = abcg::glGetUniformLocation(m_program, "scale");

    std::vector<glm::vec2> positions;
    std::vector<glm::vec4> colors;

    // Polygon center
    positions.emplace_back(0, 0);
    colors.push_back(color);

    // Border vertices
    auto const step{M_PI * 2 / m_sides};
    for (auto const angle : iter::range(0.0, M_PI * 2 + step, step)) {
        positions.emplace_back(std::cos(angle), std::sin(angle));
        colors.push_back(color);
    }

    // Generate VBO of positions
    abcg::glGenBuffers(1, &m_VBOPositions);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
    abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                        positions.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate VBO of colors
    abcg::glGenBuffers(1, &m_VBOColors);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
    abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4),
                        colors.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Get location of attributes in the program
    auto const positionAttribute{
        abcg::glGetAttribLocation(m_program, "inPosition")};
    auto const colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

    // Create VAO
    abcg::glGenVertexArrays(1, &m_VAO);

    // Bind vertex attributes to current VAO
    abcg::glBindVertexArray(m_VAO);

    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
    abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                                nullptr);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    abcg::glEnableVertexAttribArray(colorAttribute);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
    abcg::glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                                nullptr);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    // End of binding to current VAO
    abcg::glBindVertexArray(0);
}

void Circle::paint(glm::vec2 translation, float scale) {
    abcg::glUseProgram(m_program);

    // fmt::print("scale = {} e translation {}x{}\n", scale, translation.x, translation.y);
    abcg::glUniform2fv(m_translationLoc, 1, &translation.x);
    abcg::glUniform1f(m_scaleLoc, scale);

    // Render
    abcg::glBindVertexArray(m_VAO);
    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, m_sides + 2);
    abcg::glBindVertexArray(0);

    abcg::glUseProgram(0);
}

void Circle::paint(float scale) {
    abcg::glUseProgram(m_program);

    // fmt::print("scale = {} e translation {}x{}\n", scale, m_translation.x, m_translation.y);
    abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);
    abcg::glUniform1f(m_scaleLoc, scale);

    // Render
    abcg::glBindVertexArray(m_VAO);
    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, m_sides + 2);
    abcg::glBindVertexArray(0);

    abcg::glUseProgram(0);
}

void Circle::destroy() {
    abcg::glDeleteBuffers(1, &m_VBOPositions);
    abcg::glDeleteBuffers(1, &m_VBOColors);
    abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Circle::update(
    const GameData &gameData, 
    const glm::vec2 &mousePosition,
    float scale,
    float deltaTime
) {
    if (gameData.m_state != State::CircleDropping) {
        m_translation = {mousePosition.x, 1 - scale };
    } else {
        if (isDropping()) {
            m_translation.x = m_lastPosition.x;
            m_translation.y -= scale * deltaTime * 20;
        } else {
            m_translation.y = 1 - scale;
        }   
    }
}

bool Circle::isDropping() {
    return m_translation.y >= m_lastPosition.y;
}