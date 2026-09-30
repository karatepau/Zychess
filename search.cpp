#include "search.h"

template<Color C>
i32 Board::eval() const {
  constexpr i32 val[6] = {100, 320, 330, 500, 900, 0};
  i32 score = 0;
  for (i32 p = 0; p < 6; p++)
    score += val[p] * (std::popcount(pieces_[p]) - std::popcount(pieces_[p + 6]));
  return C == WHITE ? score : -score;
}

template<Color C>
i32 Board::negaMax(i32 alpha, i32 beta, i32 depth) {
  if (depth == 0) return eval<C>();
  MoveList moves = getMoves<C>();
  for (u16 move : moves) {
    makeMove<C>(move);
    if (isCheck<C>()) {
      unmakeMove<C>();
      continue;
    }
    i32 score = -negaMax<C ^ 1>(-beta, -alpha, depth - 1);
    unmakeMove<C>();
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  return alpha;
}

template<Color C>
u16 Board::getBestMove(i32 depth) {
  MoveList moves = getMoves<C>();
  i32 alpha = -INF;
  u16 bestMove = 0;
  for (u16 move : moves) {
    makeMove<C>(move);
    if (isCheck<C>()) {
      unmakeMove<C>();
      continue;
    }
    i32 score = -negaMax<C ^ 1>(-INF, -alpha, depth - 1);
    unmakeMove<C>();
    if (score > alpha) {
      alpha = score;
      bestMove = move;
    }
  }
  return bestMove;
}

// needed so other translation units can link against these
template u16 Board::getBestMove<WHITE>(i32);
template u16 Board::getBestMove<BLACK>(i32);