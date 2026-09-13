#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_PROC 10
#define MAX_RES 10

int adj[MAX_PROC][MAX_PROC];
int n_proc, m_res;
int visited[MAX_PROC];
int recStack[MAX_PROC];
int parent[MAX_PROC];

void build_wait_for(int alloc[][MAX_RES], int req[][MAX_RES], int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adj[i][j] = 0;
        }
    }

    for (int i = 0; i < n; i++) { // Process i requesting
        for (int k = 0; k < m; k++) {
            if (req[i][k] > 0) {
                // Find who holds resource k
                for (int j = 0; j < n; j++) {
                    if (i != j && alloc[j][k] > 0) {
                        adj[i][j] = 1;
                    }
                }
            }
        }
    }
}

int dfs(int v) {
    visited[v] = 1;
    recStack[v] = 1;

    for (int i = 0; i < n_proc; i++) {
        if (adj[v][i]) {
            if (!visited[i]) {
                parent[i] = v;
                if (dfs(i))
                    return 1;
            } else if (recStack[i]) {
                // Cycle found
                printf(1, "Deadlock detected! Cycle: ");
                int curr = v;
                // Since this is a simple path up to i
                // Actually to print in order, we can collect them
                int cycle[MAX_PROC];
                int c_len = 0;
                while (curr != i) {
                    cycle[c_len++] = curr;
                    curr = parent[curr];
                }
                cycle[c_len++] = i;
                
                // Print in reverse (which is the forward direction of edges)
                for (int k = c_len - 1; k >= 0; k--) {
                    printf(1, "P%d -> ", cycle[k]);
                }
                printf(1, "P%d\n", cycle[c_len - 1]);
                return 1;
            }
        }
    }
    recStack[v] = 0;
    return 0;
}

void detect_deadlock(int alloc[][MAX_RES], int req[][MAX_RES], int n, int m) {
    n_proc = n;
    m_res = m;
    build_wait_for(alloc, req, n, m);

    for (int i = 0; i < n; i++) {
        visited[i] = 0;
        recStack[i] = 0;
        parent[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            if (dfs(i)) {
                return;
            }
        }
    }
    printf(1, "No deadlock detected.\n");
}

int main() {
    printf(1, "--- Scenario 1: No Deadlock ---\n");
    int alloc1[3][MAX_RES] = {
        {1, 0, 0}, // P0
        {0, 1, 0}, // P1
        {0, 0, 1}  // P2
    };
    int req1[3][MAX_RES] = {
        {0, 1, 0}, // P0 waits on P1
        {0, 0, 1}, // P1 waits on P2
        {0, 0, 0}  // P2 waits on none
    };
    detect_deadlock(alloc1, req1, 3, 3);

    printf(1, "\n--- Scenario 2: Deadlock (3 processes) ---\n");
    int alloc2[4][MAX_RES] = {
        {1, 0, 0}, // P0
        {0, 1, 0}, // P1
        {0, 0, 1}, // P2
        {0, 0, 0}  // P3
    };
    int req2[4][MAX_RES] = {
        {0, 1, 0}, // P0 waits on P1
        {0, 0, 1}, // P1 waits on P2
        {1, 0, 0}, // P2 waits on P0 (Circular wait: P0->P1->P2->P0)
        {0, 0, 0}  
    };
    detect_deadlock(alloc2, req2, 4, 3);

    exit();
}
