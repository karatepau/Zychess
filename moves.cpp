#include "bitboard.h"


void attacks(u64 legalMoves, u64& mask) {
  while (legalMoves) {
    mask |= 1ULL << __builtin_ctzll(legalMoves);
    legalMoves &= legalMoves-1;
  }
}

void knightAttacks(u64 pieces, u64 board, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~board;
    attacks(legalMoves, mask);
    pieces &= pieces-1;
  }
}

void bishopAttacks(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    attacks(legalMoves, mask);
    pieces &= pieces-1;
  }
}

void rookAttacks(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    attacks(legalMoves, mask);
    pieces &= pieces-1;
  }
}

void kingAttacks(u64 pieces, u64 board, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = kMsk[origin] & ~board;
    attacks(legalMoves, mask);
    pieces &= pieces-1;
  }
}


template<Color C>
void pawnAttacks(u64 pieces, u64 both, u64 enemies, u64& mask) {
  u64 empty = ~both;
  u64 legalMoves;
  if constexpr (C == WHITE) {
    legalMoves = (pieces << 7);
    attacks(legalMoves, mask);
    legalMoves = (pieces << 9);
    attacks(legalMoves, mask);
  }

  else {
    legalMoves = (pieces >> 7);
    attacks(legalMoves, mask);
    legalMoves = (pieces >> 9);
    attacks(legalMoves, mask);
  }
}

template<Color C>
void getAttacks(const Board& board, u64& mask, MoveTables& moves) {
  constexpr int offset = C * 6;
  knightAttacks(board.pieces[WN + offset], board.occupancies[C], mask);
  bishopAttacks(moves, board.pieces[WB + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  rookAttacks(moves, board.pieces[WR + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  bishopAttacks(moves, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  kingAttacks(board.pieces[WK + offset], board.occupancies[C], mask);
  pawnAttacks<C>(board.pieces[WP + offset], board.occupancies[BOTH], board.occupancies[C ^ 1ULL], mask);
}

void moves(u64 legalMoves, u8 origin, u16*& ptr) {
  while (legalMoves) {
    *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
    legalMoves &= legalMoves-1;
  }
}
void knightMoves(u64 pieces, u64 board, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~board;
    moves(legalMoves, origin, ptr);
    pieces &= pieces-1;
  }
}

void bishopMoves(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    pieces &= pieces-1;
  }
}

void rookMoves(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    pieces &= pieces-1;
  }
}

void kingMoves(u64 pieces, u64 board, u64 legalMoves, u16*& ptr) {
  u8 origin = __builtin_ctzll(pieces);
  legalMoves &= kMsk[origin] & ~board;
  moves(legalMoves, origin, ptr);
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
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables) {
  u16* ptr = maxMovesList.data();
  constexpr u8 offset = C * 6;
  constexpr Color enemy = static_cast<Color>(C ^ 1);
  u64 checkMask = 0;
  getAttacks<enemy>(board, checkMask, moveTables);
  knightMoves(board.pieces[WN + offset], board.occupancies[C], ptr);
  bishopMoves(moveTables, board.pieces[WB + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  rookMoves(moveTables, board.pieces[WR + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  bishopMoves(moveTables, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  kingMoves(board.pieces[WK + offset], board.occupancies[C], ~checkMask, ptr);
  pawnMoves<C>(board.pieces[WP + offset], board.occupancies[BOTH], board.occupancies[enemy], ptr, u64 (board.extras.passantSq << 1ULL));
  return std::span<u16>(maxMovesList.data(), ptr);
}

template void getAttacks<WHITE>(const Board& board, u64& mask, MoveTables& moveTables);
template void getAttacks<BLACK>(const Board& board, u64& mask, MoveTables& moveTables);
template std::span<u16> getMoves<WHITE>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);
template std::span<u16> getMoves<BLACK>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);
