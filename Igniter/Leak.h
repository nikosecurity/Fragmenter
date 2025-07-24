#pragma once

// 4c 8d 05 4e e9 45 00 8b
// { 0x4C, 0x8D, 0x05, 0x4E, 0xE9, 0x45, 0x00, 0x8B };
#define SECICALLBACKS_SIGNATURE 0x8B0045E94E058D4C

unsigned long long GetBaseAddress(char* pTargetDriverName);
unsigned long long GetFunctionAddress(unsigned long long BaseAddress, char* pDriverName, char* pFunctionName);
unsigned long long GetDSEOverwriteAddress(unsigned long long BaseAddress);