#include "uci.h"

void Board::fenLoader(const std::vector<std::string>& fen) {
  i32 c = 56;
  for (char piece : fen[1]) {
    switch (piece) {
      case 'P': squares_[c++] = WP; break;
      case 'N': squares_[c++] = WN; break;
      case 'B': squares_[c++] = WB; break;
      case 'R': squares_[c++] = WR; break;
      case 'Q': squares_[c++] = WQ; break;
      case 'K': squares_[c++] = WK; break;

      case 'p': squares_[c++] = BP; break;
      case 'n': squares_[c++] = BN; break;
      case 'b': squares_[c++] = BB; break;
      case 'r': squares_[c++] = BR; break;
      case 'q': squares_[c++] = BQ; break;
      case 'k': squares_[c++] = BK; break;

      case '/': c -= 16; break;

      case '1': c += 1; break;
      case '2': c += 2; break;
      case '3': c += 3; break;
      case '4': c += 4; break;
      case '5': c += 5; break;
      case '6': c += 6; break;
      case '7': c += 7; break;
      case '8': c += 8; break;
    }
  }
  for (u64 i = 0; i < 64; i++) {
    if (squares_[i] != -1) {
      pieces_[squares_[i]] |= 1ULL << i;
    }
  }
}


void uci (const std::string& command, Board& board) {
  std::istringstream stream(command);
  std::vector<std::string> parameters;
  std::string parameter;
  while (stream >> parameter) {
      parameters.push_back(parameter);
  }
  if (parameters[0] == "fen") {
    if (parameters[1] == "startpos") {

    }
    else {
      board.fenLoader(parameters);
    }
  }
}