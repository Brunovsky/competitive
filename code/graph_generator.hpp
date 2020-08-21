#ifndef GRAPH_GENERATOR_HPP
#define GRAPH_GENERATOR_HPP

#include "graph_operations.hpp"

// *****

/**
 * Take a graph and add self-loops with probability p
 */
void add_self_loops(graph& g, double p) {
    boold distp(p);
    for (int u = 0; u < g.V; u++) {
        if (distp(mt))
            g.add(u, u);
    }
}

/**
 * Take a graph and a parent map and add parent edges
 */
void add_parent_edges(graph& g, const vector<int>& parent, int start) {
    for (int u = start; u < g.V; u++) {
        g.add(u, parent[u]);
    }
}

/**
 * Take a digraph and a parent map and add parent/child edges
 */
void add_parent_edges(digraph& g, const vector<int>& parent, int start,
                      bool toparent = true, bool tochild = false) {
    for (int u = start; u < g.V; u++) {
        if (toparent)
            g.add(u, parent[u]);
        if (tochild)
            g.add(parent[u], u);
    }
}

/**
 * Take a digraph and a level size list and back edges with probability q.
 */
void add_ranked_back_edges(digraph& g, double q, const vector<int>& R) {
    boold distq(q);
    int start = 0, ranks = R.size();
    for (int r = 0; r < ranks; r++) {
        int mid = start + R[r];
        int universe = g.V - mid;
        binomd distk(universe, q);
        for (int u = start; u < mid; u++) {
            int k = distk(mt);
            for (int v : int_sample(k, mid, g.V - 1)) {
                g.add(v, u);
            }
        }
        start = mid;
    }
}

/**
 * Take any graph and add all edges from vertices [u1..u2) to [v1..v2).
 */
template <typename Graph>
void add_level_step_full(Graph& g, int u1, int u2, int v1, int v2) {
    for (int u = u1; u < u2; u++)
        for (int v = v1; v < v2; v++)
            g.add(u, v);
}

/**
 * Take any graph and add edges from vertices [u1..u2) to [v1..v2) with probability p.
 */
template <typename Graph>
void add_level_step_uniform(Graph& g, int u1, int u2, int v1, int v2, double p,
                            bool mustout = false, bool mustin = false) {
    if (u1 == u2 || v1 == v2)
        return;
    vector<bool> out(u2 - u1, false), in(v2 - v1, false);
    if (p <= 0.25) {
        binomd distk(v2 - v1, p);
        for (int u = u1; u < u2; u++)
            for (int v : int_sample(distk(mt), v1, v2 - 1))
                g.add(u, v), out[u - u1] = true, in[v - v1] = true;
    } else {
        boold distp(p);
        for (int u = u1; u < u2; u++)
            for (int v = v1; v < v2; v++)
                if (distp(mt))
                    g.add(u, v), out[u - u1] = true, in[v - v1] = true;
    }
    if (mustout) {
        intd distv(v1, v2 - 1);
        for (int v, u = u1; u < u2; u++)
            if (!out[u - u1])
                v = distv(mt), g.add(u, v), in[v - v1] = true;
    }
    if (mustin) {
        intd distu(u1, u2 - 1);
        for (int v = v1; v < v2; v++)
            if (!in[v - v1])
                g.add(distu(mt), v);
    }
}

/**
 * Take any graph and a level size list, and add level edges from level 0 to level 1,
 * level 1 to level 2, ..., level L - 2 to level L - 1.
 * If loop is true, add edges from level L - 1 to level 0.
 */
template <typename Graph>
void link_levels_full(Graph& g, const vector<int>& R, bool loop = false) {
    int start = 0, ranks = R.size();
    for (int r = 0; r + 1 < ranks; r++) {
        int mid = start + R[r], end = mid + R[r + 1];
        add_level_step_full(g, start, mid, mid, end);
        start = mid;
    }
    assert(start != g.V);
    if (loop && ranks >= 2) {
        int mid = start + R[ranks - 1];
        assert(mid == g.V);
        add_level_step_full(g, start, mid, 0, R[0]);
    }
}

/**
 * Take any graph and a level size list, and add level edges from level 0 to level 1,
 * level 1 to level 2, ..., level L - 2 to level L - 1.
 * If loop is true, add edges from level L - 1 to level 0.
 */
template <typename Graph>
void link_levels_uniform(Graph& g, double p, const vector<int>& R, bool loop = false,
                         bool mustout = true, bool mustin = true) {
    int start = 0, ranks = R.size();
    for (int r = 0; r + 1 < ranks; r++) {
        int mid = start + R[r], end = mid + R[r + 1];
        add_level_step_uniform(g, start, mid, mid, end, p, mustout, mustin);
        start = mid;
    }
    assert(start != g.V);
    if (loop && ranks >= 2) {
        int mid = start + R[ranks - 1];
        assert(mid == g.V);
        add_level_step_uniform(g, start, mid, 0, R[0], p, mustout, mustin);
    }
}

/**
 * Take any graph and a level size list, and add level edges from level n to level m
 * with probability p^(m-n).
 * If loop is true, add edges where m < n as well, "going around".
 * mustout and mustin are only for adjacent levels
 */
template <typename Graph>
void link_levels_exp(Graph& g, double p, const vector<int>& R, bool loop = false,
                     bool mustout = true, bool mustin = true) {
    int ranks = R.size();
    vector<int> starts(ranks + 1);
    for (int r = 0; r < ranks; r++) {
        starts[r + 1] = starts[r] + R[r];
    }
    for (int r = 0; r - 1 < ranks; r++) {
        int u1 = starts[r], u2 = starts[r + 1];
        double q = 1.0;
        for (int t = r + 1; t < ranks; t++) {
            int v1 = starts[t], v2 = starts[t + 1];
            q *= p;
            bool out = mustout && t == r + 1, in = mustin && t == r + 1;
            add_level_step_uniform(g, u1, u2, v1, v2, q, out, in);
        }
    }
    if (loop && ranks >= 2) {
        double z = 1.0;
        for (int t = ranks - 1; t >= 1; t--) {
            int v1 = starts[t], v2 = starts[t + 1];
            double q = z;
            for (int r = 0; r < t; r++) {
                int u1 = starts[r], u2 = starts[r + 1];
                q *= p;
                add_level_step_uniform(g, u1, u2, v1, v2, q, false, false);
            }
            z *= p;
        }
    }
}

/**
 * Generate an arbitrary degree undirected tree rooted at 0.
 */
graph generate_tree_undirected(int V) {
    graph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1);
    return g;
}

/**
 * Generate an arbitrary degree directed tree rooted at 0.
 */
digraph generate_tree_directed(int V, bool toparent = true, bool tochild = false) {
    digraph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1, toparent, tochild);
    return g;
}

/**
 * Generate grid graph of size WxH vertices
 * Vertices numbered top to bottom, left to right, row-major
 */
graph generate_grid_undirected(int W, int H) {
    int V = W * H;
    graph g(V);
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < H; j++) {
            int u = i * H + j;
            int south = u + H;
            int east = u + 1;
            if (i + 1 < W)
                g.add(u, south);
            if (j + 1 < H)
                g.add(u, east);
        }
    }
    return g;
}

/**
 * Generate grid digraph of size WxH vertices directed towards southeast
 * Vertices numbered top to bottom, left to right, row-major
 */
digraph generate_grid_directed(int W, int H) {
    int V = W * H;
    digraph g(V);
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < H; j++) {
            int u = i * H + j;
            int south = u + H;
            int east = u + 1;
            if (i + 1 < W)
                g.add(u, south);
            if (j + 1 < H)
                g.add(u, east);
        }
    }
    return g;
}

/**
 * Generate the complete graph on V vertices
 */
graph generate_complete_undirected(int V) {
    graph g(V);
    for (int u = 0; u < V; u++) {
        for (int v = u + 1; v < V; v++) {
            g.add(u, v);
        }
    }
    return g;
}

/**
 * Generate the complete digraph on V vertices
 * Vertex u has an edge to vertex v iff u < v.
 */
digraph generate_complete_directed(int V) {
    digraph g(V);
    for (int u = 0; u < V; u++) {
        for (int v = u + 1; v < V; v++) {
            g.add(u, v);
        }
    }
    return g;
}

/**
 * Generate cycle graph on V vertices
 * This is not a cyclic graph for V <= 2.
 */
graph generate_cycle_undirected(int V) {
    graph g(V);
    for (int u = 1; u < V; u++) {
        g.add(u - 1, u);
    }
    if (V >= 3)
        g.add(V - 1, 0);
    return g;
}

/**
 * Generate cycle digraph on V vertices
 * This is not a cyclic graph for V <= 2.
 */
digraph generate_cycle_directed(int V) {
    digraph g(V);
    for (int u = 1; u < V; u++) {
        g.add(u - 1, u);
    }
    if (V >= 3)
        g.add(V - 1, 0);
    return g;
}

/**
 * Generate uniformly at random a connected graph on V vertices where every edge
 * exists with some probability p.
 * Algorithm: Generate a tree, then add back-edges with probability p.
 * Complexity: O(pV^2)
 */
graph generate_uniform_undirected(int V, double p) {
    graph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1);
    for (int v = 1; v < V; v++) {
        binomd distk(v, p);
        for (int u : int_sample(distk(mt), 0, v - 1))
            if (u != parent[v])
                g.add(u, v);
    }
    return g;
}

/**
 * Generate uniformly at random a connected graph on V vertices with E edges.
 * Algorithm: Generate a tree with V - 1 edges, then select E + V random edges
 *     uniformly at random, shuffled, and add them until E are reached.
 * Complexity: O(V + E)
 */
graph generate_exact_undirected(int V, int E) {
    assert(V - 1 <= E && E <= V * (V - 1) / 2);
    graph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1);
    if (E == V - 1)
        return g;
    int k = min(V * (V - 1) / 2, E + V);
    auto edges = choose_sample(k, 0, V - 1, false);
    shuffle(begin(edges), end(edges), mt);
    for (auto edge : edges) {
        int u = edge[0], v = edge[1];
        assert(u < v);
        if (u != parent[v] && v != parent[u])
            g.add(u, v);
        if (g.E == E)
            break;
    }
    return g;
}

/**
 * Generate uniformly at random a rooted connected dag on V vertices where every edge
 * exists with some probability p.
 * Algorithm: Generate a directed tree, then add forward-edges with probability p.
 * Complexity: O(pV^2)
 */
digraph generate_uniform_rooted_dag(int V, double p) {
    digraph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1, false, true);
    for (int v = 1; v < V; v++) {
        binomd distk(v, p);
        for (int u : int_sample(distk(mt), 0, v - 1))
            if (u != parent[v])
                g.add(u, v);
    }
    return g;
}

/**
 * Generate uniformly at random a connected rooted dag on V vertices with E edges.
 * Algorithm: Generate a directed tree with V - 1 edges, then select E + V random edges
 *     uniformly at random, shuffled, and add them until E edges are reached.
 * Complexity: O(V + E)
 */
digraph generate_exact_rooted_dag(int V, int E) {
    assert(V - 1 <= E && E <= V * (V - 1) / 2);
    digraph g(V);
    vector<int> parent = parent_sample(V);
    add_parent_edges(g, parent, 1, false, true);
    if (E == V - 1)
        return g;
    int k = min(V * (V - 1) / 2, E + V);
    auto edges = choose_sample(k, 0, V - 1, false);
    shuffle(begin(edges), end(edges), mt);
    for (auto edge : edges) {
        int u = edge[0], v = edge[1];
        assert(u < v);
        if (u != parent[v])
            g.add(u, v);
        if (g.E == E)
            break;
    }
    return g;
}

/**
 * Simple algorithm to generate a random k-regular undirected graph (degree of every
 * node is k) meeting the configuration criteria. Fails with non-negligible
 * probability, and automatically tries again.
 * Complexity: O(kV^2)
 */
graph generate_regular(int V, int k) {
    assert(3 <= k && k < V && (k % 2 == 0 || V % 2 == 0));
    vector<vector<bool>> edges;
    vector<int> nodes;
    graph g;

    auto cmp = [&](int u, int v) {
        return g.adj[u].size() < g.adj[v].size();
    };

restart:
    nodes.resize(V);
    iota(begin(nodes), end(nodes), 0);
    edges.assign(V, vector<bool>(V, false));
    g = graph(V);

    while (k && !nodes.empty()) {
        nth_element(begin(nodes), begin(nodes), end(nodes), cmp);
        shuffle(begin(nodes) + 1, end(nodes), mt);

        int u = nodes[0], v, vi, S = nodes.size();
        for (int i = 1; i < S; i++) {
            int w = nodes[i];
            if (!edges[u][w] && u != w) {
                vi = i, v = w;
                goto add_edge;
            }
        }
        goto restart;

    add_edge:
        g.add(u, v);
        edges[u][v] = edges[v][u] = true;

        int degu = g.adj[u].size();
        int degv = g.adj[v].size();
        if (degv == k) {
            swap(nodes[vi], nodes.back());
            nodes.pop_back();
        }
        if (degu == k) {
            swap(nodes[0], nodes.back());
            nodes.pop_back();
        }
    }
    return g;
}

graph generate_full_level(int V, int ranks, int m = 1, bool loop = false) {
    graph g(V);
    auto R = partition_sample(V, ranks, m);
    link_levels_full(g, R, loop);
    return g;
}

graph generate_uniform_level(int V, double p, int ranks, int m = 1, bool loop = false) {
    graph g(V);
    auto R = partition_sample(V, ranks, m);
    link_levels_uniform(g, p, R, loop);
    return g;
}

digraph generate_full_level_dag(int V, int ranks, int m = 1, bool loop = false) {
    digraph g(V);
    auto R = partition_sample(V, ranks, m);
    link_levels_full(g, R, loop);
    return g;
}

digraph generate_uniform_level_dag(int V, double p, int ranks, int m = 1,
                                   bool loop = false) {
    digraph g(V);
    auto R = partition_sample(V, ranks, m);
    link_levels_uniform(g, p, R, loop, true, true);
    return g;
}

digraph generate_full_level_flow(int V, int ranks, int m = 1, bool loop = false) {
    digraph g(V);
    auto R = partition_sample_flow(V, ranks, m);
    link_levels_full(g, R, loop);
    return g;
}

digraph generate_uniform_level_flow(int V, double p, int ranks, int m = 1,
                                    bool loop = false) {
    digraph g(V);
    auto R = partition_sample_flow(V, ranks, m);
    link_levels_uniform(g, p, R, loop, true, true);
    return g;
}

digraph generate_exp_level_flow(int V, double p, int ranks, int m = 1,
                                bool loop = false) {
    digraph g(V);
    auto R = partition_sample_flow(V, ranks, m);
    link_levels_exp(g, p, R, loop, true, true);
    return g;
}

/**
 * Generate a ranked/level graph where nodes on level i are completely connected to
 * nodes on level i+1 only
 */
graph generate_full_level_undirected(int V, const vector<int>& R) {
    graph g(V);
    int start = 0, ranks = R.size();
    for (int r = 0; r < ranks - 1; r++) {
        int mid = start + R[r];
        int end = mid + R[r + 1];
        for (int u = start; u < mid; u++) {
            for (int v = mid; v < end; v++) {
                g.add(u, v);
            }
        }
    }
    return g;
}

/**
 * Generate a ranked/level graph where nodes on level i are completely connected to
 * nodes on level i+1 only
 */
digraph generate_full_level_directed(int V, const vector<int>& R) {
    digraph g(V);
    int start = 0, ranks = R.size();
    for (int r = 0; r < ranks - 1; r++) {
        int mid = start + R[r];
        int end = mid + R[r + 1];
        for (int u = start; u < mid; u++) {
            for (int v = mid; v < end; v++) {
                g.add(u, v);
            }
        }
    }
    return g;
}

/**
 * Expand each node u of a directed acyclic graph into a randomly generated
 * strongly connected component from f(u).
 * Connect the generated nodes of u into the generated nodes of v in adj[u] using h(u,v)
 * edges and picking nodes arbitrarily.
 */
template <typename Gn, typename En>
digraph generate_scc_expansion(const digraph& dag, Gn&& f, En&& h) {
    int V = dag.V;
    vector<int> intv(V + 1, 0);
    digraph g(0);

    for (int u = 0; u < V; u++) {
        join(g, f(u));
        intv[u + 1] = g.V;
    }
    for (int u = 0; u < V; u++) {
        for (int v : dag.adj[u]) {
            vector<array<int, 2>> choices;
            for (int su = intv[u]; su < intv[u + 1]; su++) {
                for (int sv = intv[v]; sv < intv[v + 1]; sv++) {
                    choices.push_back({su, sv});
                }
            }
            shuffle(begin(choices), end(choices), mt);
            int edges = h(u, v);
            for (int e = 0; e < edges; e++) {
                g.add(choices[e][0], choices[e][1]);
            }
        }
    }
    return g;
}

digraph generate_scc_uniform_expansion(const digraph& dag, int k, double p) {
    int V = dag.V;
    vector<int> cnt(V);
    intd dist(1, k);
    for (int u = 0; u < V; u++) {
        cnt[u] = dist(mt);
    }
    auto f = [&](int u) {
        digraph g;
        intd ranksd(1, max(cnt[u] / 2, 1));
        g = generate_uniform_level_dag(cnt[u], p, ranksd(mt), 1, true);
        return g;
    };
    auto h = [&](int u, int v) {
        return int(ceil(sqrt(cnt[u] * cnt[v])));
    };
    return generate_scc_expansion(dag, f, h);
}

/**
 * Transform a directed graph into a flow graph with uniform positive capacities
 * s is 0, t is V - 1.
 */
flow_graph make_flow_graph(const digraph& g, long max_cap) {
    longd capd(1, max_cap);
    flow_graph f(g.V);
    for (int u = 0; u < g.V; u++) {
        for (int v : g.adj[u]) {
            f.add(u, v, capd(mt));
        }
    }
    return f;
}

/**
 * Generate a random dag flow graph on V vertices
 */
flow_graph generate_dag_flow_graph(int V, double p, long max_cap) {
    intd rankd(3, max(3, V / 3));
    int ranks = rankd(mt), m = min(3, V / ranks);
    p = min(1.0, p);
    digraph g = generate_uniform_level_flow(V, p, ranks, m, false);
    return make_flow_graph(g, max_cap);
}

/**
 * Generate a random flow graph on V vertices
 */
flow_graph generate_flow_graph(int V, double p, long max_cap) {
    intd rankd(3, max(3, V / 3));
    int ranks = rankd(mt), m = min(5, V / ranks);
    p = min(1.0, p);
    digraph g = generate_exp_level_flow(V, p, ranks, m, false);
    return make_flow_graph(g, max_cap);
}

#endif // GRAPH_GENERATOR_HPP
