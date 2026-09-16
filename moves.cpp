#include "bitboard.h"

inline void knightMoves(u64 pieces, u64 board, u16*& ptr) {
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

inline void bishopMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u16*& ptr) {
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

inline void rookMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u16*& ptr) {
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

inline void kingMoves(u64 pieces, u64 board, u16*& ptr) {
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
inline void pawnMove (u64& legalMoves, u16*& ptr, i8 destDiff) {
  while (legalMoves) {
    i8 destination = __builtin_ctzll(legalMoves);
    *ptr++ = (destination << 6) | (destination + destDiff);
    legalMoves &= legalMoves-1;
  }
}

template<Color C>
void pawnMoves(u64 pieces, u64 both, u64 enemies, u8 const color, u16*& ptr) {
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
  }
}

template<Color C>
std::span<u16> getMoves(const Board& board, u8 const color, std::array<u16, 218>& maxMovesList, MoveTables& moves) {
  u16* ptr = maxMovesList.data();
  constexpr int offset = C * 6;
  knightMoves(board.pieces[WN + offset], board.occupancies[color], ptr);
  bishopMoves(moves, board.pieces[WB + offset], board.occupancies[color], board.occupancies[BOTH], ptr);
  rookMoves(moves, board.pieces[WR + offset], board.occupancies[color], board.occupancies[BOTH], ptr);
  bishopMoves(moves, board.pieces[WQ + offset], board.occupancies[color], board.occupancies[BOTH], ptr);
  kingMoves(board.pieces[WK + offset], board.occupancies[color], ptr);
  pawnMoves<C>(board.pieces[WP + offset], board.occupancies[color], board.occupancies[BOTH], color, ptr);
  return std::span<u16>(maxMovesList.data(), ptr);
}

template std::span<u16> getMoves<WHITE>(const Board&, u8, std::array<u16, 218>&, MoveTables&);
template std::span<u16> getMoves<BLACK>(const Board&, u8, std::array<u16, 218>&, MoveTables&);