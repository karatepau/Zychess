#include "bitboard.h"
#include "moves.h"

template<Color C>
u64 Board::getAttackers(u8 square) const {
  constexpr u8 offset = C * 6;
  u64 mask = t.bishop[square][_pext_u64(occupancies_[BOTH], bMsk[square])] & pieces_[WB + offset];
  mask |= t.rook[square][_pext_u64(occupancies_[BOTH], rMsk[square])] & pieces_[WR + offset];
  mask |= t.bishop[square][_pext_u64(occupancies_[BOTH], bMsk[square])] & pieces_[WQ + offset];
  mask |= t.rook[square][_pext_u64(occupancies_[BOTH], rMsk[square])] & pieces_[WQ + offset];
  mask |= nMsk[square] & pieces_[WN + offset];
  mask |= kMsk[square] & pieces_[WK + offset];
  if constexpr (C == WHITE) {
    mask |= ((1ULL << square & 0x7f7f7f7f7f7f7f7fULL) >> 7) & pieces_[WP];
    mask |= ((1ULL << square & 0xfefefefefefefefeULL) >> 9) & pieces_[WP];
  }
  else {
    mask |= ((1ULL << square & 0xfefefefefefefefeULL) << 7) & pieces_[BP];
    mask |= ((1ULL << square & 0x7f7f7f7f7f7f7f7fULL) << 9) & pieces_[BP];
  }
  return mask;
}

void moves(u64 legalMoves, u8 origin, u16*& ptr) {
  while (legalMoves) {
    *ptr++ = (origin << 6) | __builtin_ctzll(legalMoves);
    POPLSB(legalMoves);
  }
}

void knightCaptures(u64 pieces, u64 enemies, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & enemies;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void bishopCaptures(u64 pieces, u64 enemies, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = t.bishop[origin][_pext_u64(both, bMsk[origin])] & enemies;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void rookCaptures(u64 pieces, u64 enemies, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = t.rook[origin][_pext_u64(both, rMsk[origin])] & enemies;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
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
void pawnNoQuietMoves(u64 pieces, u64 empty, u64 enemies, u16*& ptr, u64 passantSq) {
  u64 legalMoves;
  if constexpr (C == WHITE) {  
    legalMoves = (pieces << 8) & empty & 0xff000000000000ffULL;
    if (legalMoves) {pawnMove(legalMoves, 0, ptr, -8);}
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
    legalMoves = (pieces >> 8) & empty & 0xff000000000000ffULL;
    if (legalMoves) {pawnMove(legalMoves, 0, ptr, 8);}
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

void knightQuietMoves(u64 pieces, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = nMsk[origin] & ~both;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void bishopQuietMoves(u64 pieces, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = t.bishop[origin][_pext_u64(both, bMsk[origin])];
    legalMoves &= ~both;
    moves(legalMoves, origin, ptr);
    POPLSB(pieces);
  }
}

void rookQuietMoves(u64 pieces, u64 both, u16*& ptr) {
  while (pieces) {
    u8 origin = __builtin_ctzll(pieces);
    u64 legalMoves = t.rook[origin][_pext_u64(both, rMsk[origin])];
    legalMoves &= ~both;
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
void kingNoQuietMoves(const Board& board, u16*& ptr) {
  constexpr i32 offset = C * 6;
  u8 origin = __builtin_ctzll(board.pieces_[WK + offset]);
  u64 legalMoves = kMsk[origin] & board.occupancies_[C ^ 1];
  kingMove(legalMoves, origin, 6, ptr);
}

template<Color C>
void kingQuietMoves(const Board& board, u16*& ptr) {
  constexpr i32 offset = C * 6;
  u8 origin = __builtin_ctzll(board.pieces_[WK + offset]);
  u64 legalMoves = kMsk[origin] & ~board.occupancies_[BOTH];
  constexpr i32 enemy = C ^ 1;
  if constexpr (C == WHITE) {
    if (!(0x60ULL & board.occupancies_[BOTH]) && (board.extras_[board.turn_].castlingRights & WHITE_OO) && !board.getAttackers<enemy>(4) && !board.getAttackers<enemy>(5) && !board.getAttackers<enemy>(6)) {
      legalMoves |= 0x40;
    }
    if (!(0xeULL & board.occupancies_[BOTH]) && (board.extras_[board.turn_].castlingRights & WHITE_OOO) && !board.getAttackers<enemy>(4) && !board.getAttackers<enemy>(3) && !board.getAttackers<enemy>(2)) {
      legalMoves |= 0x4;
    }
  }

  else {
    if (!(0x6000000000000000ULL & board.occupancies_[BOTH]) && (board.extras_[board.turn_].castlingRights & BLACK_OO) && !board.getAttackers<enemy>(60) && !board.getAttackers<enemy>(61) && !board.getAttackers<enemy>(62)) {
      legalMoves |= 0x4000000000000000ULL;
    }
    if (!(0xe00000000000000ULL & board.occupancies_[BOTH]) && (board.extras_[board.turn_].castlingRights & BLACK_OOO)  && !board.getAttackers<enemy>(60) && !board.getAttackers<enemy>(59) && !board.getAttackers<enemy>(58)) {
      legalMoves |= 0x400000000000000ULL;
    }
  }
  kingMove(legalMoves, origin, 6, ptr);
}

template<Color C>
void pawnQuietMoves(u64 pieces, u64 both, u64 enemies, u16*& ptr) {
  u64 empty = ~both;
  u64 legalMoves;
  if constexpr (C == WHITE) {  
    legalMoves = (pieces << 8) & empty & ~0xff00000000000000ULL;
    pawnMove(legalMoves, 0, ptr, -8);
    legalMoves = ((((pieces & 0xff00ULL) << 8) & empty) << 8) & empty;
    pawnMove(legalMoves, 5, ptr, -16);
    legalMoves = ((pieces & 0xfefefefefefefefeULL) << 7) & enemies;
  }

  else {
    legalMoves = (pieces >> 8) & empty & ~0xffULL;
    pawnMove(legalMoves, 0, ptr, 8);
    legalMoves = ((((pieces & 0xff000000000000ULL) >> 8) & empty) >> 8) & empty;
    pawnMove(legalMoves, 5, ptr, 16);
    legalMoves = ((pieces & 0x7f7f7f7f7f7f7f7fULL) >> 7) & enemies;
  }
}

template<Color C>
MoveList Board::getMoves() const {
  MoveList moves;
  u16* ptr = moves.data.data();
  constexpr u8 offset = C * 6;
  constexpr Color enemy = C ^ 1;
  if (std::popcount(getAttackers<C ^ 1>(__builtin_ctzll(pieces_[WK + offset]))) < 2) {
    pawnNoQuietMoves<C>(pieces_[WP + offset], ~occupancies_[BOTH], occupancies_[enemy], ptr, 1ULL << extras_[turn_].passantSq);
    knightCaptures(pieces_[WN + offset], occupancies_[enemy], ptr);
    bishopCaptures(pieces_[WB + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
    rookCaptures(pieces_[WR + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
    bishopCaptures(pieces_[WQ + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
    rookCaptures(pieces_[WQ + offset], occupancies_[enemy], occupancies_[BOTH], ptr);

    bishopQuietMoves(pieces_[WQ + offset], occupancies_[BOTH], ptr);
    rookQuietMoves(pieces_[WQ + offset], occupancies_[BOTH], ptr);
    rookQuietMoves(pieces_[WR + offset], occupancies_[BOTH], ptr);
    bishopQuietMoves(pieces_[WB + offset], occupancies_[BOTH], ptr);
    knightQuietMoves(pieces_[WN + offset], occupancies_[BOTH], ptr);
    pawnQuietMoves<C>(pieces_[WP + offset], occupancies_[BOTH], occupancies_[enemy], ptr);
  }
  kingNoQuietMoves<C>(*this, ptr);
  kingQuietMoves<C>(*this, ptr);
  moves.count = ptr - moves.data.data();
  return moves;
}

template<Color C>
MoveList Board::getNoQuietMoves() const {
  MoveList moves;
  u16* ptr = moves.data.data();
  constexpr u8 offset = C * 6;
  constexpr Color enemy = C ^ 1;
  pawnNoQuietMoves<C>(pieces_[WP + offset], ~occupancies_[BOTH], occupancies_[enemy], ptr, 1ULL << extras_[turn_].passantSq);
  knightCaptures(pieces_[WN + offset], occupancies_[enemy], ptr);
  bishopCaptures(pieces_[WB + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
  rookCaptures(pieces_[WR + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
  bishopCaptures(pieces_[WQ + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
  rookCaptures(pieces_[WQ + offset], occupancies_[enemy], occupancies_[BOTH], ptr);
  kingNoQuietMoves<C>(*this, ptr);
  moves.count = ptr - moves.data.data();
  return moves;
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
  extras_[turn_].zobristHash = extras_[turn_-1].zobristHash;
  extras_[turn_].zobristHash ^= zobristKeys.castle[extras_[turn_-1].castlingRights];
  extras_[turn_].zobristHash ^= zobristKeys.castle[rights];
  extras_[turn_].zobristHash ^= zobristKeys.side;
  extras_[turn_].zobristHash ^= zobristKeys.pieces[piece][origin];
  extras_[turn_].zobristHash ^= zobristKeys.pieces[piece][destination];
  if (piece == C * 6 || captured!=-1) extras_[turn_].fiftyMovesRule = 0;
  else extras_[turn_].fiftyMovesRule++;
  if (extras_[turn_].passantSq) {
    constexpr u64 direction = (C == WHITE) ? -8 : 8;
    u64 pushedPos = 1ULL << (extras_[turn_].passantSq + direction);
    u64 adjacentMask = ((pushedPos & 0xfefefefefefefefeULL) >> 1) | ((pushedPos & 0x7f7f7f7f7f7f7f7fULL) << 1);
    if (adjacentMask & pieces_[C * 6]) {
      extras_[turn_].zobristHash ^= zobristKeys.enPassantFile[extras_[turn_].passantSq & 7];
    }
  }
  if (flag == 9) {
    u8 capSq = C == WHITE ? destination - 8 : destination + 8;
    u64 c = 1ULL << capSq;
    captured = squares_[capSq];
    squares_[capSq] = -1;
    pieces_[captured] ^= c;
    occupancies_[E] ^= c;
    occupancies_[BOTH] ^= c;
    extras_[turn_].zobristHash ^= zobristKeys.pieces[captured][capSq];
  }
  else if (captured != -1) {
    extras_[turn_].zobristHash ^= zobristKeys.pieces[captured][destination];
    pieces_[captured] ^= destMask;
    occupancies_[E] ^= destMask;
  }
  extras_[turn_].capturedPiece = captured;
  squares_[origin] = -1;
  pieces_[piece] ^= oriMask;
 
  u8 placed = piece;
  u8 passant = 0;
  if (flag >= 1 && flag <= 4) {
    placed = flag + C * 6;
    extras_[turn_].zobristHash ^= zobristKeys.pieces[piece][destination];
    extras_[turn_].zobristHash ^= zobristKeys.pieces[flag + C * 6][destination];
  }
  else if (flag == 5) {
    passant = (origin + destination) / 2;
    u64 adjacentMask = ((destMask & 0xfefefefefefefefeULL) >> 1) | ((destMask & 0x7f7f7f7f7f7f7f7fULL) << 1);
    if (adjacentMask & pieces_[E * 6]) {
      extras_[turn_].zobristHash ^= zobristKeys.enPassantFile[passant & 7];
    }
  }
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
    extras_[turn_].zobristHash ^= zobristKeys.pieces[WR + C * 6][from];
    extras_[turn_].zobristHash ^= zobristKeys.pieces[WR + C * 6][to];
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


template MoveList Board::getMoves<WHITE>() const;
template MoveList Board::getMoves<BLACK>() const;
template MoveList Board::getNoQuietMoves<WHITE>() const;
template MoveList Board::getNoQuietMoves<BLACK>() const;
template u64 Board::getAttackers<WHITE>(u8 square) const;
template u64 Board::getAttackers<BLACK>(u8 square) const;
template void Board::makeMove<WHITE>(u16);
template void Board::makeMove<BLACK>(u16);
template void Board::unmakeMove<WHITE>();
template void Board::unmakeMove<BLACK>();