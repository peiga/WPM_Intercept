#include "../Header Files/Includes.hpp"
#include "../Header Files/Hooks.hpp"

// --------------------------------------------------------------------------------------------------------------------------

void Start()
{
    if (Globals::ShouldAllocateConsole)
    {
        FILE* FilePointer{};
        AllocConsole();
        freopen_s(&FilePointer, "CONOUT$", "w", stdout);
    }

    // Clear the console just in case.
    system("cls");

    const bool DoesDirectoryExist = std::filesystem::is_directory(DIRECTORY);

    // Create a directory to store the dumps.
    if (!DoesDirectoryExist)
        CreateDirectory(DIRECTORY, nullptr);

    const LPVOID WriteProcessMemoryAddress = (LPVOID)(GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteProcessMemory"));

    if (MH_Initialize() != MH_OK)
    {
        std::cout << "[!] Failed to initialize MH, Unloading" << std::endl;
        FreeLibraryAndExitThread(Globals::ThisModule, -1);
    }

    if (MH_CreateHook(WriteProcessMemoryAddress, &HookRelated::WriteProcessMemoryDetour, reinterpret_cast<LPVOID*>(&HookRelated::OriginalWriteProcessMemory)) != MH_OK)
    {
        std::cout << "[!] Failed to hook WriteProcessMemory, Unloading" << std::endl;
        FreeLibraryAndExitThread(Globals::ThisModule, -1);
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
    {
        std::cout << "[!] Failed to enable hook, Unloading" << std::endl;
        FreeLibraryAndExitThread(Globals::ThisModule, -1);
    }

    std::cout << "[+] Initialization succeeded" << std::endl;
    std::cout << "[+] Files are dumped in chronological order\n" << std::endl;
}

// --------------------------------------------------------------------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE ModuleAddress, uintptr_t AttachReason, LPVOID ReservedMemory)
{
    if (AttachReason == DLL_PROCESS_ATTACH)
    {
        Globals::ThisModule = ModuleAddress;

        DisableThreadLibraryCalls(ModuleAddress);

        MessageBox(nullptr, "Do you want to create a console window?\nNote: If the process already has a console created press 'No'", "Confirmation", MB_YESNO) == IDYES
            ? Globals::ShouldAllocateConsole = true
            : Globals::ShouldAllocateConsole = false;

        // Initialize.
        Start();
    }

    return TRUE;
}

// --------------------------------------------------------------------------------------------------------------------------