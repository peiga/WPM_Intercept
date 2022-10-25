#pragma once

#include "Includes.hpp"

// --------------------------------------------------------------------------------------------------------------------------

namespace HookRelated
{
	typedef BOOL(WINAPI* WriteProcessMemoryFn)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*);
	inline WriteProcessMemoryFn OriginalWriteProcessMemory = nullptr;
	BOOL WINAPI WriteProcessMemoryDetour(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
}

// --------------------------------------------------------------------------------------------------------------------------