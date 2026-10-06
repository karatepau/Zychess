#pragma once
#include "bitboard.h"
std::string moveToUCI(u16 move);
void uci(const std::string& command, Board& board);