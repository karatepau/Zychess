#include "bitboard.h"

void knightMoves(u64 pieces, u64 board, u8 const color, u16*& ptr) {
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

void bishopMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u8 const color, u16*& ptr) {
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

void rookMoves(MoveTables& moves, u64 pieces, u64 friends, u64 both, u8 const color, u16*& ptr) {
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

void kingMoves(u64 pieces, u64 board, u8 const color, u16*& ptr) {
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

template<Color C>
void pawnMoves(u64 pieces, u64 board, u8 const color, u16*& ptr) {
  u64 empty = ~both
  if constexpr (C == WHITE) {
    u64 legalMoves = (pieces << 8) & empty; //aquí while para que guarde como origen una casilla atrás
    while (legalMoves) {
      i8 destination = __builtin_ctzll(legalMoves);
      *ptr++ = (destination << 6) | (destination - 8);
      legalMoves &= legalMoves-1;
    }
    legalMoves = ((((pieces & 0xff00ULL) << 8) & empty) << 8) & empty; //aquí while para que guarde como origen 2 casillas atras
    while (legalMoves) {
      i8 destination = __builtin_ctzll(legalMoves);
      *ptr++ = (destination << 6) | (destination - 16);
      legalMoves &= legalMoves-1;
    }
  }

  else {
    u64 legalMoves = (pieces >> 8) & empty; //aquí while para que guarde como origen una casilla atrás
    while (legalMoves) {
      i8 destination = __builtin_ctzll(legalMoves);
      *ptr++ = (destination << 6) | (destination + 8);
      legalMoves &= legalMoves-1;
    }
    legalMoves = ((((pieces & 0xff000000000000ULL) >> 8) & empty) >> 8) & empty; //aquí while para que guarde como origen 2 casillas atras
    while (legalMoves) {
      i8 destination = __builtin_ctzll(legalMoves);
      *ptr++ = (destination << 6) | (destination + 16);
      legalMoves &= legalMoves-1;
    }
  }
}

std::span<u16> getMoves(const Board& board, u8 const color, std::array<u16, 218>& maxMovesList, MoveTables& moves) {
  u16* ptr = maxMovesList.data();
  knightMoves(board.pieces[WN + (color*6)], board.occupancies[color], color, ptr);
  bishopMoves(moves, board.pieces[WB + (color*6)], board.occupancies[color], board.occupancies[BOTH], color, ptr);
  rookMoves(moves, board.pieces[WR + (color*6)], board.occupancies[color], board.occupancies[BOTH], color, ptr);
  bishopMoves(moves, board.pieces[WQ + (color*6)], board.occupancies[color], board.occupancies[BOTH], color, ptr);
  kingMoves(board.pieces[WK + (color*6)], board.occupancies[color], color, ptr);
  pawnMoves<Color>
  return std::span<u16>(maxMovesList.data(), ptr);
}