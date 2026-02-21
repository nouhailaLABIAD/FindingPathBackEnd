#include <emscripten.h>
#include <vector>
#include <queue>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <functional>

using namespace std;

// Structure pour les nœuds dans la file de priorité
struct Node {
    int index;
    int g; // coût depuis le départ
    int f; // g + h

    Node(int idx, int g_cost, int f_cost) : index(idx), g(g_cost), f(f_cost) {}

    // Pour la priority_queue (max-heap par défaut), on inverse l'ordre pour avoir un min-heap
    bool operator>(const Node& other) const {
        return f > other.f;
    }
};

// Heuristique : distance de Manhattan
int heuristic(int currentRow, int currentCol, int endRow, int endCol) {
    return abs(currentRow - endRow) + abs(currentCol - endCol);
}

// Fonction utilitaire pour extraire la partie décimale (comme dans Dijkstra)
int getDecimalPart(double number) {
    if (number < 0) number = -number;
    double intPart;
    double fract = modf(number, &intPart);
    return static_cast<int>(fract * 100);
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int* astar(int rows, int cols, int* grid, int startRow, int startCol, int endRow, int endCol) {
    auto start_time = chrono::high_resolution_clock::now();

    int startIdx = startRow * cols + startCol;
    int endIdx   = endRow * cols + endCol;

    int n = rows * cols;
    vector<bool> visited(n, false);
    vector<int> parent(n, -1);
    vector<int> gCost(n, 1e9); // initialisé à l'infini
    gCost[startIdx] = 0;

    // File de priorité pour les nœuds à explorer (min-heap basé sur f)
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    pq.push(Node(startIdx, 0, heuristic(startRow, startCol, endRow, endCol)));

    vector<int> visitedOrder;

    bool found = false;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();
        int u = current.index;
        int r = u / cols;
        int c = u % cols;

        if (visited[u]) continue;
        visited[u] = true;
        visitedOrder.push_back(u);

        if (u == endIdx) {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                int v = nr * cols + nc;
                if (grid[v] == 0) { // 0 = libre
                    int tentative_g = gCost[u] + 1; // coût uniforme = 1
                    if (tentative_g < gCost[v]) {
                        gCost[v] = tentative_g;
                        int h = heuristic(nr, nc, endRow, endCol);
                        int f = tentative_g + h;
                        parent[v] = u;
                        pq.push(Node(v, tentative_g, f));
                    }
                }
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration_ms = end_time - start_time;
    double execTime = duration_ms.count();
    int time_int = static_cast<int>(execTime);
    int time_dec = getDecimalPart(execTime);

    // Reconstruction du chemin
    vector<int> path;
    if (found) {
        for (int v = endIdx; v != -1; v = parent[v]) {
            path.push_back(v);
        }
        reverse(path.begin(), path.end());
    }

    int pathSize = path.size();
    int visitedSize = visitedOrder.size();

    // Allocation du buffer de résultat
    int* buffer = (int*)malloc((pathSize + visitedSize + 4) * sizeof(int));
    if (!buffer) return nullptr;

    buffer[0] = pathSize;
    buffer[1] = visitedSize;
    buffer[2] = time_int;
    buffer[3] = time_dec;

    for (int i = 0; i < pathSize; i++) {
        buffer[4 + i] = path[i];
    }
    for (int i = 0; i < visitedSize; i++) {
        buffer[4 + pathSize + i] = visitedOrder[i];
    }

    return buffer;
}

} // extern "C"