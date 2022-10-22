#include "../Header Files/Includes.hpp"

HMODULE _currentModule = nullptr;

// --------------------------------------------------------------------------------------------------------------------------

#define NOTIFICATION(a) \
{ \
    std::cout << a << std::endl; \
} \

// --------------------------------------------------------------------------------------------------------------------------

typedef BOOL(WINAPI* WriteProcessMemoryFn)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
WriteProcessMemoryFn oWriteProcessMemory = nullptr;

BOOL WINAPI WriteProcessMemoryDetour(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten)
{
    // File name taking in the address of the dumped file as a name.
    char buffer[9999];
    sprintf_s(buffer, "C:\\Dumps\\0x%p.bin", lpBaseAddress);

    // Create the file with CreateFile.
    const HANDLE fileHandle = CreateFile(buffer, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (fileHandle)
    {
        if (lpBaseAddress != nullptr)
        {
            // Write the content to the file with WriteFile.
            try
            {
                WriteFile(fileHandle, lpBuffer, nSize, lpNumberOfBytesWritten, nullptr);
                NOTIFICATION("[+] Wrote " << nSize << " bytes (0x" << std::hex << nSize << ")" << std::dec << " from address " << "0x" << lpBaseAddress);
            }
            catch (std::exception& error)
            {

            }

            // Don't leave any un-used handles opened.
            CloseHandle(fileHandle);
        }
    }
    else
    {
        NOTIFICATION("[!] Invalid file handle");
        FreeLibraryAndExitThread(_currentModule, -1);
    }

    return oWriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
}

// --------------------------------------------------------------------------------------------------------------------------

void Start()
{
    // Allocate a console.
    FILE* f;
    AllocConsole();
    freopen_s(&f, "CONOUT$", "w", stdout);

    bool directoryExists = false;

    // Create a directory to store the dumps.
    if (!directoryExists)
    {
        CreateDirectory("C:\\Dumps", nullptr);
    }

    LPVOID target = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteProcessMemory");

    if (MH_Initialize() != MH_OK)
    {
        NOTIFICATION("[!] Failed to initialize MH, Unloading");
        FreeLibraryAndExitThread(_currentModule, -1);
    }

    if (MH_CreateHook(target, &WriteProcessMemoryDetour, reinterpret_cast<LPVOID*>(&oWriteProcessMemory)) != MH_OK)
    {
        NOTIFICATION("[!] Failed to hook WriteProcessMemory, Unloading");
        FreeLibraryAndExitThread(_currentModule, -1);
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        NOTIFICATION("[!] Failed to enable hook, Unloading");
        FreeLibraryAndExitThread(_currentModule, -1);
    }

    NOTIFICATION("[+] Initialization succeeded");
    NOTIFICATION("[+] Files are dumped in chronological order\n");
}

// --------------------------------------------------------------------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE _module, uintptr_t attachReason, LPVOID reservedMemory)
{
    if (attachReason == DLL_PROCESS_ATTACH)
    {
        _currentModule = _module;

        DisableThreadLibraryCalls(_module);

        // Initialize.
        Start();
    }

    return TRUE;
}

// --------------------------------------------------------------------------------------------------------------------------