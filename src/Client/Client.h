#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 

#include <cctype>
#include <stdint.h>
#include <windows.h>


class Client {
public:
	Client() = default;
	~Client() = default; // TODO: this needs to clean up correctly, including threads, buffers etc..
	void Run(WCHAR* pIpv4Addr, uint16_t nPort);
};
