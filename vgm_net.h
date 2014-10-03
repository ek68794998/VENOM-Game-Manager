#pragma once

#include "vgm_engine.h"
#include "vgm_logging.h"
#include "vgm_player.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

void __stdcall ClientConnectionHook(Stewie_cConnection &connection, Stewie_cPacket &packet);
void ClientConnectionAccepted(int ClientID, Stewie_cConnection &connection);
int ClientConnectionAllowedCheck(Stewie_cPacket *packet);

class vConnectionClass {
public:
	int ClientID;
	std::string PlayerName;
	sockaddr_in Address;
	Stewie_cPacket *Packet;
	Stewie_cConnection *Connection;
	bool Resolved;
	const char *Host;
	clock_t timeout;

	vConnectionClass();
	void ResolveHostname();
	void OnHostnameResolved();
	void AcceptConnection();
};
