#include "types.h"
#include "stat.h"
#include "user.h"

#define N 5
#define M 3

int Allocation[N][M] = {
    {0, 1, 0}, // P0
    {2, 0, 0}, // P1
    {3, 0, 2}, // P2
    {2, 1, 1}, // P3
    {0, 0, 2}  // P4
};

int Max[N][M] = {
    {7, 5, 3}, // P0
    {3, 2, 2}, // P1
    {9, 0, 2}, // P2
    {2, 2, 2}, // P3
    {4, 3, 3}  // P4
};

int Available[M] = {3, 3, 2};
int Need[N][M];

void calc_need() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            Need[i][j] = Max[i][j] - Allocation[i][j];
        }
    }
}

int is_safe(int seq[]) {
    int work[M];
    int finish[N];
    for (int i = 0; i < M; i++) work[i] = Available[i];
    for (int i = 0; i < N; i++) finish[i] = 0;

    int count = 0;
    while (count < N) {
        int found = 0;
        for (int p = 0; p < N; p++) {
            if (finish[p] == 0) {
                int j;
                for (j = 0; j < M; j++) {
                    if (Need[p][j] > work[j])
                        break;
                }
                if (j == M) {
                    for (int k = 0; k < M; k++)
                        work[k] += Allocation[p][k];
                    seq[count++] = p;
                    finish[p] = 1;
                    found = 1;
                }
            }
        }
        if (found == 0) {
            return 0; // Not safe
        }
    }
    return 1; // Safe
}

void print_sequence(int seq[]) {
    printf(1, "Safe sequence: ");
    for(int i = 0; i < N; i++){
        printf(1, "P%d ", seq[i]);
    }
    printf(1, "\n");
}

void request_resources(int pid, int request[]) {
    printf(1, "\n--- Process P%d requesting resources: %d %d %d ---\n", pid, request[0], request[1], request[2]);
    for(int i = 0; i < M; i++){
        if (request[i] > Need[pid][i]) {
            printf(1, "Request denied — exceeds Need.\n");
            return;
        }
        if (request[i] > Available[i]) {
            printf(1, "Request denied — exceeds Available (must wait).\n");
            return;
        }
    }

    // Simulate allocation
    for(int i = 0; i < M; i++){
        Available[i] -= request[i];
        Allocation[pid][i] += request[i];
        Need[pid][i] -= request[i];
    }

    int seq[N];
    if (is_safe(seq)) {
        printf(1, "Request granted safely.\n");
        print_sequence(seq);
    } else {
        printf(1, "Request denied — would lead to unsafe state.\n");
        // Rollback
        for(int i = 0; i < M; i++){
            Available[i] += request[i];
            Allocation[pid][i] -= request[i];
            Need[pid][i] += request[i];
        }
    }
}

int main(int argc, char *argv[]) {
    calc_need();
    
    int seq[N];
    printf(1, "Initial State:\n");
    if (is_safe(seq)) {
        print_sequence(seq);
    } else {
        printf(1, "Initial state is unsafe!\n");
    }

    // Scenario 1: Safe request
    // P1 requests 1, 0, 2
    int req1[] = {1, 0, 2};
    request_resources(1, req1);

    // Scenario 2: Unsafe request (leads to deadlock or exceeds available/need)
    // After req1, Available is 2, 3, 0.
    // P4 requests 3, 3, 0. Available has 2, 3, 0 -> exceeds available.
    // P0 requests 0, 2, 0. Need for P0 is 7, 4, 3. Available has 2, 3, 0. 
    // If P0 takes 0, 2, 0 -> Available is 2, 1, 0.
    // Can any process finish? 
    // Need: P0 (7,2,3), P1 (0,2,0), P2 (6,0,0), P3 (0,1,1), P4 (4,3,1).
    // Available: 2, 1, 0. None can finish! So unsafe.
    int req2[] = {0, 2, 0};
    request_resources(0, req2);
    
    exit();
}
