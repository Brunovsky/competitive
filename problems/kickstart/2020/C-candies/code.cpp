#include <bits/stdc++.h>

using namespace std;

// *****

/**
 * Use 0-based exclusive end indexing for everything (as usual)
 *
 * The idea: 4 segments trees
 *
 *     A            0  1  2  3  4  5  6  7  8  9
 * STree even       1     1     1     1     1
 * STree odd           1     1     1     1     1
 * STree inc_even   0     2     4     6     8
 * STree inc_odd       1     3     5     7     9
 *
 * Then to answer a Query [L, R[ perform 4 range queries in each tree
 * and get A, B, C, D. The answer will be ±(C - D - (L - 1)(A - B)).
 */

struct data_t {
    long even = 0, odd = 0, inc_even = 0, inc_odd = 0;
};

data_t add(const data_t &lhs, const data_t &rhs) {
    return data_t{lhs.even + rhs.even, lhs.odd + rhs.odd,
                  lhs.inc_even + rhs.inc_even, lhs.inc_odd + rhs.inc_odd};
}

void update_data(data_t &data, int i, long v) {
    data.even = (i & 1) ? 0 : v;
    data.odd = (i & 1) ? v : 0;
    data.inc_even = (i & 1) ? 0 : i * v;
    data.inc_odd = (i & 1) ? i * v : 0;
}

// use only one tree in the actual implementation, obviously
struct node_t {
    int L, R;
    data_t data;
};

// as usual, P is the highest power of 2 leq 2N.
int N, P;
vector<node_t> tree;

// Pushup update at tree leaf node i
void pushup_update(int i) {
    assert(i >= N);
    while ((i >>= 1) > 0) {
        tree[i].data = add(tree[i << 1].data, tree[i << 1 | 1].data);
    }
}

// Update a value at array index i and pushup the update in the tree
void update(int ai, long v) {
    int ti = ai < (2 * N - P) ? P + ai : P + ai - N;
    update_data(tree[ti].data, ai, v);
    pushup_update(ti);
}

// Perform all range queries in the range [L, R[
data_t range_query(int i, int L, int R) {
    auto iL = tree[i].L, iR = tree[i].R;

    // [L, R] overlaps [iL, iR]?
    if (iR <= L || iL >= R)
        return {};
    // [L, R] contains [iL, iR]?
    if (L <= iL && iR <= R)
        return tree[i].data;
    // they only cross
    assert(i < N);
    return add(range_query(i << 1, L, R), range_query(i << 1 | 1, L, R));
}

// Setup the tree after population the array-aligned leaves with data and ranges
void pushup_prepare(int i) {
    if (i >= N)
        return;
    pushup_prepare(i << 1);
    pushup_prepare(i << 1 | 1);
    tree[i].L = tree[i << 1].L;
    tree[i].R = tree[i << 1 | 1].R;
    tree[i].data = add(tree[i << 1].data, tree[i << 1 | 1].data);
}

auto solve() {
    int Q;
    cin >> N >> Q;
    P = 1;
    while (P < N)
        P <<= 1;

    vector<int> A(N);
    for (int i = 0; i < N; ++i)
        cin >> A[i];

    // prepare tree
    tree.resize(2 * N);
    for (int i = 0; i < N; ++i) {
        tree[i + N].L = i;
        tree[i + N].R = i + 1;
        update_data(tree[i + N].data, i, A[i]);
    }

    // realign leaves from array to tree
    // [leaftbegin, leafend[ is the leaves range in tree
    auto leafbegin = tree.begin() + N, leafend = tree.end();
    rotate(leafbegin, leafbegin + (2 * N - P), leafend);
    pushup_prepare(1);

    long sum = 0;

    for (int q = 0; q < Q; ++q) {
        string operation;
        cin >> operation;
        if (operation == "U") {
            int i;
            long v;
            cin >> i >> v;
            update(i - 1, v);
        }
        if (operation == "Q") {
            int L, R;
            cin >> L >> R;
            data_t data = range_query(1, --L, R);
            auto a = data.even, b = data.odd;
            auto c = data.inc_even, d = data.inc_odd;
            auto value = (c - d - (L - 1) * (a - b));
            auto query = (L & 1) ? -value : value;
            sum += query;
        }
    }

    return sum;
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
