#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#define SHM_NAME "Local\\MySharedMemory"
#define SEM_NAME "Local\\MySemaphore"
#define MAX_COUNT 1000

int main() {
    HANDLE hMapFile;
    HANDLE hSemaphore;
    int* counter;
    DWORD pid = GetCurrentProcessId();

    std::srand((unsigned int)std::time(nullptr) ^ pid);

    hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(int),
        SHM_NAME
    );

    if (hMapFile == NULL) {
        std::cerr << "Eroare la CreateFileMapping (" << GetLastError() << ").\n";
        return 1;
    }

    counter = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
    if (counter == NULL) {
        std::cerr << "Eroare la MapViewOfFile (" << GetLastError() << ").\n";
        CloseHandle(hMapFile);
        return 1;
    }

    hSemaphore = CreateSemaphoreA(NULL, 1, 1, SEM_NAME);
    if (hSemaphore == NULL) {
        std::cerr << "Eroare la CreateSemaphore (" << GetLastError() << ").\n";
        UnmapViewOfFile(counter);
        CloseHandle(hMapFile);
        return 1;
    }

    std::cout << "Procesul " << pid << " a inceput...\n";

    while (true) {
        WaitForSingleObject(hSemaphore, INFINITE);

        if (*counter >= MAX_COUNT) {
            ReleaseSemaphore(hSemaphore, 1, NULL);
            break;
        }

        while (true) {
            int ban = (std::rand() % 2) + 1;
            if (ban == 2) {
                (*counter)++;
                std::cout << "[PID: " << pid << "] A scris: " << *counter << "\n";
                
                if (*counter >= MAX_COUNT) {
                    break;
                }
            } else {
                break;
            }
        }

        ReleaseSemaphore(hSemaphore, 1, NULL);
        Sleep(1);
    }

    std::cout << "Procesul " << pid << " s-a terminat.\n";

    UnmapViewOfFile(counter);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

    return 0;
}