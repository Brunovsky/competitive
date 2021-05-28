#include "test_utils.hpp"
#include "../struct/integer_heaps.hpp"
#include "../lib/graph_generator.hpp"

inline namespace detail {

template <typename Compare>
ostream& operator<<(ostream& out, pairing_int_heap<Compare> heap) {
    vector<int> nums;
    while (!heap.empty())
        nums.push_back(heap.pop());
    return out << nums;
}

template <typename Compare>
ostream& operator<<(ostream& out, binary_int_heap<Compare> heap) {
    vector<int> nums;
    while (!heap.empty())
        nums.push_back(heap.pop());
    return out << nums;
}

} // namespace detail

inline namespace stress_testing_int_heap {

template <template <typename> typename Heap, bool adjust = true>
void stress_test_int_heap(int N = 60) {
    const int K = 100'000;
    assert(2LL * K * N <= INT_MAX);
    int super = N * K + 1, bound = N * K;
    intd numd(0, N - 1), vald(0, K);

    vector<int> weight(N);
    Heap heap(N, greater_container(weight));
    set<int, greater_container<vector<int>>> nums(weight);
    long size_sum = 0;

    auto getnext = [&](int n) {
        if constexpr (adjust) {
            return (void)n, vald(mt) * N + n;
        } else if (weight[n] >= bound) {
            return super++;
        } else {
            return weight[n] = intd(weight[n] / N, K)(mt) * N + n;
        }
    };

    for (int n = 0; n < N; n++) {
        weight[n] = getnext(n);
        heap.push(n), nums.insert(n);
    }
    for (int n = 0; n < N; n++) {
        assert(heap.contains(n));
    }

    reald actiond(0, 1);
    double action;

#define STRESS_VERIFY(ok) verify(ok, #ok, __LINE__)

    auto verify = [&](bool ok, string text, int line) {
        if (!ok) {
            print("{}: Assertion '{}' failed.\nHeap: {}\nNums: {}\nAction: {}\n", line,
                  text, heap, nums, action);
            abort();
        }
    };

    LOOP_FOR_DURATION_OR_RUNS_TRACKED(10s, now, 300'000, runs) {
        print_time(now, 10s, 50ms, "stress heap");
        action = actiond(mt);

        if (action < 0.0000) { // clear
            nums.clear();
            heap.clear();
        } else if (action < 0.40) { // push
            int n = numd(mt);
            if (!nums.count(n)) {
                STRESS_VERIFY(!heap.contains(n));
                weight[n] = getnext(n);
                heap.push(n);
                nums.insert(n);
            }
        } else if (action < 0.80) { // improve or adjust
            int n = numd(mt);
            if (nums.count(n)) {
                STRESS_VERIFY(heap.contains(n));
                nums.erase(n);
                weight[n] = getnext(n);
                if constexpr (adjust) {
                    heap.adjust(n);
                } else {
                    heap.improve(n);
                }
                nums.insert(n);
            }
        } else if (action < 1.0) { // pop
            if (!nums.empty()) {
                STRESS_VERIFY(!heap.empty());
                int n = *nums.begin();
                nums.erase(nums.begin());
                int m = heap.pop();
                STRESS_VERIFY(n == m);
            }
        }

        size_sum += nums.size();

        STRESS_VERIFY(heap.empty() == nums.empty());
        STRESS_VERIFY(heap.empty() || heap.top() == *nums.begin());
    }

    double avg = 1.0 * size_sum / runs;
    print_clear("average size: {:.2f} ({:.2f}%)\n", avg, 100.0 * avg / N);
}

} // namespace stress_testing_int_heap

inline namespace unit_testing_pairing_heaps {

void unit_test_pairing_heaps() {
    constexpr int R = 5, N = 15;

    long cost[N] = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28};
    pairing_int_heaps<greater_container<long*>> heaps(R, N, cost);
    int u;

    heaps.push(0, 1);
    heaps.push(0, 3);

    heaps.push(1, 4);
    heaps.push(1, 5);
    heaps.push(1, 2);
    heaps.push(1, 0);
    u = heaps.pop(1);
    assert(u == 5);

    heaps.merge(0, 1);
    assert(heaps.empty(1));
    u = heaps.pop(0);
    assert(u == 4);
    u = heaps.pop(0);
    assert(u == 3); // 0,1,2 left

    heaps.push(1, 5);
    heaps.push(1, 9);
    u = heaps.pop(1);
    assert(u == 9);

    heaps.push(2, 7);
    heaps.push(2, 8);
    heaps.push(2, 9);
    u = heaps.pop(2);
    assert(u == 9);
    u = heaps.pop(2);
    assert(u == 8);
    assert(heaps.top(2) == 7);

    cost[0] = 5;
    assert(heaps.top(0) == 2);
    heaps.improve(0, 0);
    assert(heaps.top(0) == 0);
    u = heaps.pop(0);
    assert(u == 0);
    u = heaps.pop(0);
    assert(u == 2);

    heaps.push_or_improve(0, 3);
    assert(heaps.top(0) == 3);
}

} // namespace unit_testing_pairing_heaps

int main() {
    RUN_BLOCK((stress_test_int_heap<binary_int_heap, false>()));
    RUN_BLOCK((stress_test_int_heap<pairing_int_heap, false>()));
    RUN_BLOCK((stress_test_int_heap<pairing_int_heap, true>()));
    RUN_SHORT(unit_test_pairing_heaps());
    return 0;
}
