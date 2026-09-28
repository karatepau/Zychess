#include "bitboard.h"
#include "moves.h"

void knightAttacks(u64 pieces, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin];
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void bishopAttacks(const MoveTables& moveTables, u64 pieces, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void rookAttacks(const MoveTables& moveTables, u64 pieces, u64 both, u64& mask) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    mask |= legalMoves;
    POPLSB(pieces);
  }
}

void kingAttacks(u64 pieces, u64& mask) {
  u8 origin = __builtin_ctzll(pieces);
  u64 legalMoves = kMsk[origin];
  mask |= legalMoves;
}


template<Color C>
void pawnAttacks(u64 pieces, u64& mask) {
  u64 legalMoves;
  if constexpr (C == WHITE) {
    legalMoves = ((pieces & 0xfefefefefefefefeULL) << 7);
    mask |= legalMoves;
    legalMoves = ((pieces & 0x7f7f7f7f7f7f7f7fULL) << 9);
    mask |= legalMoves;
  }

  else {
    legalMoves = ((pieces & 0x7f7f7f7f7f7f7f7fULL) >> 7);
    mask |= legalMoves;
    legalMoves = ((pieces & 0xfefefefefefefefeULL) >> 9);
    mask |= legalMoves;
  }
}

template<Color C>
u64 Board::getAttacks(const MoveTables& t) const {
  u64 mask = 0;
  constexpr i32 offset = C * 6;
  u64 occ = occupancies_[BOTH] & ~pieces_[WK + (C ^ 1) * 6];
  knightAttacks(pieces_[WN + offset], mask);
  bishopAttacks(t, pieces_[WB + offset], occ, mask);
  rookAttacks(t, pieces_[WR + offset], occ, mask);
  bishopAttacks(t, pieces_[WQ + offset], occ, mask);
  rookAttacks(t, pieces_[WQ + offset], occ, mask);
  kingAttacks(pieces_[WK + offset], mask);
  pawnAttacks<C>(pieces_[WP + offset], mask);
  return mask;
}

void moves(u64 legalMoves, u8 origin, u16*& ptr) {
  while (legalMoves) {
    *ptr++ = (origin << 6) | __builtin_ctzll(legalMoves);
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

void bishopMoves(const MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void rookMoves(const MoveTables& moveTables, u64 pieces, u64 friends, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = moveTables.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~friends;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void kingMove(u64 legalMoves, u8 origin, u8 flag, u16*& ptr) {
  while (legalMoves) {
    *ptr++ = (flag << 12) | (origin << 6) | __builtin_ctzll(legalMoves);
    POPLSB(legalMoves);
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
    if ((0x1c00000000000000ULL & legalSquares & ~both) == 0x1c00000000000000ULL && (0x200000000000000ULL & ~both) == 0x200000000000000ULL && (castlingRights & BLACK_OOO) == BLACK_OOO) {
      legalMoves |= 0x400000000000000ULL;
    }
  }
  kingMove(legalMoves, origin, 6, ptr);
}

void pawnMove (u64& legalMoves, u8 flag, u16*& ptr, i8 destDiff) {
  while (legalMoves) {
    u8 destination = __builtin_ctzll(legalMoves);
    u8 origin = destination + destDiff;
    if (1ULL << destination & 0xff000000000000ffULL) {
      for (u8 i = 1; i < 5; i++) {
        *ptr++ = (i << 12) | (origin << 6) | destination;
      }
    }
    else {
      *ptr++ = (flag << 12) | (origin << 6) | destination;
    }
    POPLSB(legalMoves);
  }
}

template<Color C>
void pawnMoves(u64 pieces, u64 both, u64 enemies, u16*& ptr, u64 passantSq) {
  u64 empty = ~both;
  u64 legalMoves;
  if constexpr (C == WHITE) {  
    legalMoves = (pieces << 8) & empty;
    pawnMove(legalMoves, 0, ptr, -8);
    legalMoves = ((((pieces & 0xff00ULL) << 8) & empty) << 8) & empty;
    pawnMove(legalMoves, 5, ptr, -16);
    legalMoves = ((pieces & 0xfefefefefefefefeULL) << 7) & enemies;
    pawnMove(legalMoves, 0, ptr, -7);
    legalMoves = ((pieces & 0x7f7f7f7f7f7f7f7fULL) << 9) & enemies;
    pawnMove(legalMoves, 0, ptr, -9);
    legalMoves = ((pieces & 0xfe00000000ULL) << 7) & passantSq;
    pawnMove(legalMoves, 9, ptr, -7);
    legalMoves = ((pieces & 0x7f00000000ULL) << 9) & passantSq;
    pawnMove(legalMoves, 9, ptr, -9);
  }

  else {
    legalMoves = (pieces >> 8) & empty;
    pawnMove(legalMoves, 0, ptr, 8);
    legalMoves = ((((pieces & 0xff000000000000ULL) >> 8) & empty) >> 8) & empty;
    pawnMove(legalMoves, 5, ptr, 16);
    legalMoves = ((pieces & 0x7f7f7f7f7f7f7f7fULL) >> 7) & enemies;
    pawnMove(legalMoves, 0, ptr, 7);
    legalMoves = ((pieces & 0xfefefefefefefefeULL) >> 9) & enemies;
    pawnMove(legalMoves, 0, ptr, 9);
    legalMoves = ((pieces & 0x7f000000ULL) >> 7) & passantSq;
    pawnMove(legalMoves, 9, ptr, 7);
    legalMoves = ((pieces & 0xfe000000ULL) >> 9) & passantSq;
    pawnMove(legalMoves, 9, ptr, 9);
  }
}

template<Color C>
std::span<u16> Board::getMoves(std::array<u16, 218>& maxMovesList, const MoveTables& t) const {
  u16* ptr = maxMovesList.data();
  constexpr u8 offset = C * 6;
  constexpr Color enemy = C ^ 1;
  u64 checkMask = getAttacks<enemy>(t);
  knightMoves(pieces_[WN + offset], occupancies_[C], ptr);
  bishopMoves(t, pieces_[WB + offset], occupancies_[C], occupancies_[BOTH], ptr);
  rookMoves(t, pieces_[WR + offset], occupancies_[C], occupancies_[BOTH], ptr);
  bishopMoves(t, pieces_[WQ + offset], occupancies_[C], occupancies_[BOTH], ptr);
  rookMoves(t, pieces_[WQ + offset], occupancies_[C], occupancies_[BOTH], ptr);
  kingMoves<C>(pieces_[WK + offset], occupancies_[C], occupancies_[BOTH]^pieces_[WK + offset], ~checkMask, extras_[turn_].castlingRights, ptr);
  pawnMoves<C>(pieces_[WP + offset], occupancies_[BOTH], occupancies_[enemy], ptr, 1ULL << extras_[turn_].passantSq);
  return std::span<u16>(maxMovesList.data(), ptr);
}

template<Color C>
u64 Board::isCheck(const MoveTables& t) const {
  return getAttacks<C ^ 1>(t) & pieces_[WK + C * 6];
}

template<Color C>
void Board::updateBoard(u8 piece, u8 destination, u64 oriMask, u64 destMask, u8 passantSq, u8 castlingRights) {
  squares_[destination] = piece;
  pieces_[piece] |= destMask;
  occupancies_[C] ^= oriMask | destMask;
  occupancies_[BOTH] ^= oriMask;
  occupancies_[BOTH] |= destMask;
  extras_[turn_].passantSq = passantSq;
  extras_[turn_].castlingRights = castlingRights;
}

static constexpr u8 castleKeep(int sq) {
  switch (sq) {
    case 0:  return ~WHITE_OOO;
    case 7:  return ~WHITE_OO;
    case 4:  return ~(WHITE_OO | WHITE_OOO);
    case 56: return ~BLACK_OOO;
    case 63: return ~BLACK_OO;
    case 60: return ~(BLACK_OO | BLACK_OOO);
    default: return 0xff;
  }
}
 
template<Color C>
void Board::makeMove(u16 movement) {
  extras_[turn_+1] = extras_[turn_];
  turn_++;
  extras_[turn_].movement = movement;
  constexpr Color E = static_cast<Color>(C ^ 1);
  u8 origin = (movement >> 6) & 0x3f;
  u8 destination = movement & 0x3f;
  u8 flag = movement >> 12;
  u8 piece = squares_[origin];
  u64 oriMask = 1ULL << origin;
  u64 destMask = 1ULL << destination;
  i8 captured = squares_[destination];
  u8 rights = extras_[turn_].castlingRights & castleKeep(origin) & castleKeep(destination);
 
  if (flag == 9) {
    u8 capSq = C == WHITE ? destination - 8 : destination + 8;
    u64 c = 1ULL << capSq;
    captured = squares_[capSq];
    squares_[capSq] = -1;
    pieces_[captured] ^= c;
    occupancies_[E] ^= c;
    occupancies_[BOTH] ^= c;
  }
  else if (captured != -1) {
    pieces_[captured] ^= destMask;
    occupancies_[E] ^= destMask;
  }
  extras_[turn_].capturedPiece = captured;
  squares_[origin] = -1;
  pieces_[piece] ^= oriMask;
 
  u8 placed = piece;
  u8 passant = 0;
  if (flag >= 1 && flag <= 4) placed = flag + C * 6;
  else if (flag == 5) passant = (origin + destination) / 2;
  updateBoard<C>(placed, destination, oriMask, destMask, passant, rights);
 
  if (flag == 6) {
    u8 from, to;
    if (destination == origin + 2) {
      from = origin + 3;
      to = origin + 1;
    }
    else if (destination + 2 == origin) {
      from = origin - 4;
      to = origin - 1;
    }
    else return;
    u64 rm = (1ULL << from) | (1ULL << to);
    squares_[from] = -1;
    squares_[to] = WR + C * 6;
    pieces_[WR + C * 6] ^= rm;
    occupancies_[C] ^= rm;
    occupancies_[BOTH] ^= rm;
  }
}

template<Color C>
void Board::unmakeMove () {
  constexpr u8 offset = C * 6;
  constexpr Color E = static_cast<Color>(C ^ 1);
  constexpr u8 eOffset = E * 6;
  u8 origin = (extras_[turn_].movement >> 6) & 0x3f;
  u8 destination = extras_[turn_].movement & 0x3f;
  u8 flag = extras_[turn_].movement >> 12;
  u64 oriMask = 1ULL << origin;
  u64 destMask = 1ULL << destination;
  u64 mask = oriMask | destMask;

  if (flag == 9) {
    u8 capSq = C == WHITE ? destination - 8 : destination + 8;
    u64 c = 1ULL << capSq;
    squares_[origin] = offset;
    squares_[capSq] = eOffset;
    squares_[destination] = -1;
    pieces_[eOffset] ^= c;
    pieces_[offset] ^= mask;
    occupancies_[C] ^= mask;
    occupancies_[E] ^= c;
    occupancies_[BOTH] ^= mask | c;
  }
  else {
    if (extras_[turn_].capturedPiece != -1) {
      if (flag > 0 && flag < 5) {
        squares_[origin] = offset;
        squares_[destination] = extras_[turn_].capturedPiece;
        pieces_[extras_[turn_].capturedPiece] ^= destMask;
        occupancies_[E] ^= destMask;
        occupancies_[C] ^= mask;
        pieces_[offset] ^= oriMask;
        pieces_[flag+offset] ^= destMask;
        occupancies_[BOTH] ^= oriMask;
        turn_--;
        return;
      }
      squares_[origin] = squares_[destination];
      squares_[destination] = extras_[turn_].capturedPiece;
      pieces_[extras_[turn_].capturedPiece] ^= destMask;
      occupancies_[E] ^= destMask;
      occupancies_[C] ^= mask;
      pieces_[squares_[origin]] ^= mask;
      occupancies_[BOTH] ^= oriMask;
    }
    else {
      if (flag == 6) {
        if (destination == 2 && origin == 4) {
          pieces_[WR+offset] ^= 0x9;
          occupancies_[C] ^= 0x9;
          occupancies_[BOTH] ^= 0x9;
          squares_[0] = WR+offset;
          squares_[3] = -1;
        }
        else if (destination == 6 && origin == 4) {
          pieces_[WR+offset] ^= 0xa0;
          occupancies_[C] ^= 0xa0;
          occupancies_[BOTH] ^= 0xa0;
          squares_[7] = WR+offset;
          squares_[5] = -1;
        }
        else if (destination == 58 && origin == 60) {
          pieces_[WR+offset] ^= 0x900000000000000;
          occupancies_[C] ^= 0x900000000000000;
          occupancies_[BOTH] ^= 0x900000000000000;
          squares_[56] = WR+offset;
          squares_[59] = -1;
        }
        else if (destination == 62 && origin == 60) {
          pieces_[WR+offset] ^= 0xa000000000000000;
          occupancies_[C] ^= 0xa000000000000000;
          occupancies_[BOTH] ^= 0xa000000000000000;
          squares_[63] = WR+offset;
          squares_[61] = -1;
        }
      }
      else if (flag > 0 && flag < 5) {
        squares_[origin] = offset;
        squares_[destination] = -1;
        occupancies_[C] ^= mask;
        pieces_[offset] ^= oriMask;
        pieces_[flag+offset] ^= destMask;
        occupancies_[BOTH] ^= mask;
        turn_--;
        return;
      }
      squares_[origin] = squares_[destination];
      squares_[destination] = -1;
      occupancies_[C] ^= mask;
      pieces_[squares_[origin]] ^= mask;
      occupancies_[BOTH] ^= mask;
    }
  }
  turn_--;
}


template std::span<u16> Board::getMoves<WHITE>(std::array<u16, 218>&, const MoveTables&) const;
template std::span<u16> Board::getMoves<BLACK>(std::array<u16, 218>&, const MoveTables&) const;
template u64 Board::isCheck<WHITE>(const MoveTables&) const;
template u64 Board::isCheck<BLACK>(const MoveTables&) const;
template void Board::makeMove<WHITE>(u16);
template void Board::makeMove<BLACK>(u16);
template void Board::unmakeMove<WHITE>();
template void Board::unmakeMove<BLACK>();