#include "uci.h"
#include "search.h"

i32 sqToIndex(const std::string& square) {
  i32 x = square[0] - 'a';
  i32 y = square[1] - '1';
  return y * 8 + x;
}

i64 getValueOf (std::vector<std::string>& parameters, std::string word) {
  for (i32 i = 0; i+1 < parameters.size(); i++) {
    if (word == parameters[i]) {
      return std::stoll(parameters[i + 1]);
    }
  }
  return 0;
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

void UCIToMove(std::string cords, Board& board) {
  i32 origin = sqToIndex(cords.substr(0, 2));
  i32 destination = sqToIndex(cords.substr(2, 2));
  i32 coronedPiece = 0;
  if (cords.size() == 5) {
    switch(cords[4]) {
      case 'n': coronedPiece = 1; break;
      case 'b': coronedPiece = 2; break;
      case 'r': coronedPiece = 3; break;
      case 'q': coronedPiece = 4; break;
    }
  }
  i32 C;
  MoveList moves;
  if (board.turn_ & BLACK) {
    C = BLACK;
    moves = board.getMoves<BLACK>();
  }
  else {
    C = WHITE;
    moves = board.getMoves<WHITE>();
  }
  for (u16 move : moves) {
    i32 moveOrigin = (move >> 6) & 0x3f;
    i32 moveDest = move & 0x3f;
    i32 moveCoronedPiece = ((move >> 12) < 5) ? move >> 12 : 0;
    if (origin == moveOrigin && destination == moveDest && coronedPiece == moveCoronedPiece) {
      if (C) board.makeMove<BLACK>(move);
      else board.makeMove<WHITE>(move);
      return;
    }
  }
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
  if (turn_ & 1) extras_[turn_].zobristHash = getZobristHash<BLACK>();
  else extras_[turn_].zobristHash = getZobristHash<WHITE>();
}


void uci (const std::string& command, Board& board) {
  searchState.nodes = 0;
  std::istringstream stream(command);
  std::vector<std::string> parameters;
  std::string parameter;
  while (stream >> parameter) {
      parameters.push_back(parameter);
  }
  if (parameters.empty()) return;
  if (parameters.size() > 1 && parameters[0] == "position") {
    if (parameters[1] == "startpos") {
      uci("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
      if (parameters.size() > 2 && parameters[2] == "moves") {
        for (i32 i = 3; i < parameters.size(); i++) {
          UCIToMove(parameters[i], board);
        }
      }
    }
    else if (parameters[1] == "fen") {
      board.fenLoader(parameters);
      if (parameters.size() > 8 && parameters[8] == "moves") {
        for (i32 i = 9; i < parameters.size(); i++) {
          UCIToMove(parameters[i], board);
        }
      }
    }
  }
  else if (parameters[0] == "go") {
    if (parameters.size() > 2 && parameters[1] == "depth") {
      i32 depth = std::stoi(parameters[2]);
      u16 bestMove;
      if (board.turn_ & 1) {bestMove = getBestMove<BLACK>(board, depth, 0);}
      else {bestMove = getBestMove<WHITE>(board, depth, 0);}
      std::cout << "bestmove " << moveToUCI(bestMove) << '\n';
    }
    else {
      searchState.startTime = std::chrono::steady_clock::now();
      i64 wTime = getValueOf(parameters, "wtime"),
      bTime = getValueOf(parameters, "btime"),
      //wInc = getValueOf(parameters, "winc"),
      //bInc = getValueOf(parameters, "binc"),
      moveTime = getValueOf(parameters, "movetime");
      if (!moveTime) {
        if (board.turn_ & BLACK) {
          searchState.timeLimit = bTime / 20;
        }
        else {
          searchState.timeLimit = wTime / 20;
        }
      }
      else {searchState.timeLimit = moveTime;}
      u16 bestMove = iDeeping(board);
      std::cout << "bestmove " << moveToUCI(bestMove) << '\n';
    }
  }
  else if (parameters[0] == "uci") {
    std::cout << "id name Zychess" << '\n';
    std::cout << "id author karatepau" << '\n';
    std::cout << "uciok" << '\n';
  }
  else if (parameters[0] == "isready") {
    std::cout << "readyok" << '\n';
  }
  else if (parameters[0] == "ucinewgame") {
    board.reset();
    std::fill(tt.begin(), tt.end(), TT{});
  }
}