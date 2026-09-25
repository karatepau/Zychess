#pragma once
#include "bitboard.h"
#define POPLSB(x) ((x) &= ((x) - 1))

template<Color C>
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, const MoveTables& moveTables);
template<Color C>
u64 isCheck(const Board& board, const MoveTables& moveTables);
template<Color C>
void makeMove(Board& board, u16 movement);
template<Color C>
void unmakeMove(Board& board);