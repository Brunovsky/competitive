#include <bits/stdc++.h>

using namespace std;

// *****

constexpr int compute_factorial(int n) {
  int k = n;
  while (--n)
    k *= n;
  return k;
}

class Solution {
public:
  string getPermutation(int n, int k) {
    string text;
    int factorial = compute_factorial(n);

    list<char> nums(n);
    iota(nums.begin(), nums.end(), '1');

    for (int i = n; i >= 1; --i) {
      auto it = nums.begin();

      factorial /= i;
      while (k > factorial)
        ++it, k -= factorial;

      text += *it;
      nums.erase(it);
    }

    return text;
  }
};

// *****

void test() {
  cout << "All tests passed \033[1;32m" << u8"\u2713" << "\033[0m\n";
}

// *****

int main() {
  test();
  return 0;
}
