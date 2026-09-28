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

using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using Color = i32;
constexpr Color WHITE = 0;
constexpr Color BLACK = 1;
constexpr Color BOTH  = 2;

enum {
  WP = 0, WN, WB, WR, WQ, WK,
  BP, BN, BB, BR, BQ, BK
};

enum CastlingRights : u8
{
  WHITE_OO  = 1,
  WHITE_OOO = 2,
  BLACK_OO  = 4,
  BLACK_OOO = 8
};

struct Extras {
  u16 movement;
  u8 passantSq;
  u8 castlingRights;
  i8 capturedPiece;
};

struct MoveTables {
  std::array<std::vector<u64>, 64> bishop;
  std::array<std::vector<u64>, 64> rook;
};

class Board {
  public:
    Board() {
      pieces_.fill(0);
      occupancies_.fill(0);
      squares_.fill(-1);
      turn_ = 0;
      extras_[0] = {};
    }
    u64 pieces(i32 piece) const {
      return pieces_[piece];
    }
    u64 occupancy(i32 c) const {
      return occupancies_[c];
    }
    i8 operator[](i32 sq) const {
      return squares_[sq];
    }
    u16 ply() const {
      return turn_;
    }
    const Extras& state() const {
      return extras_[turn_];
    }
    
    template<Color C>
    std::span<u16> getMoves(std::array<u16, 218>& maxMovesList, const MoveTables& t) const;
    template<Color C>
    u64 isCheck(const MoveTables& moveTables) const;
    template<Color C>
    void makeMove(u16 movement);
    template<Color C>
    void unmakeMove();

  private:
    std::array<u64, 12> pieces_;
    std::array<u64, 3> occupancies_;
    std::array<i8, 64> squares_;
    u16 turn_;
    std::array<Extras, 2048> extras_;

    template<Color C>
    u64 getAttacks(const MoveTables& t) const;
    template<Color C>
    void updateBoard(u8 piece, u8 destination, u64 oriMask, u64 destMask, u8 passantSq, u8 castlingRights);
};

extern const std::array<u64, 64> nMsk;
extern const std::array<u64, 64> kMsk;
extern const std::array<u64, 64> bMsk;
extern const std::array<u64, 64> rMsk;

MoveTables gnMoves();