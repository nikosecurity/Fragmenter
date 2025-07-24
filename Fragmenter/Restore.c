#include <ntddk.h>

#include "Leak.h"
#include "Restore.h"

// TODO: Refer to the Leak.c code in the Igniter code-base and replicate it here.
// This function is incomplete.
//
// Patch back the overwritten callback within ntoskrnl.exe.
// A bit more specifically, restore the CI!CiValidateImageHeader callback in nt!SeCiCallbacks.
NTSTATUS RestoreCallbacks(void)
{
	NTSTATUS Status = STATUS_SUCCESS;

	// TODO: ...

	return Status;
}

// TODO: ...
// Remove the loaded driver from the unloaded modules list.
// There are some other locations as well, reference UnknownCheats for locations to clean.
NTSTATUS CleanTraces(void)
{
	NTSTATUS Status = STATUS_SUCCESS;

	// TODO: ...

	return Status;
}