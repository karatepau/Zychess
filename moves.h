#include <span>

template<Color C>
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);
template<Color C>
void getAttacks(const Board& board, u64& mask, MoveTables& moveTables);