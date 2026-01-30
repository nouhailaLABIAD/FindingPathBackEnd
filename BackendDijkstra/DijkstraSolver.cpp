#include "DijkstraSolver.h"

#include <queue>
#include <algorithm>
#include <functional>
#include <set>

using namespace std;

/* ===== Constructeur ===== */
DijkstraSolver::DijkstraSolver(int n, int* matrix)
    : n(n), matrix(matrix) {}

/* ===== Reconstruction du chemin ===== */
PathResult DijkstraSolver::buildPath(
    const vector<Node>& prev,
    Node end
) {
    PathResult result;

    for (Node at = end; at != -1; at = prev[at]) {
        result.path.push_back(at);
    }

    reverse(result.path.begin(), result.path.end());
    return result;
}

/* ===== Algorithme de Dijkstra ===== */
PathResult DijkstraSolver::solve(Node start, Node end) {

    vector<Weight> dist(n, INF);
    vector<Node> prev(n, -1);
    vector<Node> visitedOrder ;

    using State = pair<Weight, Node>;
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto  Top = pq.top();
        Weight d = Top.first ;
        Node u   = Top.second ;
        pq.pop();

        if (d > dist[u]) continue;
        visitedOrder.push_back(u);

        if (u == end) break; // optimisation

        for (Node v = 0; v < n; v++) {
            Weight w = matrix[u * n + v];
            if (w <= 0) continue;

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    PathResult result =  buildPath(prev, end);
    result.visitedNodes = visitedOrder ;
    return result ;
}
