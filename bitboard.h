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
using i8  = int8_t;
using u8  = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;

enum {
    WP = 0, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK
};

enum Color {
    WHITE = 0,
    BLACK = 1,
    BOTH  = 2
};

struct Extras {
    u8 passantSq;
    u8 wKingRights;
    u8 bKingRights;
    u8 fiftyRule;
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