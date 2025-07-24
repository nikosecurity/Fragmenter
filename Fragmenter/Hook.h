#pragma once

NTSTATUS ReadWriteMemory(PVOID pTargetAddress, PVOID pData, ULONG ByteCount, UCHAR IsWrite);
NTSTATUS ScanForPaddingBytes(ULONG_PTR BaseAddress, ULONG ScanAddress, ULONG SizeOfCodeSegment, PULONG_PTR pFoundAddress);
NTSTATUS InstallBackdoor(void);