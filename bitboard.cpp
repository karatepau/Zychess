#include "bitboard.h"

consteval std::array<u64, 64> gnBMsk () {
    std::array<u64, 64> msk{};
    u64 NE = 0x40201008040200;
    u64 NW = 0X2040810204000;
    i8 x;
    i8 y;
    i8 shNE;
    i8 shNW;
    u64 NEMsk;
    u64 NWMsk;

    for (u8 i = 0; i < 64; i++) {
        x = i & 7;
        y = (i >> 3);
        shNE = x - y;
        shNW = x + y - 7;
        NEMsk = (shNE >= 0) ? (NE >> (shNE*8)) : (NE << (-shNE*8));
        NWMsk = (shNW >= 0) ? (NW << (shNW*8)) : (NW >> (-shNW*8));
        msk[i] = (0x7e7e7e7e7e7e00ULL & (NEMsk^NWMsk));
    }
    return msk;
}

consteval std::array<u64, 64> gnRMsk () {
    std::array<u64, 64> msk{};
    u64 NSMsk = 0x1010101010100;
    u64 EWMsk = 0x7e;
    u64 fullMv;

    for (u64 i = 0; i < 64; i++) {
        msk[i] = ((NSMsk << (i & 7)) | (EWMsk << (i & 0x78))) & ~(1ULL << i);
    }
    return msk;
}

consteval std::array<u64, 64> gnNMsk () {
    std::array<u64, 64> msk{};
    u64 x[] = {
        0x606060606060606,
        0xd0d0d0d0d0d0d0d,
        0x1b1b1b1b1b1b1b1b,
        0x3636363636363636,
        0x6c6c6c6c6c6c6c6c,
        0xd8d8d8d8d8d8d8d8,
        0xb0b0b0b0b0b0b0b0,
        0x6060606060606060

    };

    u64 y[] = {
        0xffff00,
        0xffff00ff,
        0xffff00ffff,
        0xffff00ffff00,
        0xffff00ffff0000,
        0xffff00ffff000000,
        0xff00ffff00000000,
        0xffff0000000000

    };
    u64 fullMv;

    for (u64 i = 0; i < 64; i++) {
        fullMv = std::rotl(0x442800000028440ULL, i);
        fullMv &= x[i & 7] & y[i >> 3];
        msk[i] = fullMv;
    }
    return msk;

}


consteval std::array<u64, 64> gnKMsk () {

    std::array<u64, 64> msk{};
    u64 x[] = {
        0x303030303030303,
        0x707070707070707,
        0xe0e0e0e0e0e0e0e,
        0x1c1c1c1c1c1c1c1c,
        0x3838383838383838,
        0x7070707070707070,
        0xe0e0e0e0e0e0e0e0,
        0xc0c0c0c0c0c0c0c0
    };

    u64 y[] = {
        0xffff,
        0xffffff,
        0xffffff00,
        0xffffff0000,
        0xffffff000000,
        0xffffff00000000,
        0xffffff0000000000,
        0xffff000000000000
    }; 

    u64 fullMv;
    for (u64 i = 0; i < 64; i++) {
        fullMv = std::rotl(0x8380000000000382, i);
        fullMv &= x[i & 7] & y[i >> 3];
        msk[i] = fullMv;
    }
    return msk;
}

u64 raycasting (u64 pos, u64 occ, u64 mask, int shift) {
  u64 moves = pos;
  while (!(pos & mask) && !(pos & occ)) {
    pos = shift >= 0 ? pos << shift : pos >> -shift;
    if (pos == 0) break;
    moves |= pos;
  }
  return moves;
}

u64 bRaycasting (u64 pos, u64 occ) {
  u64 legalMoves = raycasting(pos, occ, 0xff80808080808080, 9);
  legalMoves |= raycasting(pos, occ, 0xff01010101010101, 7);
  legalMoves |= raycasting(pos, occ, 0x1010101010101ff, -7);
  legalMoves |= raycasting(pos, occ, 0x80808080808080ff, -9);
  return legalMoves;
}

u64 rRaycasting (u64 pos, u64 occ) {
  u64 legalMoves = raycasting(pos, occ, 0xff00000000000000, 8);
  legalMoves |= raycasting(pos, occ, 0x8080808080808080, 1);
  legalMoves |= raycasting(pos, occ, 0xff, -8);
  legalMoves |= raycasting(pos, occ, 0x101010101010101, -1);
  return legalMoves;
}

const std::array<u64, 64> nMsk = gnNMsk();
const std::array<u64, 64> kMsk = gnKMsk();
const std::array<u64, 64> bMsk = gnBMsk();
const std::array<u64, 64> rMsk = gnRMsk();

MoveTables gnMoves () {
  u64 occupancy = 0;
  MoveTables moves;
  for (i8 i = 0; i < 64; i++) {
    u64 size = 1ULL << std::popcount(bMsk[i]);
    moves.bishop[i].resize(size);
    do {
      occupancy = (occupancy - bMsk[i]) & bMsk[i];
      moves.bishop[i][_pext_u64(occupancy, bMsk[i])] = bRaycasting(1ULL << i, occupancy);
    } while (occupancy > 0);
  }
  occupancy = 0;
  for (i8 i = 0; i < 64; i++) {
    u64 size = 1ULL << std::popcount(rMsk[i]);
    moves.rook[i].resize(size);
    do {
      occupancy = (occupancy - rMsk[i]) & rMsk[i];
      moves.rook[i][_pext_u64(occupancy, rMsk[i])] = rRaycasting(1ULL << i, occupancy);
    } while (occupancy > 0);
  }
  return moves;
}