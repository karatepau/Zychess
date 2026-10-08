#include "search.h"
#include "eval.h"
#include "uci.h"

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
i32 qSearch(Board& board, i32 alpha, i32 beta) {
  if (searchState.stop) return 0;
  if ((searchState.nodes & 2047) == 0 && searchState.timeLimit && searchState.timeElapsed() >= searchState.timeLimit) {
    searchState.stop = true;
    return 0;
  }
  searchState.nodes++;
  i32 bestValue = eval<C>(board);
  constexpr u8 offset = C * 6;
  u64 inCheck = board.getAttackers<C ^ 1>(__builtin_ctzll(board.pieces_[WK + offset]));
  MoveList moves;
  if (inCheck) {
    moves = board.getMoves<C>();
  }
  else {
    moves = board.getNoQuietMoves<C>();
    if (bestValue >= beta) {
      return beta;
    }
    if (bestValue > alpha) {
      alpha = bestValue;
    }
  }
  i32 legal = 0;
  std::array<i32, 218> movesEval = valueMoves<C>(board, moves);
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    u8 flag = moves.data[i] >> 12;
    if (!inCheck && flag >= 1 && flag <= 3) continue;
    board.makeMove<C>(moves.data[i]);
    if (board.getAttackers<C ^ 1>(__builtin_ctzll(board.pieces_[WK + offset]))) {
      board.unmakeMove<C>();
      continue;
    }
    legal++;
    i32 score = -qSearch<C ^ 1>(board, -beta, -alpha);
    board.unmakeMove<C>();
    if (searchState.stop) return 0;
    if (score >= beta) return beta;
    if (score > alpha) alpha = score;
  }
  if (inCheck && legal == 0) {
    return -INF + board.turn_;
  }
  return alpha;
}

void storeBoardZobristHash(const u64& hash, const i32& depth, const i32& score, const u8& type, const u16& bestMove, const u16& ply) {
  tt[hash & 0xfffff].zobristHash = hash;
  tt[hash & 0xfffff].depth = depth;
  tt[hash & 0xfffff].scoreType = type;
  tt[hash & 0xfffff].bestMove = bestMove;
  if (score >= INF - 1000) {
    tt[hash & 0xfffff].score = score + ply;
  }
  else if (score <= -INF + 1000) {
    tt[hash & 0xfffff].score = score - ply;
  }
  else {
    tt[hash & 0xfffff].score = score;
  }
}

bool isRepetition(const Board& board) {
  const u64 hash = board.extras_[board.turn_].zobristHash;
  for (i32 i = static_cast<i32>(board.turn_) - 2; i >= 0; i -= 2) {
    if (board.extras_[i].zobristHash == hash) return true;
  }
  return false;
}

template<Color C>
i32 negaMax(Board& board, i32 alpha, i32 beta, i32 depth, bool allowNullMove) {
  if (searchState.stop) return 0;
  if (allowNullMove && isRepetition(board)) return 0;
  if ((searchState.nodes & 2047) == 0 && searchState.timeLimit && searchState.timeElapsed() >= searchState.timeLimit) {
    searchState.stop = true;
    return 0;
  }
  if (depth == 0) return qSearch<C>(board, alpha, beta);
  searchState.nodes++;
  const u64 hash = board.extras_[board.turn_].zobristHash;
  u16 bestMove = 0;
  if (tt[hash & 0xfffff].zobristHash == hash) {
    const TT& entry = tt[hash & 0xfffff];
    if (entry.depth >= depth) {
      i32 ttScore = entry.score;
      if (ttScore >= INF - 1000) ttScore -= board.turn_;
      else if (ttScore <= -INF + 1000) ttScore += board.turn_;
      switch (entry.scoreType) {
        case EXACT:
          return ttScore;
        case LOWER:
          if (ttScore >= beta) return beta;
          break;
        case UPPER:
          if (ttScore <= alpha) return alpha;
          break;
      }
    }
    bestMove = tt[hash & 0xfffff].bestMove;
  }
  i32 originalAlpha = alpha;
  constexpr u8 offset = C * 6;
  u64 inCheck = board.getAttackers<C ^ 1>(__builtin_ctzll(board.pieces_[WK + offset]));
  if (allowNullMove && depth >= 3 && !inCheck && (board.pieces_[WN + offset] | board.pieces_[WB + offset] | board.pieces_[WR + offset] | board.pieces_[WQ + offset])) {
    board.extras_[board.turn_ + 1] = board.extras_[board.turn_];
    if (u8& passantSq = board.extras_[board.turn_].passantSq) {
      constexpr u64 direction = (C == WHITE) ? -8 : 8;
      u64 pushedPos = 1ULL << (passantSq + direction);
      u64 adjacentMask = ((pushedPos & 0xfefefefefefefefeULL) >> 1) | ((pushedPos & 0x7f7f7f7f7f7f7f7fULL) << 1);
      if (adjacentMask & board.pieces_[C * 6]) {
        board.extras_[board.turn_ + 1].zobristHash ^= zobristKeys.enPassantFile[passantSq & 7];
      }
    }
    board.extras_[board.turn_ + 1].zobristHash ^= zobristKeys.side;
    board.extras_[board.turn_ + 1].passantSq = 0;
    board.turn_++;
    i32 nullMoveScore = -negaMax<C ^ 1>(board, -beta, -beta + 1, depth - 3, false);
    board.turn_--;
    if (searchState.stop) return 0;
    if (nullMoveScore >= beta) return beta;
  }
  MoveList moves = board.getMoves<C>();
  i32 legal = 0;
  std::array<i32, 218> movesEval = valueMoves<C>(board, moves);
  if (bestMove) {
    for (i32 i = 0; i < moves.count; i++) {
      if (bestMove == moves.data[i]) {
        movesEval[i] = INF;
        break;
      }
    }
  }
  for (i32 i = 0; i < moves.count; i++) {
    promoteBestMove(i, moves, movesEval);
    board.makeMove<C>(moves.data[i]);
    if (board.getAttackers<C ^ 1>(__builtin_ctzll(board.pieces_[WK + offset]))) {
      board.unmakeMove<C>();
      continue;
    }
    legal++;
    i32 score = -negaMax<C ^ 1>(board, -beta, -alpha, depth - 1, true);
    board.unmakeMove<C>();
    if (searchState.stop) return 0;
    if (score >= beta) {
      storeBoardZobristHash(hash, depth, beta, LOWER, moves.data[i], board.turn_);
      return beta;
    }
    if (score > alpha) {
      bestMove = moves.data[i];
      alpha = score;
    }
  }
  if (legal == 0) {
    if (inCheck) {
      return -INF + board.turn_;
    }
    return 0;
  }
  if (originalAlpha >= alpha) {
    storeBoardZobristHash(hash, depth, alpha, UPPER, bestMove, board.turn_);
  }
  else {
    storeBoardZobristHash(hash, depth, alpha, EXACT, bestMove, board.turn_);
  }
  return alpha;
}

template<Color C>
u16 getBestMove(Board& board, i32 depth, u16 bestMove) {
  searchState.stop = false;
  constexpr u8 offset = C * 6;
  MoveList moves = board.getMoves<C>();
  i32 alpha = -INF -1000;
  std::array<i32, 218> movesEval = valueMoves<C>(board, moves);
  if (bestMove) {
    for (i32 i = 0; i < moves.count; i++) {
      if (bestMove == moves.data[i]) {
        movesEval[i] = INF;
        break;
      }
    }
  }
  for (i32 i = 0; i < moves.count; i++) {
    if (searchState.stop) break;
    promoteBestMove(i, moves, movesEval);
    board.makeMove<C>(moves.data[i]);
    if (board.getAttackers<C ^ 1>(__builtin_ctzll(board.pieces_[WK + offset]))) {
      board.unmakeMove<C>();
      continue;
    }
    i32 score = -negaMax<C ^ 1>(board, -INF, -alpha, depth - 1, true);
    board.unmakeMove<C>();
    if (score > alpha) {
      alpha = score;
      bestMove = moves.data[i];
    }
  }
  searchState.score = alpha;
  return bestMove;
}

u16 iDeeping (Board& board) {
  u16 bestMove = 0;
  for (i32 i = 1; i < 16; i++) {
    u16 move = 0;
    if (board.turn_ & BLACK) {
      move = getBestMove<BLACK>(board, i, bestMove);
    }
    else {
      move = getBestMove<WHITE>(board, i, bestMove);
    }
    if (searchState.stop) {
      break;
    }
    bestMove = move;
    i64 ms = std::max<i64>(1, searchState.timeElapsed());
    u64 nps = searchState.nodes * 1000 / ms;
    std::string scoreStr;
    if (searchState.score >= INF - 1000) {
      i32 plies = INF - searchState.score - board.turn_;
      scoreStr = "mate " + std::to_string((plies + 1) / 2);
    }
    else if (searchState.score <= -INF + 1000) {
      i32 plies = INF + searchState.score - board.turn_;
      scoreStr = "mate " + std::to_string(-(plies / 2));
    }
    else {
      scoreStr = "cp " + std::to_string(searchState.score);
    }
    std::cout << "info depth " << i << " score " << scoreStr << " nodes " << searchState.nodes << " nps " << nps << " time " << ms << " pv " << moveToUCI(bestMove) << '\n';
  }
  searchState.timeLimit = 0;
  searchState.nodes = 0;
  return bestMove;
}

template u16 getBestMove<WHITE>(Board& board, i32 depth, u16 bestMove);
template u16 getBestMove<BLACK>(Board& board, i32 depth, u16 bestMove);