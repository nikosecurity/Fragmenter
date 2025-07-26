# Fragmenter
A driver that creates R/W functionality within `int 3` padding bytes between functions, designed for Windows 10 x64 and above.

The code is split into two components: the Fragmenter driver, and the Igniter user-mode loader.

## Note
This has not been tested on a system without test-signing yet. I will be doing so once the technical details of the Fragmenter driver are published.

# Building & Usage
To build both solutions, you will need [the Windows Driver Kit (WDK), the Windows SDK, and Visual Studio 2022](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) (other versions may work, but this is what I use). Simply open the solution and build for Release | x64 and you're good to go.

To use this software in its default state, the Fragmenter backdoor must be named `RWHelper.sys` and the vulnerable driver must be named `mimkrnl.sys`. These also must be in the same directory as the user-mode loader (Igniter). If you would like to change it, you can reference the `Defs.h` header within the Igniter solution directory; there are constants that you can easily modify to your needs.

From here, all that's left is to run the exploit.

# Technical Details
## Igniter
The user-mode program is designed with automation in mind, which means you should be able to run it and immediately gain arbitrary kernel read/write.

The program first uses the [`fodhelper.exe` UAC bypass](https://www.elastic.co/security-labs/exploring-windows-uac-bypasses-techniques-and-detection-strategies) to gain administrative privileges if it was not executed w/ administrative privileges already. Afterwards, it copies the two driver files that were bundled with it to the `%windir%\System32\drivers` directory and loads the `mimkrnl.sys` driver.

The Igniter then leaks out the NT kernel base address, the address to overwrite within `nt!SeCiCallbacks`, and the function address for `ZwFlushInstructionCache`. More information on this in a bit.

The loader abuses an arbitrary kernel write within the driver to disable DSE. The target IOCTL to reach the arbitrary write is `0x3CEC04B`, which translates to `GENERIC_READ | GENERIC_WRITE` access and `METHOD_NEITHER`. This means that the function will be using `Type3InputBuffer`, which is a raw user-mode pointer for the input buffer (the function does not reference the `UserBuffer`). The first function reached checks the first `DWORD` present within the input buffer, and if it is, it will call into the vulnerable function with the input buffer as its argument.

<img width="707" height="408" alt="The first function reached upon sending IOCTL 0x3CEC04B" src="https://github.com/user-attachments/assets/1c8835b9-00d8-4fb6-9779-9e3c6a587c87"/>

The vulnerable function contains multiple vulnerabilities, most notably however are the two arbitrary writes. By providing a proper input structure, the exploit reaches the vulnerable `memmove` function and gains arbitrary kernel write. From here, it's relatively simple to exploit for LPE. However, I couldn't find a way to exploit it on Windows 10 as I could not think of any kernel objects to abuse. Thankfully, this is meant to disable DSE, not elevate privileges.

<img width="988" height="308" alt="The vulnerable function" src="https://github.com/user-attachments/assets/8806e102-6515-4417-8f8a-18d2b9cfadb5"/>

Using the new-found arbitrary write, the exploit overwrites the `CI!CiValidateImageHeader` entry within the callback structure mentioned previously. This allows for unsigned code to be loaded as the function always returns 0 (`STATUS_SUCCESS`). Realistically however, any function that always returns 0 (or can be forced to return 0 consistently) will work.

For more information on the technique, you can take a look [here](https://blog.cryptoplague.net/main/research/windows-research/the-dusk-of-g_cioptions-circumventing-dse-with-vbs-enabled), [here](https://blog.xpnsec.com/gcioptions-in-a-virtualized-world/), and [here](https://www.fortinet.com/blog/threat-research/driver-signature-enforcement-tampering) (also provided within the references chapter). They explain the technique far better than I could (albeit the explanation is still rather convoluted in my opinion).

After disabling DSE, the Fragmenter driver is loaded, and the kernel read/write backdoor is installed.

## Fragmenter
My wrist hurts right now, so I'll take a break from writing documentation and write this later. Check back soon, I hope it's done by then.

# References Used
https://blog.cryptoplague.net/main/research/windows-research/the-dusk-of-g_cioptions-circumventing-dse-with-vbs-enabled (DSE Bypass)

https://blog.xpnsec.com/gcioptions-in-a-virtualized-world/ (DSE Bypass)

https://www.fortinet.com/blog/threat-research/driver-signature-enforcement-tampering (DSE Bypass)

https://stackoverflow.com/questions/14889643/how-encode-a-relative-short-jmp-in-x86 (jmp opcode encoding)

https://github.com/sqrtZeroKnowledge/XWorm-Trojan (UAC bypass, requires reverse engineering the UAC plugin binary w/ dnSpy which was trivial)
