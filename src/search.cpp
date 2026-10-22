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
  tt[hash & TTMASK].zobristHash = hash;
  tt[hash & TTMASK].depth = depth;
  tt[hash & TTMASK].scoreType = type;
  tt[hash & TTMASK].bestMove = bestMove;
  if (score >= INF - 1000) {
    tt[hash & TTMASK].score = score + ply;
  }
  else if (score <= -INF + 1000) {
    tt[hash & TTMASK].score = score - ply;
  }
  else {
    tt[hash & TTMASK].score = score;
  }
}

bool isRepetition(const Board& board) {
  const u64 hash = board.extras_[board.turn_].zobristHash;
  for (i32 i = static_cast<i32>(board.extras_[board.turn_].fiftyMovesRule) - 2; i >= 0; i -= 2) {
    if (board.extras_[i].zobristHash == hash) return true;
  }
  return false;
}

template<Color C>
i32 negaMax(Board& board, i32 alpha, i32 beta, i32 depth, bool allowNullMove, bool isPv) {
  if (searchState.stop) return 0;
  if (allowNullMove && isRepetition(board)) return 0;
  if ((searchState.nodes & 2047) == 0 && searchState.timeLimit && searchState.timeElapsed() >= searchState.timeLimit) {
    searchState.stop = true;
    return 0;
  }
  if (board.extras_[board.turn_].fiftyMovesRule >= 100) return 0;
  if (depth <= 0) return qSearch<C>(board, alpha, beta);
  searchState.nodes++;
  const u64 hash = board.extras_[board.turn_].zobristHash;
  u16 bestMove = 0;
  if (tt[hash & TTMASK].zobristHash == hash) {
    const TT& entry = tt[hash & TTMASK];
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
    bestMove = tt[hash & TTMASK].bestMove;
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
    i32 nullMoveScore = -negaMax<C ^ 1>(board, -beta, -beta + 1, depth - 3, false, false);
    board.turn_--;
    if (searchState.stop) return 0;
    if (nullMoveScore >= beta) return beta;
  }
  MoveList moves = board.getMoves<C>();
  i32 legal = 0;
  i32 score = -INF;
  bool firstMove = true;
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
    if (isPv && firstMove) {
      score = -negaMax<C ^ 1>(board, -beta, -alpha, depth - 1, true, true);
    }
    else {
      i32 reduction = legal >= 4 ? 1 : 0;
      score = -negaMax<C ^ 1>(board, -alpha - 1, -alpha, depth - 1 - reduction, true, false);
      if (reduction && score > alpha) {
        score = -negaMax<C ^ 1>(board, -alpha - 1, -alpha, depth - 1, true, false);
      }
      if (score > alpha && score < beta) {
        score = -negaMax<C ^ 1>(board, -beta, -alpha, depth - 1, true, isPv);
      }
    }
    firstMove = false;
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
  if (tt[hash & TTMASK].zobristHash != hash || tt[hash & TTMASK].depth < depth) {
    if (originalAlpha >= alpha) {
      storeBoardZobristHash(hash, depth, alpha, UPPER, bestMove, board.turn_);
    }
    else {
      storeBoardZobristHash(hash, depth, alpha, EXACT, bestMove, board.turn_);
    }
  }
  return alpha;
}

template<Color C>
u16 getBestMove(Board& board, i32 depth, u16 bestMove, i32 alpha, i32 beta) {
  bool isPv = true;
  i32 bestScore = -INF - 1000;
  constexpr u8 offset = C * 6;
  MoveList moves = board.getMoves<C>();
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
    i32 score;
    if (isPv) {
      score = -negaMax<C ^ 1>(board, -beta, -alpha, depth - 1, true, true);
    }
    else {
      score = -negaMax<C ^ 1>(board, -alpha - 1, -alpha, depth - 1, true, false);
      if (score > alpha && score < beta) {
        score = -negaMax<C ^ 1>(board, -beta, -alpha, depth - 1, true, false);
      }
    }
    isPv = false;
    board.unmakeMove<C>();
    if (score >= beta) {
      searchState.score = score;
      return moves.data[i];
    }
    if (score > bestScore) {
      bestScore = score;
      bestMove = moves.data[i];
    }
    if (score > alpha) {
      alpha = score;
    }
  }
  searchState.score = bestScore;
  return bestMove;
}

u16 iDeeping (Board& board) {
  searchState.stop = false;
  u16 bestMove = 0;
  i32 previousScore = 0;
  for (i32 depth = 1; depth < 16; depth++) {
    i32 betaDelta = 125;
    i32 alphaDelta = 125;
    i32 alpha = (depth == 1) ? -INF - 1000 : previousScore - alphaDelta;
    i32 beta  = (depth == 1) ?  INF + 1000 : previousScore + betaDelta;
    u16 move = bestMove;
    while (!searchState.stop) {
      if (board.turn_ & BLACK) {
        move = getBestMove<BLACK>(board, depth, bestMove, alpha, beta);
      } else {
        move = getBestMove<WHITE>(board, depth, bestMove, alpha, beta);
      }
      if (searchState.stop) break;
      const i32 score = searchState.score;
      if (score <= alpha) {
        alphaDelta *= 2;
        alpha = previousScore - alphaDelta;
      }
      else if (score >= beta) {
        betaDelta *= 2;
        beta = previousScore + betaDelta;
      }
      else {
        previousScore = score;
        bestMove = move;
        break;
      }
    }
    if (searchState.stop) break;
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
    std::cout << "info depth " << depth << " score " << scoreStr << " nodes " << searchState.nodes << " nps " << nps << " time " << ms << " pv " << moveToUCI(bestMove) << '\n';
    if (ms*3>=searchState.timeLimit) {
      break;
    }
  }
  searchState.timeLimit = 0;
  searchState.nodes = 0;
  return bestMove;
}

template u16 getBestMove<WHITE>(Board&, i32, u16, i32, i32);
template u16 getBestMove<BLACK>(Board&, i32, u16, i32, i32);