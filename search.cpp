#include "search.h"

template<Color C>
i32 Board::negaMax(i32 alpha, i32 beta, i32 depth) {
  if (depth == 0) return eval<C>();
  MoveList moves = getMoves<C>();
  i32 legal = 0;
  for (u16 move : moves) {
    makeMove<C>(move);
    if (isCheck<C>()) {
      unmakeMove<C>();
      continue;
    }
    legal++;
    i32 score = -negaMax<C ^ 1>(-beta, -alpha, depth - 1);
    unmakeMove<C>();
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  if (legal == 0) {
    if (isCheck<C>()) return -INF + ply();
    return 0;
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