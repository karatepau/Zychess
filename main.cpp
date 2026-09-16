#include <stdio.h>
#include "bitboard.h"
#include "moves.h"
#include <iomanip>
#include <iostream>

int main (int argc, char *argv[]) {
  MoveTables moves = gnMoves();
  std::array<u16, 218> maxMovesList;
  Board board{};
  board.pieces[WK] = 0x8000000;
  board.occupancies[WHITE] = 0x20000000000;
  board.occupancies[BOTH] = board.occupancies[WHITE] | board.occupancies[BLACK];
  std::span<u16> movesList = getMoves<WHITE>(board, WHITE, maxMovesList, moves);

  u64 result = 0;
  for (u16 m : movesList) {
      result |= 1ULL << (m >> 6);
  }
  printf("%llu\n", result);
}