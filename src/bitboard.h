#pragma once
#include <span>
#include <bit>
#include <cstdint>
#include <array>
#include <cstdio>
#include <span>
#include <stdbit.h>
#include <immintrin.h>
#include <vector>
#include <bit>
#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <random>

using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using Color = i32;
#define POPLSB(x) ((x) &= ((x) - 1))
constexpr i32 INF = 1000000;
constexpr Color WHITE = 0;
constexpr Color BLACK = 1;
constexpr Color BOTH  = 2;

enum {
  WP = 0, WN, WB, WR, WQ, WK,
  BP, BN, BB, BR, BQ, BK
};

enum CastlingRights : u8 {
  WHITE_OO  = 1,
  WHITE_OOO = 2,
  BLACK_OO  = 4,
  BLACK_OOO = 8
};

enum {
  EXACT = 0,
  LOWER,
  UPPER,
};

struct ZobristKeys {
  std::array<std::array<u64, 64>, 12> pieces;
  u64 side;
  std::array<u64, 16> castle;
  std::array<u64, 8> enPassantFile;
};

struct TT {
  u64 zobristHash;
  u8 depth;
  i32 score;
  u16 bestMove;
  u8 scoreType;
};

struct Extras {
  u16 movement;
  u8 passantSq;
  u8 castlingRights;
  i8 capturedPiece;
  u64 zobristHash;
  u16 fiftyMovesRule;
};

struct MoveTables {
  std::array<std::vector<u64>, 64> bishop;
  std::array<std::vector<u64>, 64> rook;
};

struct MoveList {
    std::array<u16, 218> data;
    i32 count = 0;
    u16* begin() {return data.data();}
    u16* end() {return data.data() + count;}
};

class Board {
  public:
    Board() {reset();}
    std::array<u64, 12> pieces_;
    std::array<u64, 3> occupancies_;
    std::array<i8, 64> squares_;
    u16 turn_;
    std::array<Extras, 2048> extras_;
    template<Color C>
    u64 getAttackers(u8 square) const;
    template<Color C>
    MoveList getMoves() const;
    template<Color C>
    MoveList getNoQuietMoves() const;
    template<Color C>
    u64 isCheck() const;
    template<Color C>
    void makeMove(u16 movement);
    template<Color C>
    void unmakeMove();
    void fenLoader(const std::vector<std::string>& fen);
    template<Color C>
    u64 getAttacks() const;
    template<Color C>
    void updateBoard(u8 piece, u8 destination, u64 oriMask, u64 destMask, u8 passantSq, u8 castlingRights);
    template<Color C>
    void updateZobristHash();
    template<Color C>
    u64 getZobristHash();
    void reset() {
      pieces_.fill(0);
      occupancies_.fill(0);
      squares_.fill(-1);
      turn_ = 0;
      extras_[0] = {};
    }
};

extern const std::array<u64, 64> nMsk;
extern const std::array<u64, 64> kMsk;
extern const std::array<u64, 64> bMsk;
extern const std::array<u64, 64> rMsk;
extern const MoveTables t;
extern const ZobristKeys zobristKeys;
inline std::vector<TT> tt(0x100000);