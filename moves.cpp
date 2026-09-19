#include "bitboard.h"
#include "moves.h"

void attacks(u64 legalMoves, u64& mask) {
  mask |= 1ULL << __builtin_ctzll(legalMoves);
}

void knightAttacks(u64 pieces, u64 board, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~board;
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void bishopAttacks(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void rookAttacks(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void kingAttacks(u64 pieces, u64 board, u64& mask) {
  u8 origin = __builtin_ctzll(pieces);
  u64 legalMoves = kMsk[origin] & ~board;
  mask |= legalMoves;
}


template<Color C>
void pawnAttacks(u64 pieces, u64& mask) {
  u64 legalMoves;
  if constexpr (C == WHITE) {
    legalMoves = (pieces << 7);
    mask |= legalMoves;
    legalMoves = (pieces << 9);
    mask |= legalMoves;
  }

  else {
    legalMoves = (pieces >> 7);
    mask |= legalMoves;
    legalMoves = (pieces >> 9);
    mask |= legalMoves;
  }
}

template<Color C>
void getAttacks(const Board& board, u64& mask, MoveTables& moves) {
  constexpr int offset = C * 6;
  knightAttacks(board.pieces[WN + offset], board.occupancies[C], mask);
  bishopAttacks(moves, board.pieces[WB + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  rookAttacks(moves, board.pieces[WR + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  bishopAttacks(moves, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  rookAttacks(moves, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], mask);
  kingAttacks(board.pieces[WK + offset], board.occupancies[C], mask);
  pawnAttacks<C>(board.pieces[WP + offset], mask);
}

void moves(u64 legalMoves, u8 origin, u16*& ptr) {
  while (legalMoves) {
    *ptr++ = (__builtin_ctzll(legalMoves) << 6) | origin;
    POPLSB(legalMoves);
  }
}
void knightMoves(u64 pieces, u64 board, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~board;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void bishopMoves(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void rookMoves(MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

template<Color C>
void kingMoves(u64 pieces, u64 friends, u64 both, u64 legalSquares, u8 castlingRights, u16*& ptr) {
  u8 origin = __builtin_ctzll(pieces);
  u64 legalMoves = kMsk[origin] & ~friends & legalSquares;
  if constexpr (C == WHITE) {
    if ((0x70u & legalSquares & ~both) == 0x70u && (castlingRights & WHITE_OO) == WHITE_OO) {
      legalMoves |= 0x40;
    }
    if ((0x1c & legalSquares & ~both) == 0x1c && (0x02ULL & ~both) == 0x02ULL && (castlingRights & WHITE_OOO) == WHITE_OOO) {
      legalMoves |= 0x4;
    }
  }

  else {
    if ((0x7000000000000000ULL & legalSquares & ~both) == 0x7000000000000000ULL && (castlingRights & BLACK_OO) == BLACK_OO) {
      legalMoves |= 0x4000000000000000ULL;
    }
    if ((0x1c00000000000000ULL & legalSquares & ~both) == 0x1c00000000000000ULL && (0x200000000000002ULL & ~both) == 0x200000000000002ULL && (castlingRights & BLACK_OOO) == BLACK_OOO) {
      legalMoves |= 0x400000000000000ULL;
    }
  }
  moves(legalMoves, origin, ptr);
}

void pawnMove (u64& legalMoves, u16*& ptr, i8 destDiff) {
  while (legalMoves) {
    u8 destination = __builtin_ctzll(legalMoves);
    u8 origin = destination + destDiff;
    *ptr++ = (origin << 6) | destination;
    POPLSB(legalMoves);
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
  kingMoves<C>(board.pieces[WK + offset], board.occupancies[C], board.occupancies[BOTH]^board.pieces[WK + offset], ~checkMask, board.extras.castlingRights, ptr);
  pawnMoves<C>(board.pieces[WP + offset], board.occupancies[BOTH], board.occupancies[enemy], ptr, u64 (board.extras.passantSq << 1ULL));
  return std::span<u16>(maxMovesList.data(), ptr);
}

template std::span<u16> getMoves<WHITE>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);
template std::span<u16> getMoves<BLACK>(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);
