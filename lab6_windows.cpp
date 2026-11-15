#include <iostream>
#include <windows.h>
#include <string>
#include <cstdlib>
#include <ctime>

const char* SHARED_MEM_NAME = "Local\\Lab6SharedMem";
const char* SEMAPHORE_NAME = "Local\\Lab6Semaphore";
const int MAX_COUNT = 1000;

int main(int argc, char* argv[]) {
    bool isChild = (argc > 1 && std::string(argv[1]) == "child");

    HANDLE hMapFile;
    int* counter;
    HANDLE hSemaphore;

    if (!isChild) {
        hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), SHARED_MEM_NAME);
        counter = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
        *counter = 0;

        hSemaphore = CreateSemaphoreA(NULL, 1, 1, SEMAPHORE_NAME);

        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char cmdBuf[MAX_PATH + 20];
        sprintf(cmdBuf, "\"%s\" child", exePath);

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessA(NULL, cmdBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "Eroare la crearea procesului copil!\n";
            return 1;
        }
    } else {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        counter = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
        hSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_NAME);
    }

    int pid = GetCurrentProcessId();
    
    srand(time(NULL) ^ pid); 

    std::cout << "A pornit procesul " << (isChild ? "COPIL" : "PARINTE") << " cu PID: " << pid << "\n";

    while (true) {
        WaitForSingleObject(hSemaphore, INFINITE);

        if (*counter >= MAX_COUNT) {
            ReleaseSemaphore(hSemaphore, 1, NULL);
            break;
        }

        int ban = rand() % 2 + 1; 

        while (ban == 2 && *counter < MAX_COUNT) {
            (*counter)++;
            std::cout << "[PID: " << pid << "] a scris: " << *counter << "\n";
            ban = rand() % 2 + 1; 
        }

        ReleaseSemaphore(hSemaphore, 1, NULL);
        Sleep(1);
    }

    std::cout << "Procesul " << pid << " s-a terminat.\n";

    UnmapViewOfFile(counter);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);

    if (!isChild) {
        Sleep(500);
    }

    return 0;
}