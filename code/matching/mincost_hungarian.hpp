#ifndef MINCOST_HUNGARIAN_HPP
#define MINCOST_HUNGARIAN_HPP

#include "../struct/integer_heaps.hpp" // pairing_int_heap

// *****

using edges_t = vector<array<int, 2>>;

/**
 * Min-cost maximum bipartite matching (hungarian, dijkstra-based)
 * Based on: https://web.stanford.edu/class/cs261/min_cost_bipartite_matching.pdf
 * Complexity: O(EW log E), W=max(U,V).
 *
 * If the bipartite graph is not balanced (U != V)
 * it will be padded with infinite weight edges.
 * A perfect matching must exist on the padded graph, otherwise the answer risks being
 * bogus. To obtain a min-cost non-perfect matching of arbitrary size use reduction
 * to mincost flow instead.
 */
struct mincost_hungarian {
    int U, V, W, E = 0;
    vector<vector<pair<int, long>>> adj;
    vector<int> m[2];

    mincost_hungarian(int U, int V, const edges_t& g = {}, const vector<long>& cost = {})
        : U(U), V(V), W(max(U, V)), adj(W) {
        for (int e = 0, n = g.size(); e < n; e++) {
            auto [u, v] = g[e];
            add(u, v, cost[e]);
        }
        for (int v = V; v < U; v++) // edges if padding V side
            for (int u = 0; u < U; u++)
                adj[u].push_back({v, inf}), E++;
        for (int u = U; u < V; u++) // edges if padding U side
            for (int v = 0; v < V; v++)
                adj[u].push_back({v, inf}), E++;
    }

    void add(int u, int v, long w) {
        assert(0 <= u && u < U && 0 <= v && v < V && w >= 0);
        adj[u].push_back({v, w});
    }

    vector<int> prev[2];
    vector<long> pi[2], dist[2];
    static inline constexpr long inf = LONG_MAX / 3;

    bool dijkstra() {
        dist[0].assign(W + 1, inf);
        dist[1].assign(W, inf);
        prev[0].assign(W + 1, -1);
        prev[1].assign(W, -1);

        vector<bool> vis(W, false);
        pairing_int_heap<less_container<vector<long>>> Q(W + 1, dist[0]);

        for (int u = 0; u < W; u++)
            if (m[0][u] == W)
                dist[0][u] = 0, Q.push(u);

        while (!Q.empty()) {
            int u = Q.pop();
            if (u == W || vis[u]) {
                continue;
            }
            vis[u] = true;
            for (auto [v, w] : adj[u]) {
                int y = m[1][v];
                w = min(dist[0][u] + w + pi[0][u] - pi[1][v], inf);
                if (dist[0][y] > w) {
                    dist[0][y] = w, prev[0][y] = v;
                    Q.push_or_improve(y);
                }
                if (dist[1][v] > w) {
                    dist[1][v] = w, prev[1][v] = u;
                }
            }
        }
        reprice();
        return prev[0][W] != -1;
    }

    void reprice() {
        for (int i : {0, 1}) {
            for (int u = 0; u < W; u++) {
                pi[i][u] = min(dist[i][u] + pi[i][u], inf);
            }
        }
    }

    auto path() const {
        edges_t nodes;
        int v = prev[0][W];
        while (v != -1) {
            nodes.push_back({prev[1][v], v});
            v = prev[0][prev[1][v]];
        }
        return nodes;
    }

    long mincost_max_matching() {
        m[0].assign(W, W);
        m[1].assign(W, W);
        pi[0].assign(W, 0);
        pi[1].assign(W, 0);

        int matchings = 0;
        while (matchings < W && dijkstra()) {
            for (auto [u, v] : path()) {
                m[0][u] = v, m[1][v] = u;
            }
            matchings++;
        }
        if (matchings < W)
            return -1;

        long min_cost = 0;
        for (int u = 0; u < U; u++) {
            if (int v = m[0][u]; v < V) {
                for (auto [t, w] : adj[u]) {
                    if (v == t) {
                        min_cost += w;
                        break;
                    }
                }
            }
        }
        return min_cost;
    }
};

#endif // MINCOST_HUNGARIAN_HPP