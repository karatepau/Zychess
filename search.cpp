#include "search.h"

std::array<i32, 218> Board::valueMoves(const MoveList& moves) {
  std::array<i32, 218> scores;
  for (i32 i = 0; i < moves.count; i++) {
      u8 origin = (moves.data[i] >> 6) & 0x3f;
      u8 destination = moves.data[i] & 0x3f;
      scores[i] = (squares_[destination] % 6) - (squares_[origin] % 6);
  }
  return scores;
}

void promoteBestMove(i32 startIndex, MoveList& moves, std::array<i32, 218>& scores) {
  for (i32 i = startIndex; i < moves.count; i++) {
    if (scores[i] > scores[startIndex]) {
      std::swap(moves.data[i], moves.data[startIndex]);
      std::swap(scores[i], scores[startIndex]);
    }
  }
}

template<Color C>
i32 Board::negaMax(i32 alpha, i32 beta, i32 depth) {
  if (depth == 0) return eval<C>();
  MoveList moves = getMoves<C>();
  i32 legal = 0;
  std::array<i32, 218> movesEval = valueMoves(moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    makeMove<C>(moves.data[i]);
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
  std::array<i32, 218> movesEval = valueMoves(moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    makeMove<C>(moves.data[i]);
    if (isCheck<C>()) {
      unmakeMove<C>();
      continue;
    }
    i32 score = -negaMax<C ^ 1>(-INF, -alpha, depth - 1);
    unmakeMove<C>();
    if (score > alpha) {
      alpha = score;
      bestMove = moves.data[i];
    }
  }
  return bestMove;
}

// needed so other translation units can link against these
template u16 Board::getBestMove<WHITE>(i32);
template u16 Board::getBestMove<BLACK>(i32);