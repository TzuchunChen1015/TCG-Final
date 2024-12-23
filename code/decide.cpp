#include "board/board.hpp"
#include <stdio.h>
#include <random>

std::mt19937 random_num(std::random_device{}());

namespace Const {

constexpr int MAX_COLOR_NUM = 2;
constexpr int MAX_DICE_NUM = 6;
constexpr int MAX_PIECE_NUM = 6;
constexpr int MAX_POS_NUM = 25;

constexpr double VAL_MAX = 100.0;
constexpr double VAL_MIN = -100.0;

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

class Solve {
private:
  static constexpr int chebyshev[Const::MAX_COLOR_NUM][Const::MAX_POS_NUM] = {
    {
      4, 4, 4, 4, 4,
      4, 3, 3, 3, 3,
      4, 3, 2, 2, 2,
      4, 3, 2, 1, 1,
      4, 3, 2, 1, 0
    },
    {
      0, 1, 2, 3, 4,
      1, 1, 2, 3, 4,
      2, 2, 2, 3, 4,
      3, 3, 3, 3, 4,
      4, 4, 4, 4, 4
    }
  };
  static constexpr double C = 6.0;

  static double Evaluate(Board& board);
  static double StarOne(const Board& board, double alpha, double beta, int depth);
public:
  static double NegaScout(Board& board, double alpha, double beta, int depth);
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

/* NegaScout and Star1 */
double Solve::Evaluate(Board& board) {
  if(board.check_winner()) {
    return Const::VAL_MIN;
  } else {
    double value = 0.0;
    int opponent = (board.moving_color ^ 1);
    for(int pcs = 0; pcs < Const::MAX_PIECE_NUM; pcs++) {
      int pos = board.piece_position[board.moving_color][pcs];
      if(pos != -1) {
        value -= Solve::chebyshev[board.moving_color][pos];
      }
      int op_pos = board.piece_position[opponent][pcs];
      if(op_pos != -1) {
        value += Solve::chebyshev[opponent][op_pos];
      }
    }
    return value;
  }
}

double Solve::StarOne(const Board& board, double alpha, double beta, int depth) {
  double A = Solve::C * (alpha - Const::VAL_MAX) + Const::VAL_MAX;
  double B = Solve::C * (beta - Const::VAL_MIN) + Const::VAL_MIN;
  double m = Const::VAL_MIN;
  double M = Const::VAL_MAX;
  double val_sum = 0.0;
  for(char d = 0; d < Const::MAX_DICE_NUM; d++) {
    Board child = board;
    child.dice = d;
    double t = Solve::NegaScout(child, std::max(A, Const::VAL_MIN), std::min(B, Const::VAL_MAX), depth);
    m += (t - Const::VAL_MIN) / Solve::C;
    M += (t - Const::VAL_MAX) / Solve::C;
    if(t >= B) {        // Fail High
      return m; 
    } else if(t <= A) { // Fail Low
      return M; 
    }
    val_sum += t;
    A += Const::VAL_MAX - t;
    B += Const::VAL_MIN - t;
  }
  return val_sum / Solve::C;
}

double Solve::NegaScout(Board& board, double alpha, double beta, int depth) {
  unsigned int hash_value = Zobrist::GetHashValue(board);
  int hash_index = hash_value % Const::MAX_SIZE;
  if(TranspositionTable::hash_value[hash_index] == hash_value && TranspositionTable::data[hash_index].depth >= depth) {
    return TranspositionTable::data[hash_index].m;
  }
  TranspositionTable::hash_value[hash_index] = hash_value;
  TranspositionTable::data[hash_index].alpha = alpha;
  TranspositionTable::data[hash_index].beta = beta;
  TranspositionTable::data[hash_index].depth = depth;
  if(board.check_winner() || !depth) {
    return TranspositionTable::data[hash_index].m = Solve::Evaluate(board);
  }
  board.generate_moves();
  double m = Const::VAL_MIN;
  double n = beta;
  for(int move_id = 0; move_id < board.move_count; move_id++) {
    Board child = board;
    child.move(move_id);
    double t = -Solve::StarOne(child, -n, -std::max(alpha, m), depth - 1);
    if(t > m) {
      if(n == beta || t >= beta) {
        m = t;
      } else {
        m = -Solve::StarOne(child, -beta, -t, depth - 1);
      }
      TranspositionTable::data[hash_index].move_id = move_id;
    }
    if(m >= beta) {
      return TranspositionTable::data[hash_index].m = m;
    }
    n = std::max(alpha, m) + 1;
  }
  return TranspositionTable::data[hash_index].m = m;
}
/* NegaScout and Star1 */
