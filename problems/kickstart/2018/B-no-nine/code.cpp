#include <bits/stdc++.h>

using namespace std;

// *****

using u64 = uint64_t;

// n^k
u64 pown(u64 n, int k) {
  u64 power = 1;
  while (k--) {
    power *= n;
  }
  return power;
}

u64 no_nine(u64 n) {
  vector<u64> digits;
  u64 m = 1;
  while (m <= n) {
    digits.push_back((n / m) % 10);
    m *= 10;
  }

  int num_digits = digits.size();
  int sum = 0; // sum of digits
  u64 count = 0;

  for (int k = num_digits; k >= 2; k--) {
    int digit = digits[k - 1];
    assert(digit != 9); // problem statement
    count += digit * 8 * pown(9, k - 2);
    sum = (sum + digit) % 9;
  }

  int D = digits[0];
  assert(D != 9); // problem statement
  for (int d = 0; d <= D; d++) {
    count += ((sum + d) % 9) != 0;
  }
  return count;
}

auto solve() {
  u64 F, L;
  cin >> F >> L >> ws;
  return no_nine(L) - no_nine(F) + 1;
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
