#include <Windows.h>

#include "Defs.h"

char MoveDriver(char* pDriverName, char* pExistingPath, char* pNewPath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char pFilePath[MAX_PATH] = { 0 };
	char pFilePath2[MAX_PATH] = { 0 };

	strcat_s(pFilePath, sizeof(pFilePath), pExistingPath);
	strcat_s(pFilePath, sizeof(pFilePath), pDriverName);
	strcat_s(pFilePath2, sizeof(pFilePath2), pNewPath);
	strcat_s(pFilePath2, sizeof(pFilePath2), pDriverName);

	hFile = CreateFileA(pFilePath2, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (!MoveFileA(pFilePath, pFilePath2))
		{
			return 1;
		}
	}
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	return 0;
}

char LoadDriver(char* pDriverName, char* pServiceName, char FailOnFailedStart)
{
	SC_HANDLE hServiceManager = 0;
	SC_HANDLE hService = 0;
	char pFullDriverPath[MAX_PATH] = { 0 };
	SERVICE_STATUS ServiceStatus = { 0 };

	memset(pFullDriverPath, 0, sizeof(pFullDriverPath));
	memset(&ServiceStatus, 0, sizeof(ServiceStatus));

	hServiceManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!hServiceManager)
	{
		return 1;
	}

	strcat_s(pFullDriverPath, sizeof(pFullDriverPath), DRIVER_PATH);
	strcat_s(pFullDriverPath, sizeof(pFullDriverPath), pDriverName);

	hService = CreateServiceA
	(
		hServiceManager,
		pServiceName,
		pServiceName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		pFullDriverPath,
		0,
		0,
		0,
		0,
		0
	);
	if (!hService)
	{
		hService = OpenServiceA(hServiceManager, pServiceName, SERVICE_ALL_ACCESS);
		if (!hService)
		{
			return 1;
		}
	}

	if (!QueryServiceStatus(hService, &ServiceStatus))
	{
		return 1;
	}

	// StartServiceA is a blocking call, so there should be no reason to add a Sleep call at the end of the function.
	if (
		ServiceStatus.dwCurrentState != SERVICE_RUNNING &&
		ServiceStatus.dwCurrentState != SERVICE_START_PENDING &&
		!StartServiceA(hService, 0, 0) &&
		FailOnFailedStart
		)
	{
		return 1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hServiceManager);

	return 0;
}

char UnloadDriver(char* pDriverName, char* pServiceName)
{
	SC_HANDLE hServiceManager = 0;
	SC_HANDLE hService = 0;
	char pFilePath[MAX_PATH] = { 0 };

	memset(pFilePath, 0, sizeof(pFilePath));

	hServiceManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!hServiceManager)
	{
		return 1;
	}

	hService = OpenServiceA(hServiceManager, pServiceName, SERVICE_ALL_ACCESS);
	if (!hService)
	{
		return 1;
	}

	DeleteService(hService);

	strcat_s(pFilePath, sizeof(pFilePath), DRIVER_PATH);
	strcat_s(pFilePath, sizeof(pFilePath), pDriverName);

	DeleteFileA(pFilePath);

	CloseServiceHandle(hService);
	CloseServiceHandle(hServiceManager);

	return 0;
}