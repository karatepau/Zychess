#include <stdio.h>
#include "bitboard.h"
#include "moves.h"
#include <iomanip>
#include <iostream>

int main (int argc, char *argv[]) {
  MoveTables moveTables = gnMoves();
  std::array<u16, 218> maxMovesList;
  Board board{};
  std::fill_n(board.board, 64, -1);
}