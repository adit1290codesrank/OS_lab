#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
    sem_init(1, 1); // Lock1
    sem_init(2, 1); // Lock2

    printf(1, "Starting bad resource ordering (deadlock)... \n");
    
    int pid = fork();
    if (pid == 0) {
        // Child: Process B
        printf(1, "Process B: Waiting for Lock2...\n");
        sem_wait(2);
        printf(1, "Process B: Acquired Lock2\n");
        
        sleep(50); // Deliberate delay to allow Process A to run

        printf(1, "Process B: Waiting for Lock1...\n");
        sem_wait(1);
        printf(1, "Process B: Acquired Lock1\n");
        
        sem_post(1);
        sem_post(2);
        printf(1, "Process B: Finished\n");
        exit();
    } else {
        // Parent: Process A
        printf(1, "Process A: Waiting for Lock1...\n");
        sem_wait(1);
        printf(1, "Process A: Acquired Lock1\n");
        
        sleep(50); // Deliberate delay to allow Process B to run

        printf(1, "Process A: Waiting for Lock2...\n");
        sem_wait(2);
        printf(1, "Process A: Acquired Lock2\n");
        
        sem_post(2);
        sem_post(1);
        printf(1, "Process A: Finished\n");
        wait();
    }
    
    exit();
}
