#include <ntddk.h>
#include <aux_klib.h>

#include "Defs.h"

NTSTATUS LeakDriverInfo(const char* pDriverName, PVOID* pImageBase, PULONG pImageSize)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PAUX_MODULE_EXTENDED_INFO pModuleInfoArray = 0;
	PAUX_MODULE_EXTENDED_INFO CurrentModuleInfo = { 0 };
	ULONG BufferSize = 0;

	UCHAR Found = 0;

	memset(&CurrentModuleInfo, 0, sizeof(CurrentModuleInfo));

	AuxKlibQueryModuleInformation(&BufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), 0);

	pModuleInfoArray = ExAllocatePool2(POOL_FLAG_NON_PAGED, BufferSize, RWHELPER_POOL_TAG);
	if (!pModuleInfoArray)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	memset(pModuleInfoArray, 0, BufferSize);

	Status = AuxKlibQueryModuleInformation(&BufferSize, sizeof(AUX_MODULE_EXTENDED_INFO), pModuleInfoArray);
	if (!NT_SUCCESS(Status))
	{
		ExFreePoolWithTag(pModuleInfoArray, RWHELPER_POOL_TAG);
		return Status;
	}

	for (ULONG i = 0; i < BufferSize / sizeof(AUX_MODULE_EXTENDED_INFO); i++)
	{
		// The goddess of disgust.
		CurrentModuleInfo = (PAUX_MODULE_EXTENDED_INFO)((PUCHAR)(pModuleInfoArray)+(i * sizeof(AUX_MODULE_EXTENDED_INFO)));

		if (strstr(pDriverName, (const char*)CurrentModuleInfo->FullPathName))
		{
			Found = 1;

			*pImageBase = CurrentModuleInfo->BasicInfo.ImageBase;
			*pImageSize = CurrentModuleInfo->ImageSize;
			break;
		}
	}
	ExFreePoolWithTag(pModuleInfoArray, RWHELPER_POOL_TAG);

	return Found ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}
