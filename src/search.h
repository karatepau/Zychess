#pragma once
#include "bitboard.h"
#include <chrono>

struct SearchState {
  u64 nodes = 0;
  bool stop = false;
  i64 timeLimit = 0;
  std::chrono::steady_clock::time_point startTime;
  i32 score = 0;
  i64 timeElapsed() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
  }
};
inline SearchState searchState;

inline constexpr std::array<std::array<i32, 7>, 7> MVV_LVA = {{
  {{0,  0,  0,  0,  0,  0,  0}},
  {{0, 15, 14, 13, 12, 11, 10}},
  {{0, 25, 24, 23, 22, 21, 20}},
  {{0, 35, 34, 33, 32, 31, 30}},
  {{0, 45, 44, 43, 42, 41, 40}},
  {{0, 55, 54, 53, 52, 51, 50}},
  {{0,  0,  0,  0,  0,  0,  0}}
}};

u16 iDeeping (Board& board);
template<Color C>
u16 getBestMove(Board& board, i32 depth, u16 bestMove);