#include "search.h"

template<Color C>
std::array<i32, 218> valueMoves(const Board& board, const MoveList& moves) {
  std::array<i32, 218> scores;
  for (i32 i = 0; i < moves.count; i++) {
      u8 origin = (moves.data[i] >> 6) & 0x3f;
      u8 destination = moves.data[i] & 0x3f;
      u8 flag = moves.data[i] >> 12;
      i32 oriPiece;
      i32 destPiece;
      if constexpr (C == WHITE) {
        oriPiece = board.squares_[origin] + 1;
        destPiece = (board.squares_[destination] == -1) ? 0 : board.squares_[destination] - 5;
      }
      else {
        oriPiece = board.squares_[origin] - 5;
        destPiece = board.squares_[destination] + 1;
      }
      if (flag==9) {
        scores[i] = 60;
      }
      else {
        scores[i] = MVV_LVA[destPiece][oriPiece];
      }
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
i32 Board::qSearch(i32 alpha, i32 beta) {
  i32 bestValue = eval<C>();
  constexpr u8 offset = C * 6;
  u64 inCheck = getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]));
  MoveList moves;
  if (inCheck) {
    moves = getMoves<C>();
  }
  else {
    moves = getNoQuietMoves<C>();
    if (bestValue >= beta) {
      return beta;
    }
    if (bestValue > alpha) {
      alpha = bestValue;
    }
  }
  i32 legal = 0;
  std::array<i32, 218> movesEval = valueMoves<C>(*this, moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    u8 flag = moves.data[i] >> 12;
    if (!inCheck && flag >= 1 && flag <= 3) continue;
    makeMove<C>(moves.data[i]);
    if (getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]))) {
      unmakeMove<C>();
      continue;
    }
    legal++;
    i32 score = -qSearch<C ^ 1>(-beta, -alpha);
    unmakeMove<C>();
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  if (inCheck && legal == 0) {
    return -INF;
  }
  return alpha;
}

template<Color C>
i32 Board::negaMax(i32 alpha, i32 beta, i32 depth, bool allowNullMove) {
  if (depth == 0) return qSearch<C>(alpha, beta);
  constexpr u8 offset = C * 6;
  u64 inCheck = getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]));
  if (allowNullMove && depth >= 3 && !inCheck && (pieces_[WN + offset] | pieces_[WB + offset] | pieces_[WR + offset] | pieces_[WQ + offset])) {
    extras_[turn_ + 1] = extras_[turn_];
    extras_[turn_ + 1].passantSq = 0;
    turn_++;
    i32 nullMoveScore = -negaMax<C ^ 1>(-beta, -beta + 1, depth - 3, false);
    turn_--;
    if (nullMoveScore >= beta) return beta;
  }
  MoveList moves = getMoves<C>();
  i32 legal = 0;
  std::array<i32, 218> movesEval = valueMoves<C>(*this, moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    makeMove<C>(moves.data[i]);
    if (getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]))) {
      unmakeMove<C>();
      continue;
    }
    legal++;
    i32 score = -negaMax<C ^ 1>(-beta, -alpha, depth - 1, allowNullMove);
    unmakeMove<C>();
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  if (legal == 0) {
    if (inCheck) return -INF + turn_;
    return 0;
  }
  return alpha;
}

template<Color C>
u16 Board::getBestMove(i32 depth) {
  constexpr u8 offset = C * 6;
  MoveList moves = getMoves<C>();
  i32 alpha = -INF -1000;
  u16 bestMove = 0;
  std::array<i32, 218> movesEval = valueMoves<C>(*this, moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    makeMove<C>(moves.data[i]);
    if (getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]))) {
      unmakeMove<C>();
      continue;
    }
    i32 score = -negaMax<C ^ 1>(-INF, -alpha, depth - 1, true);
    unmakeMove<C>();
    if (score > alpha) {
      alpha = score;
      bestMove = moves.data[i];
    }
  }
  return bestMove;
}

template u16 Board::getBestMove<WHITE>(i32);
template u16 Board::getBestMove<BLACK>(i32);