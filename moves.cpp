#include "bitboard.h"

void knightMoves(u64 pieces, u64 board, u16*& ptr) {
  while (pieces) {
    u16 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~board;
    while (legalMoves) {
      *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
      legalMoves &= legalMoves-1;
    }
    pieces &= pieces-1;
  }
}

void bishopMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u16 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moves.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    while (legalMoves) {
      *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
      legalMoves &= legalMoves-1;
    }
    pieces &= pieces-1;
  }
}

void rookMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u16 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moves.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    while (legalMoves) {
      *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
      legalMoves &= legalMoves-1;
    }
    pieces &= pieces-1;
  }
}

void kingMoves(u64 pieces, u64 board, u16*& ptr) {
  while (pieces) {
    u16 origin = __builtin_ctzll(pieces);
    u64 legalMoves = kMsk[origin] & ~board;
    while (legalMoves) {
      *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
      legalMoves &= legalMoves-1;
    }
    pieces &= pieces-1;
  }
}

void pawnMove (u64& legalMoves, u16*& ptr, i8 destDiff) {
  while (legalMoves) {
    u8 destination = __builtin_ctzll(legalMoves);
    u8 origin = destination + destDiff;
    *ptr++ = (origin << 6) | destination;
    legalMoves &= legalMoves-1;
  }
}

template<Color C>
void pawnMoves(u64 pieces, u64 both, u64 enemies, u16*& ptr, u64 passantSq) {
  u64 empty = ~both;
  u64 legalMoves;
  if constexpr (C == WHITE) {  
    legalMoves = (pieces << 8) & empty;
    pawnMove(legalMoves, ptr, -8);
    legalMoves = ((((pieces & 0xff00ULL) << 8) & empty) << 8) & empty;
    pawnMove(legalMoves, ptr, -16);
    legalMoves = (pieces << 7) & enemies;
    pawnMove(legalMoves, ptr, -9);
    legalMoves = (pieces << 9) & enemies;
    pawnMove(legalMoves, ptr, -7);
    legalMoves = (pieces << 7) & passantSq;
    pawnMove(legalMoves, ptr, -9);
    legalMoves = (pieces << 9) & passantSq;
    pawnMove(legalMoves, ptr, -7);
  }

  else {
    legalMoves = (pieces >> 8) & empty;
    pawnMove(legalMoves, ptr, 8);
    legalMoves = ((((pieces & 0xff000000000000ULL) >> 8) & empty) >> 8) & empty;
    pawnMove(legalMoves, ptr, 16);
    legalMoves = (pieces >> 7) & enemies;
    pawnMove(legalMoves, ptr, 9);
    legalMoves = (pieces >> 9) & enemies;
    pawnMove(legalMoves, ptr, 7);
    legalMoves = (pieces >> 7) & passantSq;
    pawnMove(legalMoves, ptr, 9);
    legalMoves = (pieces >> 9) & passantSq;
    pawnMove(legalMoves, ptr, 7);
  }
}

template<Color C>
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moves, u8 passantSq) {
  u16* ptr = maxMovesList.data();
  constexpr int offset = C * 6;
  knightMoves(board.pieces[WN + offset], board.occupancies[C], ptr);
  bishopMoves(moves, board.pieces[WB + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  rookMoves(moves, board.pieces[WR + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  bishopMoves(moves, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  kingMoves(board.pieces[WK + offset], board.occupancies[C], ptr);
  pawnMoves<C>(board.pieces[WP + offset], board.occupancies[BOTH], board.occupancies[C ^ 1ULL], ptr, u64 (passantSq << 1ULL));
  return std::span<u16>(maxMovesList.data(), ptr);
}

template std::span<u16> getMoves<WHITE>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moves, u8 passantSq);
template std::span<u16> getMoves<BLACK>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moves, u8 passantSq);