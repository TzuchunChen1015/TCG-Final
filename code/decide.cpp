#include "board/board.hpp"
#include <stdio.h>
#include <random>

std::mt19937 random_num(std::random_device{}());

namespace Const {

constexpr int MAX_COLOR_NUM = 2;
constexpr int MAX_DICE_NUM = 6;
constexpr int MAX_PIECE_NUM = 6;
constexpr int MAX_POS_NUM = 25;

constexpr int MAX_SIZE = 5e6;

}

class Zobrist {
private:
  static unsigned int piece_pos[Const::MAX_COLOR_NUM][Const::MAX_PIECE_NUM][Const::MAX_POS_NUM];
  static unsigned int color[Const::MAX_COLOR_NUM];
  static unsigned int dice[Const::MAX_DICE_NUM];
public:
  static void Initialize();
  static unsigned int GetHashValue(const Board& board);
};

class TranspositionTable {
private:
  struct Data {
    double alpha, beta;
    double m;
    int depth;
    int move_id;
  };
public:
  static unsigned int hash_value[Const::MAX_SIZE];
  static Data data[Const::MAX_SIZE];
};

int Board::decide()
{
    generate_moves();
    return random_num() % move_count;
}

/* Zobrist Hash */
unsigned int Zobrist::piece_pos[Const::MAX_COLOR_NUM][Const::MAX_PIECE_NUM][Const::MAX_POS_NUM];
unsigned int Zobrist::color[Const::MAX_COLOR_NUM];
unsigned int Zobrist::dice[Const::MAX_DICE_NUM];

void Zobrist::Initialize() {
  static bool initialized = false;
  if(!initialized) {
    initialized = true;
    for(int clr = 0; clr < Const::MAX_COLOR_NUM; clr++) {
      for(int pcs = 0; pcs < Const::MAX_PIECE_NUM; pcs++) {
        for(int pos = 0; pos < Const::MAX_POS_NUM; pos++) {
          Zobrist::piece_pos[clr][pcs][pos] = random_num();
        }
      }
    }
    for(int c = 0; c < Const::MAX_COLOR_NUM; c++) {
      Zobrist::color[c] = random_num();
    }
    for(int d = 0; d < Const::MAX_DICE_NUM; d++) {
      Zobrist::dice[d] = random_num();
    }
  }
}

unsigned int Zobrist::GetHashValue(const Board& board) {
  unsigned int hash_value = 0;
  for(int clr = 0; clr < Const::MAX_COLOR_NUM; clr++) {
    for(int pcs = 0; pcs < Const::MAX_PIECE_NUM; pcs++) {
      int pos = board.piece_position[clr][pcs];
      if(pos != -1) {
        hash_value ^= Zobrist::piece_pos[clr][pcs][pos];
      }
    }
  }
  hash_value ^= Zobrist::color[board.moving_color];
  hash_value ^= Zobrist::dice[board.dice];
  return hash_value;
}
/* Zobrist Hash */

/* Transposition Table */
unsigned int TranspositionTable::hash_value[Const::MAX_SIZE];
TranspositionTable::Data TranspositionTable::data[Const::MAX_SIZE];
/* Transposition Table */
