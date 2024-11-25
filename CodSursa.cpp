#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <time.h>
#include <sys/wait.h>

#define SHARED_MEM_NAME "/shared_mem_example"
#define SEMAPHORE_NAME "/shared_semaphore_example"

int main() {
    srand(time(NULL));
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Eroare la crearea memoriei partajate");
        return 1;
    }
    ftruncate(shm_fd, sizeof(int));
    int *counter = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (counter == MAP_FAILED) {
        perror("Eroare la maparea memoriei");
        return 1;
    }
    *counter = 1;
    sem_t *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("Eroare la crearea semaforului");
        return 1;
    }
    if (fork() == 0) {
        while (*counter < 1000) {
            sem_wait(sem);
            if (*counter < 1000 && rand() % 2 == 0) {
                (*counter)++;
                printf("Proces copil: %d\n", *counter);
            }
            sem_post(sem);
            usleep(100000);
        }
    } else {
        while (*counter < 1000) {
            sem_wait(sem);
            if (*counter < 1000 && rand() % 2 == 0) {
                (*counter)++;
                printf("Proces părinte: %d\n", *counter);
            }
            sem_post(sem);
            usleep(100000);
        }
        wait(NULL);
        sem_close(sem);
        sem_unlink(SEMAPHORE_NAME);
        munmap(counter, sizeof(int));
        shm_unlink(SHARED_MEM_NAME);
    }
    return 0;
}
