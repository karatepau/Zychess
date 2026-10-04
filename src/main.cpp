#include "bitboard.h"
#include "uci.h"

int main() {
    Board board;
    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) break;
        if (line == "quit") break;
        uci(line, board);
    }
    return 0;
}