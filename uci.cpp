#include "uci.h"

i32 sqToIndex(const std::string& square) {
  i32 x = square[0] - 'a';
  i32 y = square[1] - '1';
  return y * 8 + x;
}

std::string moveToUCI(u16 move) {
  if (move == 0) return "0000";
  u8 origin = (move >> 6) & 63;
  u8 destination = move & 63;
  u8 flag = move >> 12;

  std::string s;
  s += static_cast<char>('a' + (origin & 7));
  s += static_cast<char>('1' + (origin >> 3));
  s += static_cast<char>('a' + (destination & 7));
  s += static_cast<char>('1' + (destination >> 3));
  if (flag >= 1 && flag <= 4) s += "nbrq"[flag - 1];
  return s;
}

void Board::fenLoader(const std::vector<std::string>& fen) {
  reset();
  i32 c = 56;
  for (char piece : fen[2]) {
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
    if (squares_[i] == -1) continue;
    u64 bit = 1ULL << i;
    pieces_[squares_[i]] |= bit;
    occupancies_[squares_[i] < 6 ? WHITE : BLACK] |= bit;
    occupancies_[BOTH] |= bit;
  }
  turn_ = (fen[3][0] == 'b');

  Extras& e = extras_[turn_];
  e = {};
  if (fen[4].find('K') != std::string::npos) e.castlingRights |= WHITE_OO;
  if (fen[4].find('Q') != std::string::npos) e.castlingRights |= WHITE_OOO;
  if (fen[4].find('k') != std::string::npos) e.castlingRights |= BLACK_OO;
  if (fen[4].find('q') != std::string::npos) e.castlingRights |= BLACK_OOO;
  e.passantSq = (fen[5] == "-") ? 0 : sqToIndex(fen[5]);
}


void uci (const std::string& command, Board& board) {
  std::istringstream stream(command);
  std::vector<std::string> parameters;
  std::string parameter;
  while (stream >> parameter) {
      parameters.push_back(parameter);
  }
  if (parameters[0] == "position") {
    if (parameters[1] == "startpos") {
      uci("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
    }
    else if (parameters[1] == "fen") {
      board.fenLoader(parameters);
    }
  }
  else if (parameters[0] == "go") {
    if (parameters[1] == "depth") {
      i32 depth = std::stoi(parameters[2]);
      u16 bestMove;
      if (board.ply() & 1) {bestMove = board.getBestMove<BLACK>(depth);}
      else {bestMove = board.getBestMove<WHITE>(depth);}
      std::cout << "bestmove " << moveToUCI(bestMove) << '\n';
    }
  }
}