#pragma once

NTSTATUS LeakDriverInfo(const char* pDriverName, PVOID* pImageBase, PULONG pImageSize);