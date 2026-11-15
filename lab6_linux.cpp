#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <ctime>

#define SHM_NAME "/my_shared_memory"
#define SEM_NAME "/my_semaphore"
#define MAX_COUNT 1000

int main() {
    int shm_fd;
    int *counter;
    sem_t *sem;
    
    std::srand(std::time(nullptr) ^ getpid());

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Eroare shm_open\n";
        exit(1);
    }
    ftruncate(shm_fd, sizeof(int));
    
    counter = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        std::cerr << "Eroare sem_open\n";
        exit(1);
    }

    std::cout << "Procesul " << getpid() << " a inceput\n";

    while (true) {
        sem_wait(sem);

        if (*counter >= MAX_COUNT) {
            sem_post(sem);
            break;
        }

        while (true) {
            int ban = (std::rand() % 2) + 1;
            if (ban == 2) {
                (*counter)++;
                std::cout << "[PID: " << getpid() << "] A scris: " << *counter << "\n";
                
                if (*counter >= MAX_COUNT) {
                    break;
                }
            } else {
                break;
            }
        }

        sem_post(sem);
        usleep(1000); 
    }

    std::cout << "Procesul " << getpid() << " s-a terminat.\n";

    munmap(counter, sizeof(int));
    sem_close(sem);
    
    return 0;
}