#include "eval.h"

template<Color C>
i32 Board::eval() const {
  constexpr i32 val[6] = {100, 320, 330, 500, 900, 0};
  i32 score = 0;
  for (i32 p = 0; p < 6; p++)
    score += val[p] * (std::popcount(pieces_[p]) - std::popcount(pieces_[p + 6]));
  if constexpr (C == WHITE) return score;
  else return -score; 
}

template i32 Board::eval<WHITE>() const;
template i32 Board::eval<BLACK>() const;