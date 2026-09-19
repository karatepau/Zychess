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

using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

enum {
  WP = 0, WN, WB, WR, WQ, WK,
  BP, BN, BB, BR, BQ, BK
};

enum Color {
  WHITE = 0,
  BLACK = 1,
  BOTH  = 2
};

enum CastlingRights : u8
{
  WHITE_OO  = 1,
  WHITE_OOO = 2,
  BLACK_OO  = 4,
  BLACK_OOO = 8
};

struct Extras {
  u8 passantSq;
  u8 castlingRights;
  u8 capturedPiece;
};

struct Board{
  u64 pieces[12];
  u64 occupancies[3];
  u64 board[64];
  Extras extras;
};

struct MoveTables {
  std::array<std::vector<u64>, 64> bishop;
  std::array<std::vector<u64>, 64> rook;
};

extern const std::array<u64, 64> nMsk;
extern const std::array<u64, 64> kMsk;
extern const std::array<u64, 64> bMsk;
extern const std::array<u64, 64> rMsk;

MoveTables gnMoves();