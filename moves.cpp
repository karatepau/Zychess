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
void getAttacks(const Board& board, u64& mask, const MoveTables& moves) {
  constexpr int offset = C * 6;
  u64 occ = board.occupancies[BOTH] & ~board.pieces[WK + (C ^ 1) * 6];
  knightAttacks(board.pieces[WN + offset], mask);
  bishopAttacks(moves, board.pieces[WB + offset], occ, mask);
  rookAttacks(moves, board.pieces[WR + offset], occ, mask);
  bishopAttacks(moves, board.pieces[WQ + offset], occ, mask);
  rookAttacks(moves, board.pieces[WQ + offset], occ, mask);
  kingAttacks(board.pieces[WK + offset], mask);
  pawnAttacks<C>(board.pieces[WP + offset], mask);
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
  kingMove(legalMoves, origin, 6 + C, ptr);
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
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, const MoveTables& moveTables) {
  u16* ptr = maxMovesList.data();
  constexpr u8 offset = C * 6;
  constexpr Color enemy = static_cast<Color>(C ^ 1);
  u64 checkMask = 0;
  getAttacks<enemy>(board, checkMask, moveTables);
  knightMoves(board.pieces[WN + offset], board.occupancies[C], ptr);
  bishopMoves(moveTables, board.pieces[WB + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  rookMoves(moveTables, board.pieces[WR + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  bishopMoves(moveTables, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  rookMoves(moveTables, board.pieces[WQ + offset], board.occupancies[C], board.occupancies[BOTH], ptr);
  kingMoves<C>(board.pieces[WK + offset], board.occupancies[C], board.occupancies[BOTH]^board.pieces[WK + offset], ~checkMask, board.extras[board.turn].castlingRights, ptr);
  pawnMoves<C>(board.pieces[WP + offset], board.occupancies[BOTH], board.occupancies[enemy], ptr, 1ULL << board.extras[board.turn].passantSq);
  return std::span<u16>(maxMovesList.data(), ptr);
}

template<Color C>
u64 isCheck(const Board& board, const MoveTables& moveTables) {
  u64 checkMask = 0;
  getAttacks<static_cast<Color>(C ^ 1)>(board, checkMask, moveTables);
  return checkMask & board.pieces[WK + C*6];
}

template<Color C>
void updateBoard(Board& board, u8 piece, u8 destination, u64 oriMask, u64 destMask, u8 passantSq, u8 castlingRights) {
  board.board[destination] = piece;
  board.pieces[piece] |= destMask;
  board.occupancies[C] ^= oriMask | destMask;
  board.occupancies[BOTH] ^= oriMask;
  board.occupancies[BOTH] |= destMask;
  board.extras[board.turn + 1] = board.extras[board.turn];
  board.turn++;
  board.extras[board.turn].passantSq = passantSq;
  board.extras[board.turn].castlingRights = castlingRights;
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
void makeMove(Board& board, u16 movement) {
  constexpr Color E = static_cast<Color>(C ^ 1);
  u8 origin = (movement >> 6) & 0x3f;
  u8 destination = movement & 0x3f;
  u8 flag = movement >> 12;
  u8 piece = board.board[origin];
  u64 oriMask = 1ULL << origin;
  u64 destMask = 1ULL << destination;
  i8 captured = board.board[destination];
  u8 rights = board.extras[board.turn].castlingRights & castleKeep(origin) & castleKeep(destination);
 
  if (flag == 9) {
    u8 capSq = C == WHITE ? destination - 8 : destination + 8;
    u64 c = 1ULL << capSq;
    captured = board.board[capSq];
    board.board[capSq] = -1;
    board.pieces[captured] ^= c;
    board.occupancies[E] ^= c;
    board.occupancies[BOTH] ^= c;
  }
  else if (captured != -1) {
    board.pieces[captured] ^= destMask;
    board.occupancies[E] ^= destMask;
  }
  board.extras[board.turn].capturedPiece = captured;
  board.board[origin] = -1;
  board.pieces[piece] ^= oriMask;
 
  u8 placed = piece;
  u8 passant = 0;
  if (flag >= 1 && flag <= 4) placed = flag + C * 6;
  else if (flag == 5) passant = (origin + destination) / 2;
  updateBoard<C>(board, placed, destination, oriMask, destMask, passant, rights);
 
  if (flag == 6 || flag == 7) {
    u8 from, to;
    if (destination == origin + 2)      { from = origin + 3; to = origin + 1; }
    else if (destination + 2 == origin) { from = origin - 4; to = origin - 1; }
    else return;
    u64 rm = (1ULL << from) | (1ULL << to);
    board.board[from] = -1;
    board.board[to] = WR + C * 6;
    board.pieces[WR + C * 6] ^= rm;
    board.occupancies[C] ^= rm;
    board.occupancies[BOTH] ^= rm;
  }
}


template std::span<u16> getMoves<WHITE>(const Board& board, std::array<u16, 218>& maxMovesList, const MoveTables& moveTables);
template std::span<u16> getMoves<BLACK>(const Board& board, std::array<u16, 218>& maxMovesList, const MoveTables& moveTables);
template u64 isCheck<WHITE>(const Board&, const MoveTables&);
template u64 isCheck<BLACK>(const Board&, const MoveTables&);
template void makeMove<WHITE>(Board&, u16 movement);
template void makeMove<BLACK>(Board&, u16 movement);