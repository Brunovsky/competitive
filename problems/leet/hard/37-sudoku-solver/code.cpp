#include <bits/stdc++.h>

using namespace std;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// *****

struct State {
  vector<vector<u16>> board;
  bool set[9][9] {};
  bool row[9][10] {};
  bool col[9][10] {};
  bool sqr[3][3][10] {};
  u16 free[9][9] {};
  size_t count = 0;
};

constexpr inline u16 lowest_set_bit(u16 set) {
  return __builtin_ctz(set);
}

constexpr inline u16 count_set_bits(u16 set) {
  return __builtin_popcount(set);
}

bool assign(State& state, vector<tuple<u16, u16, u16>> cascaded) {
  while (!cascaded.empty()) {
    auto [r, c, n] = cascaded.back();
    cascaded.pop_back();

    state.board[r][c] = n;
    state.set[r][c] = true;
    state.row[r][n] = true;
    state.col[c][n] = true;
    state.sqr[r / 3][c / 3][n] = true;
    if (++state.count == 81) return true;

    for (u16 i = 0; i < 9; ++i) {
      if (!state.set[r][i]) {
        u16 k = state.free[r][i] & ~(1 << n);
        if (k != state.free[r][i]) {
          if (k == 0) return false;
          state.free[r][i] = k;
          if ((k & (k - 1)) == 0) {
            cascaded.push_back({r, i, lowest_set_bit(k)});
          }
        }
      }
    }

    for (u16 i = 0; i < 9; ++i) {
      if (!state.set[i][c]) {
        u16 k = state.free[i][c] & ~(1 << n);
        if (k != state.free[i][c]) {
          if (k == 0) return false;
          state.free[i][c] = k;
          if ((k & (k - 1)) == 0) {
            cascaded.push_back({i, c, lowest_set_bit(k)});
          }
        }
      }
    }

    for (u16 i = 0, rs = (r / 3) * 3; i < 3; ++i) {
      for (u16 j = 0, cs = (c / 3) * 3; j < 3; ++j) {
        if (!state.set[rs + i][cs + j]) {
          u16 k = state.free[rs + i][cs + j] & ~(1 << n);
          if (k != state.free[rs + i][cs + j]) {
            if (k == 0) return false;
            state.free[rs + i][cs + j] = k;
            if ((k & (k - 1)) == 0) {
              cascaded.push_back({rs + i, cs + j, lowest_set_bit(k)});
            }
          }
        }
      }
    }
  }

  return true;
}

class Solution {
  bool dive(State& state) {
    u16 best_r = 0, best_c = 0, best_set_bits = 10;

    for (u16 r = 0; r < 9; ++r) {
      for (u16 c = 0; c < 9; ++c) {
        if (state.board[r][c] == 0) {
          u16 set_bits = count_set_bits(state.free[r][c]);
          if (set_bits < best_set_bits) {
            best_r = r;
            best_c = c;
            best_set_bits = set_bits;
            if (set_bits == 2) goto assign_dive;
          }
        }
      }
    }

    if (best_set_bits == 10) return true;

  assign_dive:

    while (state.free[best_r][best_c] > 0) {
      u16 n = lowest_set_bit(state.free[best_r][best_c]);
      state.free[best_r][best_c] &= ~(1 << n);

      State new_state = state;

      if (assign(new_state, {{best_r, best_c, n}})) {
        if (state.count == 81 || dive(new_state)) {
          state = std::move(new_state);
          return true;
        }
      }
    }

    return false;
  }

 public:
  vector<vector<u16>> into_u16(const vector<vector<char>>& char_board) const {
    size_t rlen = char_board.size(), clen = char_board[0].size();
    vector<vector<u16>> u16_board(rlen);

    for (size_t r = 0; r < rlen; ++r) {
      u16_board[r].resize(clen);
      for (size_t c = 0; c < clen; ++c) {
        u16_board[r][c] = char_board[r][c] == '.' ? 0 : (char_board[r][c] - '0');
      }
    }

    return u16_board;
  }

  vector<vector<char>> into_char(const vector<vector<u16>>& u16_board) const {
    size_t rlen = u16_board.size(), clen = u16_board[0].size();
    vector<vector<char>> char_board(rlen);

    for (size_t r = 0; r < rlen; ++r) {
      char_board[r].resize(clen);
      for (size_t c = 0; c < clen; ++c) {
        char_board[r][c] = '0' + u16_board[r][c];
      }
    }

    return char_board;
  }

  void solveSudoku(vector<vector<char>>& board) {
    State state;
    state.board = into_u16(board);
    state.count = 0;

    for (u16 r = 0; r < 9; ++r) {
      for (u16 c = 0; c < 9; ++c) {
        u16 n = state.board[r][c];
        if (n > 0) {
          state.set[r][c] = true;
          state.row[r][n] = true;
          state.col[c][n] = true;
          state.sqr[r / 3][c / 3][n] = true;
          state.free[r][c] = (1 << n);
          ++state.count;
        }
      }
    }

    vector<tuple<u16, u16, u16>> cascaded;

    for (u16 r = 0; r < 9; ++r) {
      for (u16 c = 0; c < 9; ++c) {
        if (state.board[r][c] == 0) {
          for (u16 n = 1; n <= 9; ++n) {
            if (!state.row[r][n] && !state.col[c][n] && !state.sqr[r / 3][c / 3][n]) {
              state.free[r][c] |= (1 << n);
            }
          }
          assert(state.free[r][c] != 0);
          if (count_set_bits(state.free[r][c]) == 1) {
            cascaded.push_back({r, c, lowest_set_bit(state.free[r][c])});
          }
        }
      }
    }

    assign(state, cascaded);
    if (state.count < 81) dive(state);
    board = into_char(state.board);
  }
};

// *****

void test() {
  vector<vector<char>> board = {
    {'.', '.', '9', '7', '4', '8', '.', '.', '.'},
    {'7', '.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '2', '.', '1', '.', '9', '.', '.', '.'},

    {'.', '.', '7', '.', '.', '.', '2', '4', '.'},
    {'.', '6', '4', '.', '1', '.', '5', '9', '.'},
    {'.', '9', '8', '.', '.', '.', '3', '.', '.'},

    {'.', '.', '.', '8', '.', '3', '.', '2', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.', '6'},
    {'.', '.', '.', '2', '7', '5', '9', '.', '.'},
  };

  Solution S;

  S.solveSudoku(board);

  for (u16 r = 0; r < 9; ++r) {
    for (u16 c = 0; c < 9; ++c) {
      cout << board[r][c] << ' ';
    }
    cout << endl;
  }
}

// *****

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  test();
  return 0;
}