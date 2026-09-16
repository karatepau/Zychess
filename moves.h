#include <span>

template<Color C>
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moves, u8 passantSq);