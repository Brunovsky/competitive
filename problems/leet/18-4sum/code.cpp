#include <bits/stdc++.h>

using namespace std;

// *****

class Solution {
public:
  vector<vector<int>> fourSum(vector<int> &nums, int target) const {
    sort(nums.begin(), nums.end());
    set<vector<int>> solutions;
    int N = nums.size();

    for (int i = 0; i + 3 < N; ++i) {
      for (int j = i + 1; j + 2 < N; ++j) {
        int k = j + 1, l = N - 1;
        while (k < l) {
          int sum = nums[i] + nums[j] + nums[k] + nums[l];
          if (sum == target)
            solutions.insert({nums[i], nums[j], nums[k++], nums[l--]});
          else if (sum < target)
            ++k;
          else
            --l;
        }
      }
    }

    vector<vector<int>> answer;
    answer.reserve(solutions.size());
    for (auto it = solutions.begin(); it != solutions.end();) {
      answer.push_back(move(solutions.extract(it++).value()));
    }
    return answer;
  }
};

// *****

void test() {
  vector<int> nums = {1, 0, -1, 0, -2, 2};
  auto answer = Solution{}.fourSum(nums, 0);
  for (const vector<int> &quad : answer)
    cout << '[' << quad[0] << ',' << quad[1] << ',' //
         << quad[2] << ',' << quad[3] << "]\n";
  cout << "All tests passed \033[1;32m" << u8"\u2713" << "\033[0m\n";
}

// *****

int main() {
  test();
  return 0;
}
