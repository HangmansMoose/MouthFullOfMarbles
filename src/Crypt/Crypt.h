#pragma once

#include "../Network/CommsHeader.h"
#include <assert.h>
#include <stdio.h>


#define TLS_MAX_PACKET_SIZE (16384+512) // payload + extra over head for header/mac/padding (probably an overestimate)

typedef struct {
	SOCKET sock;
	CredHandle handle;
	CtxtHandle context;
	SecPkgContext_StreamSizes sizes;
	int received;    // byte count in incoming buffer (ciphertext)
	int used;        // byte count used from incoming buffer to decrypt current packet
	int available;   // byte count available for decrypted bytes
	char* decrypted; // points to incoming buffer where data is decrypted inplace
	char incoming[TLS_MAX_PACKET_SIZE];
} tls_socket;