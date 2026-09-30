#include <stdio.h>
#include "bitboard.h"
#include "moves.h"
#include "uci.h"
#include "search.h"
#include <iomanip>
#include <iostream>

int main() {
    Board board;
    uci("fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
    std::cout << board.pieces(BK) << '\n';
    return 0;
}