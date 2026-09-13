#include "types.h"
#include "stat.h"
#include "user.h"

// Semaphores IDs
#define SCANNER_SEM 3
#define PRINTER_SEM 4
#define DISK_SEM 5

void do_work(int pid, const char* res1, const char* res2) {
    printf(1, "Process %d: Started work with %s and %s.\n", pid, res1, res2);
    sleep(10); // Busy wait / sleep
    printf(1, "Process %d: Finished work.\n", pid);
}

int main() {
    // Initialize semaphores for resource pools
    sem_init(SCANNER_SEM, 1);
    sem_init(PRINTER_SEM, 2);
    sem_init(DISK_SEM, 2);

    printf(1, "Starting syncdeadlock with Resource Ordering prevention...\n");

    for (int i = 0; i < 5; i++) {
        int pid = fork();
        if (pid == 0) {
            // Child process
            for (int cycle = 0; cycle < 3; cycle++) { // run for a few cycles
                if (i == 0 || i == 4) {
                    // Needs Scanner and Printer. Order: Scanner, then Printer.
                    printf(1, "Process %d: Requesting Scanner\n", i);
                    sem_wait(SCANNER_SEM);
                    printf(1, "Process %d: Granted Scanner\n", i);

                    printf(1, "Process %d: Requesting Printer\n", i);
                    sem_wait(PRINTER_SEM);
                    printf(1, "Process %d: Granted Printer\n", i);

                    do_work(i, "Scanner", "Printer");

                    printf(1, "Process %d: Releasing Printer\n", i);
                    sem_post(PRINTER_SEM);
                    printf(1, "Process %d: Releasing Scanner\n", i);
                    sem_post(SCANNER_SEM);

                } else if (i == 1 || i == 3) {
                    // Needs Printer and Disk. Order: Printer, then Disk.
                    printf(1, "Process %d: Requesting Printer\n", i);
                    sem_wait(PRINTER_SEM);
                    printf(1, "Process %d: Granted Printer\n", i);

                    printf(1, "Process %d: Requesting Disk\n", i);
                    sem_wait(DISK_SEM);
                    printf(1, "Process %d: Granted Disk\n", i);

                    do_work(i, "Printer", "Disk");

                    printf(1, "Process %d: Releasing Disk\n", i);
                    sem_post(DISK_SEM);
                    printf(1, "Process %d: Releasing Printer\n", i);
                    sem_post(PRINTER_SEM);

                } else if (i == 2) {
                    // Needs Scanner and Disk. Order: Scanner, then Disk.
                    printf(1, "Process %d: Requesting Scanner\n", i);
                    sem_wait(SCANNER_SEM);
                    printf(1, "Process %d: Granted Scanner\n", i);

                    printf(1, "Process %d: Requesting Disk\n", i);
                    sem_wait(DISK_SEM);
                    printf(1, "Process %d: Granted Disk\n", i);

                    do_work(i, "Scanner", "Disk");

                    printf(1, "Process %d: Releasing Disk\n", i);
                    sem_post(DISK_SEM);
                    printf(1, "Process %d: Releasing Scanner\n", i);
                    sem_post(SCANNER_SEM);
                }
                sleep(5); // brief pause between cycles
            }
            exit();
        }
    }

    // Parent waits for all 5 children
    for (int i = 0; i < 5; i++) {
        wait();
    }
    
    printf(1, "All processes completed successfully (no deadlock).\n");
    exit();
}
