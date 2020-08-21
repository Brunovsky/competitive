#ifndef GRAPH_OPERATIONS_HPP
#define GRAPH_OPERATIONS_HPP

#include "graph.hpp"
#include "random.hpp"

// *****

/**
 * Relabel the nodes of this graph randomly, and return a new graph.
 */
graph relabel(const graph& g) {
    vector<int> label(g.V);
    iota(begin(label), end(label), 0);
    shuffle(begin(label), end(label), mt);
    graph h(g.V);
    for (int u = 0; u < g.V; u++) {
        for (int v : g.adj[u]) {
            h.adj[label[u]].push_back(label[v]);
        }
    }
    h.E = g.E;
    return h;
}

digraph relabel(const digraph& g) {
    vector<int> label(g.V);
    iota(begin(label), end(label), 0);
    shuffle(begin(label), end(label), mt);
    digraph h(g.V);
    for (int u = 0; u < g.V; u++) {
        for (int v : g.adj[u]) {
            h.add(label[u], label[v]);
        }
    }
    h.E = g.E;
    return h;
}

/**
 * Reverse the edges of the graph, and return a new graph.
 */
digraph reverse(const digraph& g) {
    digraph rev(g);
    swap(rev.adj, rev.rev);
    return rev;
}

/**
 * Check if a graph is (strongly) connected
 */
template <typename Graph>
int count_reachable(const Graph& g, int s = 0) {
    uint i = 0, S = 1, V = g.V;
    vector<bool> vis(V, false);
    vector<int> bfs{s};
    vis[s] = true;
    while (i++ < S && S < V) {
        for (int v : g.adj[bfs[i - 1]]) {
            if (!vis[v]) {
                vis[v] = true;
                S++;
                bfs.push_back(v);
            }
        }
    }
    return S;
}

template <typename Graph>
bool reachable(const Graph& g, int s, int t) {
    uint i = 0, S = 1, V = g.V;
    vector<bool> vis(V, false);
    vector<int> bfs{s};
    while (i++ < S && S < V) {
        for (int v : g.adj[bfs[i - 1]]) {
            if (!vis[v]) {
                vis[v] = true;
                S++;
                bfs.push_back(v);
                if (v == t)
                    return true;
            }
        }
    }
    return false;
}

bool is_connected(const graph& g) {
    if (g.V == 0)
        return true;
    return count_reachable(g) == g.V;
}

bool is_connected(const digraph& g) {
    if (g.V == 0)
        return true;
    return count_reachable(g) == g.V && count_reachable(reverse(g)) == g.V;
}

bool is_rooted(const digraph& g, int s = 0) {
    if (g.V == 0)
        return true;
    return count_reachable(g, s) == g.V;
}

/**
 * Join two graphs together.
 * The new graph has the two graphs joined as disconnected subgraphs.
 */
graph& join(graph& g, const graph& h) {
    int n = g.V, V = g.V + h.V, E = g.E + h.E;
    g.V = V, g.E = E;
    g.adj.resize(V);
    for (int u = 0; u < h.V; u++) {
        for (int v : h.adj[u]) {
            g.adj[u + n].push_back(v + n);
        }
    }
    return g;
}

digraph& join(digraph& g, const digraph& h) {
    int n = g.V, V = g.V + h.V, E = g.E + h.E;
    g.V = V, g.E = E;
    g.adj.resize(V);
    g.rev.resize(V);
    for (int u = 0; u < h.V; u++) {
        for (int v : h.adj[u]) {
            g.adj[u + n].push_back(v + n);
            g.rev[v + n].push_back(u + n);
        }
    }
    return g;
}

#endif // GRAPH_OPERATIONS_HPP