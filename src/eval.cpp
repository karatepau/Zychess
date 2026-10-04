#include "eval.h"

template<Color C>
i32 evalPiece(u64 pieces, const std::array<i32, 64>& table) {
  i32 score = 0;
  while (pieces) {
    u64 piecePos = __builtin_ctzll(pieces);
    if constexpr (C == WHITE) {
      score += table[piecePos^56];
    }
    else {
      score += table[piecePos];
    }
    POPLSB(pieces);
  }
  return score;
}

template<Color C>
i32 eval(Board& board) {
  constexpr i32 offset = C * 6;
  i32 wScore = 0;
  i32 bScore = 0;
  i32 phase = 0;
  phase += std::popcount(board.pieces_[WN]) + std::popcount(board.pieces_[BN]);
  phase += std::popcount(board.pieces_[WB]) + std::popcount(board.pieces_[BB]);
  phase += (std::popcount(board.pieces_[WR]) + std::popcount(board.pieces_[BR])) << 1;
  phase += (std::popcount(board.pieces_[WQ]) + std::popcount(board.pieces_[BQ])) << 2;
  if (phase <= 8) {
    wScore += evalPiece<WHITE>(board.pieces_[WP], EGP);
    wScore += evalPiece<WHITE>(board.pieces_[WN], EGN);
    wScore += evalPiece<WHITE>(board.pieces_[WB], EGB);
    wScore += evalPiece<WHITE>(board.pieces_[WR], EGR);
    wScore += evalPiece<WHITE>(board.pieces_[WQ], EGQ);
    wScore += evalPiece<WHITE>(board.pieces_[WK], EGK);

    bScore += evalPiece<BLACK>(board.pieces_[BP], EGP);
    bScore += evalPiece<BLACK>(board.pieces_[BN], EGN);
    bScore += evalPiece<BLACK>(board.pieces_[BB], EGB);
    bScore += evalPiece<BLACK>(board.pieces_[BR], EGR);
    bScore += evalPiece<BLACK>(board.pieces_[BQ], EGQ);
    bScore += evalPiece<BLACK>(board.pieces_[BK], EGK);
  }
  else {
    wScore += evalPiece<WHITE>(board.pieces_[WP], MGP);
    wScore += evalPiece<WHITE>(board.pieces_[WN], MGN);
    wScore += evalPiece<WHITE>(board.pieces_[WB], MGB);
    wScore += evalPiece<WHITE>(board.pieces_[WR], MGR);
    wScore += evalPiece<WHITE>(board.pieces_[WQ], MGQ);
    wScore += evalPiece<WHITE>(board.pieces_[WK], MGK);

    bScore += evalPiece<BLACK>(board.pieces_[BP], MGP);
    bScore += evalPiece<BLACK>(board.pieces_[BN], MGN);
    bScore += evalPiece<BLACK>(board.pieces_[BB], MGB);
    bScore += evalPiece<BLACK>(board.pieces_[BR], MGR);
    bScore += evalPiece<BLACK>(board.pieces_[BQ], MGQ);
    bScore += evalPiece<BLACK>(board.pieces_[BK], MGK);
  }
  if constexpr (C == WHITE) {
    return wScore - bScore;
  }
  else {
    return bScore - wScore;
  }
}

template i32 eval<WHITE>(Board& board);
template i32 eval<BLACK>(Board& board);