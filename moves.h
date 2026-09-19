#define POPLSB(x) ((x) &= ((x) - 1))
template<Color C>
std::span<u16> getMoves(const Board& board, std::array<u16, 218>& maxMovesList, MoveTables& moveTables);