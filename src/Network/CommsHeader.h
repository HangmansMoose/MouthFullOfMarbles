#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#define BUFFLENGTH 16384
#define TLS_MAX_PACKET_SIZE (BUFFLENGTH + 512)

#include <winsock2.h>

#include <security.h>
#include <schannel.h>
#include <shlwapi.h>


#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "secur32.lib")
#pragma comment (lib, "shlwapi.lib")

