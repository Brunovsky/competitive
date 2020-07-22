#include <bits/stdc++.h>

using namespace std;
using u32 = uint32_t;

// *****

struct Stone {
  u32 S, E, I;
};

u32 N;
vector<Stone> stones;

auto sum() {
  u32 maxT = 0;
  for (const Stone &stone : stones)
    maxT += stone.S;
  return maxT;
}

// *****

auto solve() {
  cin >> N >> ws;
  stones.assign(N, {});

  for (u32 n = 0; n < N; ++n) {
    cin >> stones[n].S >> stones[n].E >> stones[n].I >> ws;
    assert(stones[n].S > 0);
  }

  auto maxT = sum();

  sort(stones.begin(), stones.end(), [](const Stone &Ci, const Stone &Cj) {
    return Ci.I * Cj.S > Cj.I * Ci.S;
  });

  vector<u32> tail_energy(maxT + 1, 0);
  vector<u32> head_energy(maxT + 1, 0);

  // using only stones[k..], at time t what is the maximum energy we can get?

  for (u32 _loop = N, k = N - 1; _loop > 0; --k, --_loop) {
    u32 t = 0;
    Stone stone = stones[k];

    while (t <= maxT - stone.S) {
      u32 decay = stone.I * t;
      if (decay >= stone.E)
        break;
      u32 energy = stone.E - decay;
      head_energy[t] = max(tail_energy[t + stone.S] + energy, tail_energy[t]);
      ++t;
    }

    while (t < maxT) {
      head_energy[t] = tail_energy[t];
      ++t;
    }

    tail_energy = head_energy;
  }

  return tail_energy[0];
}

// *****

int main() {
  unsigned T;
  cin >> T >> ws;
  for (unsigned t = 1; t <= T; ++t) {
    auto solution = solve();
    cout << "Case #" << t << ": " << solution << '\n';
  }
  return 0;
}
