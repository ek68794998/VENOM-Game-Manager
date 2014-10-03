#include "vgm_threads.h"
#include "vgm_net.h"

vConnectionClass::vConnectionClass() : Host(NULL) {
}
void vConnectionClass::ResolveHostname() {
	Address = Packet->Sender;
	timeout = clock() + (Config->ResolveTimeoutDuration * CLOCKS_PER_SEC);
	HostnameFetcher *Resolver = new HostnameFetcher();
	Resolver->ip = Address.sin_addr;
	Resolver->Buffer.sin_port = Address.sin_port;
	Resolver->HostConnection = this;
	Resolver->start();
}
void vConnectionClass::OnHostnameResolved() {
	if (Host && Resolved) {
		int IPR = Check_IPR_Bans(Host);
		if (IPR == 1) {
			vLogger->Log(vLoggerType::vVGM,"_CONNECTION","Hostname %s (%s) is banned. (Nick: %s)",Host,inet_ntoa(Address.sin_addr),PlayerName.c_str());
			Connection->Send_Refusal_Sc(&Address,(REFUSAL_CODE)4);
			Connection->Destroy_Connection(ClientID);
			return;
		}
	} else if (strlen(PlayerName.c_str()) > 0) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","Unable to resolve %s's IP (%s).",PlayerName.c_str(),inet_ntoa(Address.sin_addr));
	} else {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","Unable to resolve IP %s.",inet_ntoa(Address.sin_addr));
	}
	ClientConnectionAccepted(ClientID,*Connection);
}

void __stdcall ClientConnectionHook(Stewie_cConnection &connection, Stewie_cPacket &packet) {
	sockaddr_in PacketAddress = packet.Sender;

	int ClientID = -1;
	for (unsigned int i = connection.MinClientID; i <= connection.MaxRemoteHosts; i++) {
		if (connection.RemoteHosts[i]) {
			if (GameDataObj()->Is_Single_Player()) { continue; }
			if (Stewie_cNetUtil::Is_Same_Address(&connection.RemoteHosts[i]->address,&PacketAddress)) { return; }
		} else if (ClientID == -1) {
			ClientID = i;
		}
	}

	if (ClientID == -1) {
		connection.Send_Refusal_Sc(&PacketAddress,(REFUSAL_CODE)1); // Game is full
		return;
	}

	wchar_t playerName[256];
	int OldReadPos = packet.BitReadPosition;
	packet.Get_Wide_Terminated_String(playerName,sizeof(playerName),true);
	packet.BitReadPosition = OldReadPos;
	char *PacketPlayerName = (char *)WCharToStr(playerName).c_str();

	int Allow = ClientConnectionAllowedCheck(&packet);
	if (Allow == 0) {
		Allow = (int)connection.Application_Acceptance_Handler(packet);
		if (Allow == 1) { Allow = 0; }
	}
	if (Allow != 0) {
		connection.Send_Refusal_Sc(&PacketAddress,(REFUSAL_CODE)Allow);
		return;
	}

	int SetBandwidth;
	packet.Internal_Get(SetBandwidth,-1);
	Stewie_cRemoteHost &LocalRemoteHost = Stewie_cRemoteHost::Create();
	LocalRemoteHost.ClientID = ClientID;
	LocalRemoteHost.address = packet.Sender;
	LocalRemoteHost.Bandwidth = SetBandwidth;
	connection.RemoteHostCount++;
	connection.RemoteHosts[ClientID] = &LocalRemoteHost;

	if (Config->EnableConnectionHook == false || Config->ResolveHostnames == false) {
		ClientConnectionAccepted(ClientID,connection);
	} else {
		vConnectionClass *ClientConnection = new vConnectionClass();

		ClientConnection->ClientID = ClientID;
		ClientConnection->PlayerName = std::string(PacketPlayerName);

		Stewie_cPacket &NewPacket = Stewie_cPacket::Create();
		NewPacket.Sender = packet.Sender;
		NewPacket.Type = packet.Type;
		NewPacket.ID = packet.ID;
		NewPacket.SenderID = packet.SenderID;
		NewPacket.Unk1 = packet.Unk1;
		NewPacket.Unk2 = packet.Unk2;
		NewPacket.Unk3 = packet.Unk3;
		NewPacket.NumSends = packet.NumSends;
		NewPacket.Unk0260 = packet.Unk0260;
		NewPacket.Unk0264 = packet.Unk0264;
		NewPacket.BitWritePosition = packet.BitWritePosition;
		NewPacket.BitReadPosition = packet.BitReadPosition;
		for (unsigned int i = 0; i < sizeof(packet.Data); i++) {
			NewPacket.Data[i] = packet.Data[i];
		}

		ClientConnection->Packet = &NewPacket;
		ClientConnection->Connection = &connection;
		ClientConnection->ResolveHostname();
	}
}
void ClientConnectionAccepted(int ClientID, Stewie_cConnection &connection) {
	connection.Send_Accept_Sc(ClientID);
	connection.Conn_Handler(ClientID);
	connection.RemoteHosts[ClientID]->ReliablePacketReceiveID++;
}
int ClientConnectionAllowedCheck(Stewie_cPacket *packet) {
	in_addr ip = packet->Sender.sin_addr;
	const char *address = inet_ntoa(ip);

	wchar_t playerName[256];
	int OldReadPos = packet->BitReadPosition;
	packet->Get_Wide_Terminated_String(playerName,sizeof(playerName),true);
	packet->BitReadPosition = OldReadPos;
	char pnameraw[64];
	sprintf(pnameraw,"%s",WCharToStr(playerName).c_str());
	const char *pname = (const char *)pnameraw;
	
	bool UsedReservedSlot = false;
	int RS = Check_Reserved_Slot(address),RS2 = Check_Reserved_Slot(pname);
	if ((unsigned int)(Stewie_cPlayerManager::Count()) >= GameDataObj()->MaxPlayers) {
		if (Config->EnableConnectionHook == false) { return 1; }
		if (RS != 1 && RS2 != 1) { return 1; }
		UsedReservedSlot = true;
	}

	if (Config->EnableConnectionHook == false) { return 0; }

	if (strlen(pname) > 30 || strlen(pname) < 2) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","A user with IP %s tried to connect with invalid nick \"%s\". Their connection has been blocked. (Error: Invalid length.)",address,pname);
		return 3;
	} else if (strstr(pname," ") || strstr(pname,"$") || strstr(pname,"%") || strstr(pname,";") || strstr(pname,"\\") || strstr(pname,"\"") || strstr(pname,"\'") || strstr(pname,":")) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","A user with IP %s tried to connect with invalid nick \"%s\". Their connection has been blocked. (Error: Invalid character detected.)",address,pname);
		return 3;
	} else if (!stricmp(pname,"Unnamed") || !stricmp(pname,"Renegade") || !stricmp(pname,"None")) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","A user with IP %s tried to connect with invalid nick \"%s\". Their connection has been blocked. (Error: Invalid multiplayer nickname.)",address,pname);
		return 4;
	} else if (!stricmp(pname,WCharToStr(GameDataObj()->Owner.m_Buffer).c_str())) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","A user with IP %s tried to connect with nick \"%s\", but this is the Hostname for the server. Their connection has been blocked. (Error: LAN Exploit.)",address,pname);
		return 3;
	} else {
		for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
			Stewie_cPlayer *p = Node->NodeData;
			if (!p) { continue; }
			if (p->IsActive == false || p->IsInGame == false) { continue; }
			if (!stricmp(pname,WCharToStr(p->PlayerName.m_Buffer).c_str())) {
				vLogger->Log(vLoggerType::vVGM,"_CONNECTION","A user with IP %s tried to connect with nick \"%s\", but that nick is already in use ingame. Their connection has been blocked.",address,pname);
				return 4;
			}
		}
	}

	int MIP = Check_Moderator_IP(pname,address);
	if (MIP == 2) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","User with moderator name %s was blocked trying to connect from IP address %s, which is not in their allowed IP range.",pname,address);
		return 3;
	}

	int IPR = Check_IPR_Bans(address);
	if (IPR == 1) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","IP %s is banned. (Nick: %s)",address,pname);
		return 4;
	}

	if (UsedReservedSlot) {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","User %s (IP: %s) was able to connect through a reserved slot.",pname,address);
	}

	return 0;
}

// -------------------------

bool is_cs_cid(int cid) {
	switch (cid) {
		case 1000: // BaseGameObj (NetworkGameObjectFactoryClass)
		case 1001: // cScTextObj
		case 1002: // cPlayerKill
		case 1003: // cWinEvent
		case 1004: // cPurchaseResponseEvent
		case 1005: // cConsoleCommandEvent
		case 1007: // cSvrGoodbyeEvent
		case 1008: // cGameOptionsEvent
		case 1009: // cEvictionEvent
		case 1010: // cTeam
		case 1011: // cPlayer
		case 1012: // cGameDataUpdateEvent
		case 1013: // cScPingResponseEvent
		case 1014: // cScExplosionEvent
		case 1016: // SCAnnouncement
		case 1017: // cGameSpyScChallengeEvent
		case 1022: // cMoneyEvent
		case 1023: // cWarpEvent
		case 1028: // cGodModeEvent
		case 1029: // cVipModeEvent
		case 1030: // cScoreEvent
		case 1035: // cRequestKillEvent
		case 1036: // cCsConsoleCommandEvent
		case 1039: // cDonateEvent
			return false;
		default:
			return true;
	}
}

Stewie_NetworkObjectFactoryClass* __stdcall HackSafeFindNetworkObjectFactory(unsigned int cid, int sid, Stewie_cPacket& Packet) {
	if (is_cs_cid(cid)) {
		return Stewie_NetworkObjectFactoryMgrClass::Find_Factory(cid);
	} else {
		vLogger->Log(vLoggerType::vVGM,"_CHEAT","Player %s tried to execute a netcode hack [NTC:01, CID:%d, IP:%s].",Player_Name_From_ID(sid).c_str(),cid,inet_ntoa(Packet.Sender.sin_addr));
		return NULL;
	}
}
__declspec(naked) void HackSafeFindNetworkObjectFactoryNaked() {
	__asm {
		push esi
		push ebp
		push [esp+8]
		call HackSafeFindNetworkObjectFactory
		test eax,eax
		je Error

		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx

	Error:
		add esp,4
		pop ebx
		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump HackSafeFindNetworkObjectFactoryHook(0x000000, &HackSafeFindNetworkObjectFactoryNaked); // For security and licensing purposes, an address has been hidden from this line

Stewie_NetworkObjectClass* __stdcall HackSafeFindNetworkObject(int oid, int sid, Stewie_cPacket& Packet) {
	Stewie_NetworkObjectClass* o = Stewie_NetworkObjectMgrClass::Find_Object(oid);
	if (!o) { return NULL; }
	int cid = o->Get_Network_Class_ID();
	if (is_cs_cid(cid)) {
		return o;
	} else {
		vLogger->Log(vLoggerType::vVGM,"_CHEAT","Player %s tried to execute a netcode hack [NTC:02, OID:%d, CID:%d, IP:%s].",Player_Name_From_ID(sid).c_str(),oid,cid,inet_ntoa(Packet.Sender.sin_addr));
		return NULL;
	}
}
__declspec(naked) void HackSafeFindNetworkObjectNaked() {
	__asm {
		push esi
		push ebp
		push [esp+0xC]
		call HackSafeFindNetworkObject
		retn
	}
}
ASMHackCall HackSafeFindNetworkObjectHook(0x000000, &HackSafeFindNetworkObjectNaked); // For security and licensing purposes, an address has been hidden from this line

unsigned long __stdcall RequestSenderId(Stewie_cPacket& Packet, unsigned long& trgt, int trgtlen) {
	Packet.Get_Bits(trgt, trgtlen);
	int sid = Packet.SenderID;
	if ((int)trgt != sid) {
		vLogger->Log(vLoggerType::vVGM,"_CHEAT","Player %s tried to execute a netcode hack [NTC:03, SID:%d, TRGT:%d/%s, IP:%s].",Player_Name_From_ID(sid).c_str(),sid,trgt,Player_Name_From_ID(trgt).c_str(),inet_ntoa(Packet.Sender.sin_addr));
		trgt = sid;
	}
	return trgt;
}
__declspec(naked) void RequestSenderIdNaked() {
	__asm {
		push [esp+8]
		push [esp+8]
		push ecx
		call RequestSenderId
		retn 8
	}
}
ASMHackCall RequestSenderIdHook01(0x000000, &RequestSenderIdNaked); // CSAnnouncement // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook02(0x000000, &RequestSenderIdNaked); // cBioEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook03(0x000000, &RequestSenderIdNaked); // cChangeTeamEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook04(0x000000, &RequestSenderIdNaked); // cClientBboEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook05(0x000000, &RequestSenderIdNaked); // cClientGoodbyeEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook06(0x000000, &RequestSenderIdNaked); // cCsHint // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook07(0x000000, &RequestSenderIdNaked); // cCsPingRequestEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook08(0x000000, &RequestSenderIdNaked); // cCsTextObj // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook09(0x000000, &RequestSenderIdNaked); // cGameSpyCsChallengeResponseEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook10(0x000000, &RequestSenderIdNaked); // cLoadingEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook11(0x000000, &RequestSenderIdNaked); // cPurchaseRequestEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook12(0x000000, &RequestSenderIdNaked); // cSuicideEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook13(0x000000, &RequestSenderIdNaked); // CClientFps // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook14(0x000000, &RequestSenderIdNaked); // cCsDamageEvent // For security and licensing purposes, an address has been hidden from this line
ASMHackCall RequestSenderIdHook15(0x000000, &RequestSenderIdNaked); // CClientControl // For security and licensing purposes, an address has been hidden from this line

// -------------------------

void __stdcall RadioMessageHook(Stewie_CSAnnouncement& Event, Stewie_cPacket& Packet) {
	int sid = Packet.SenderID;

	vPlayer *player = vPManager->Get_Player(Player_Name_From_ID(sid).c_str());
	if (player) {
		if (player->muted) {
			Event.Set_Delete_Pending();
			return;
		}
	}

	Stewie_cPlayer* Player = Stewie_cPlayerManager::Find_Player(sid);
	if (!Player || Player->PlayerType != Event.PlayerType) {
		vLogger->Log(vLoggerType::vVGM,"_CHEAT","Player %s tried to execute a netcode hack [NTC:04, SID:%d, IP:%s].",Player_Name_From_ID(sid).c_str(),sid,inet_ntoa(Packet.Sender.sin_addr));
		Event.Set_Delete_Pending();
		return;
	} else if (Event.AnnouncementId < 8535 || Event.AnnouncementId >= 8565 || Event.IconId < 0 || Event.IconId >= 30 || Event.AnnouncementId != (int)(Event.IconId + 8535)) {
		Event.Set_Delete_Pending();
		return;
	}

	Event.Act();
}
__declspec(naked) void RadioMessageNaked() {
	__asm {
		push edi
		push ecx
		call RadioMessageHook
		pop edi
		pop esi
		retn 4
	}
}
ASMHackJump RadioMessageHack(0x000000, &RadioMessageNaked); // For security and licensing purposes, an address has been hidden from this line

// -------------------------

__declspec(naked) void UdpFixHook() {
	static int MsgCount = 0;
	__asm {
		/* Due to licensing, portions of this file have been omitted. */
	}
}
ASMHackJump UdpFixHack(0x000000, &UdpFixHook); // For security and licensing purposes, an address has been hidden from this line
