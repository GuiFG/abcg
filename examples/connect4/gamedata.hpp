#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <array>
#include <vector>
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

#endif