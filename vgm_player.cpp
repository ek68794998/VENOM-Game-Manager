#include "vgm_player.h"

vPlayerManager *vPManager;
vVeteranManager *vVetManager;

// ---------------------------
//
//  Player Events
//
// ---------------------------
void __stdcall PlayerJoinHook(Stewie_cBioEvent *Event) {
	vPlayer *p = vPManager->Get_Player(WCharToStr(Event->PlayerName.m_Buffer).c_str());
	if (!p) { p = vPManager->Add_Player(Event->PlayerId); }

	p->Join_Game(Event->PlayerId);
	//vAManager->Played_Joined(Event->PlayerId);
	InvObj->Start_Custom_Timer(InvObj,2.0f,800001,Event->PlayerId);
	Request_Serial(Event->PlayerId);

	Stewie_cTeamManager::Find_Team(0)->Set_Object_Dirty_Bit(vDBOCCASIONAL,true);
	Stewie_cTeamManager::Find_Team(1)->Set_Object_Dirty_Bit(vDBOCCASIONAL,true);
}
void __stdcall ClientDisconnectHook(int PlayerID) {
	if (PlayerID <= 0) { return; }

	Stewie_cPlayer *p = Stewie_cPlayerManager::Find_Player(PlayerID);
	if (p) {
		std::string name = WCharToStr(p->PlayerName.m_Buffer);
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","Connection lost to %s. (Ping: %d)",name.c_str(),p->Get_Ping());
		PrivMsgColoredVA(0,2,0,200,0,"[VGM] Player %s disconnected.",name.c_str());
		if (Config->Sounds) { Create_2D_WAV_Sound("connection_broken.wav"); }
	} else {
		vLogger->Log(vLoggerType::vVGM,"_CONNECTION","Connection lost to player %d.",PlayerID);
	}
	
	vPlayer *vp = vPManager->Get_Player(PlayerID);
	if (vp) { vp->Left_Game(); }
}
void __stdcall PlayerLeaveHook(int PlayerID) {
	Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(PlayerID);
	if (player && Commands->Get_Health(Player_GameObj(PlayerID)) <= 0.0f) { Inc_Deaths(player,1); }

	vVManager->Unbind_All_Player_Vehicles(PlayerID);
	Disarm_Beacons(PlayerID);
	if (Is_Commander(PlayerID,0,false)) {
		Remove_Team_Commander(0);
		InvObj->Start_Custom_Timer(InvObj,2.0f,800002,0);
	} else if (Is_Commander(PlayerID,1,false)) {
		Remove_Team_Commander(1);
		InvObj->Start_Custom_Timer(InvObj,2.0f,800002,1);
	}

	vPlayer *vp = vPManager->Get_Player(PlayerID);
	if (vp) { vp->Left_Game(); }
}
void __stdcall PlayerTeamHook(Stewie_cPlayer *Player, int Team) {
	if (Player->IsActive) { // Otherwise this enables on join, and we don't want that.
		Disarm_Beacons(Player->PlayerId);
		Disarm_All_C4(Player->PlayerId);

		int oldteam = Player->PlayerType;
		if (Is_Commander(Player->PlayerId,oldteam,false)) {
			Remove_Team_Commander(oldteam);
			InvObj->Start_Custom_Timer(InvObj,2.0f,800002,0);
			InvObj->Start_Custom_Timer(InvObj,2.0f,800002,1);
		}

		vVManager->Unbind_All_Player_Vehicles(Player->PlayerId);
		Inc_Deaths(Player,-1);

		vPlayer *vp = vPManager->Get_Player(Player->PlayerId);
		if (vp) { vp->Clear_Hits(); }
	}
}
void __stdcall SerialRetrievedHook(Stewie_cGameSpyCsChallengeResponseEvent* data) {
	int ID = data->PlayerId;
	std::string nick = Player_Name_From_ID(ID);
	vPlayer *player = vPManager->Get_Player(nick.c_str());
	if (!player) { return; }
	Stewie_StringClass hash = data->response;
	if (!hash.m_Buffer || strlen(hash.m_Buffer) <= 0) {
		vLogger->Log(vLoggerType::vVGM,"_SERIAL","Player %s is using an invalid serial hash.",nick.c_str());
		player->ShouldKick = true;
		return;
	}
	char *rawserial = hash.m_Buffer;
	std::string serial (rawserial);
	serial.erase(32);
	const char *serialhash = serial.c_str();
	if (!Serial_Is_Valid(serialhash)) {
		vLogger->Log(vLoggerType::vVGM,"_SERIAL","Player %s is using an invalid serial hash: %s",nick.c_str(),serialhash);
		HostPrivateAdminMessage(ID,"[VGM] Invalid serial hash detected.");
		player->ShouldKick = true;
	} else {
		if (getProfileInt("HashBans",serialhash,0,"vgm_bans.ini") == 1) {
			vLogger->Log(vLoggerType::vVGM,"_SERIAL","Serial of player %s (%s) is in ban list.",nick.c_str(),serialhash);
			HostPrivateAdminMessage(ID,"[VGM] You are banned from this server.");
			player->ShouldKick = true;
		} else {
			vLogger->Log(vLoggerType::vVGM,"_SERIAL","Player %s is using serial hash: %s",nick.c_str(),serialhash);
			player->Set_Serial(true);
		}
	}
}
void __stdcall PlayerKillMsgHook(Stewie_cPlayerKill &e, int Killer, int Victim) {
	e.SenderId = Killer;
	e.ReceiverId = Victim;
	e.Set_Object_Dirty_Bit(vDBCREATION,false);
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		vPlayer *p = vPManager->Get_Player(Node->NodeData->PlayerId);
		if (!p) { continue; }
		if (p->bhsVersion < 2.6f) {
			e.Set_Object_Dirty_Bit(Node->NodeData->PlayerId,vDBCREATION,true);
			Stewie_cNetwork::Send_Object_Update(&e,Node->NodeData->PlayerId);
		} else if (p->ID == Killer) {
			Create_2D_WAV_Sound_Player(Player_GameObj(Killer),"correction_3.wav");
		}
	}
}

// ---------------------------
//
//  Player Functions
//
// ---------------------------
int Player_ID_From_GameObj(Stewie_BaseGameObj *obj) {
	if (!obj) { return 0; }
	Stewie_SoldierGameObj *soldier = obj->As_ScriptableGameObj()->As_SoldierGameObj();
	if (soldier && soldier->Player) { return soldier->Player->PlayerId; }
	return 0;
}
int Player_ID_From_Name(const char *name) {
	if (!name) { return 0; }
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		Stewie_cPlayer *p = Node->NodeData;
		if (!p) { continue; }
		if (p->IsInGame == false || p->IsActive == false) { continue; }
		if (!stricmp(WCharToStr(p->PlayerName.m_Buffer).c_str(),name)) {
			return p->PlayerId;
		}
	}
	return 0;
}
std::string Player_Name_From_GameObj(GameObject *obj) {
	return Player_Name_From_GameObj((Stewie_BaseGameObj *)obj);
}
std::string Player_Name_From_GameObj(Stewie_BaseGameObj *obj) {
	if (!obj) { return std::string("None"); }
	Stewie_SoldierGameObj *soldier = obj->As_ScriptableGameObj()->As_SoldierGameObj();
	if (soldier && soldier->Player) { return WCharToStr(soldier->Player->PlayerName.m_Buffer); }
	return std::string("None");
}
std::string Player_Name_From_ID(int ID) {
	if (ID <= 0) { return std::string("None"); }
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		Stewie_cPlayer *p = Node->NodeData;
		if (!p) { continue; }
		if (p->IsInGame == false || p->IsActive == false) { continue; }
		if (p->PlayerId == ID) {
			return WCharToStr(p->PlayerName.m_Buffer);
		}
	}
	return std::string("None");
}
GameObject *Player_GameObj_By_Name(const char *name) {
	if (!name) { return NULL; }
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		Stewie_cPlayer *p = Node->NodeData;
		if (!p) { continue; }
		if (p->IsInGame == false || p->IsActive == false) { continue; }
		if (!stricmp(WCharToStr(p->PlayerName.m_Buffer).c_str(),name)) {
			if (!p->Owner.Reference) { break; }
			if (!p->Owner.Reference->obj) { break; }
			if (!p->Owner.Reference->obj->As_SoldierGameObj()) { break; }
			return (GameObject *)p->Owner.Reference->obj;
		}
	}
	return NULL;
}
GameObject *Player_GameObj_By_Part_Name(const char *name) {
	if (!name) { return NULL; }
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		Stewie_cPlayer *p = Node->NodeData;
		if (!p) { continue; }
		if (p->IsInGame == false || p->IsActive == false) { continue; }
		if (isin(WCharToStr(p->PlayerName.m_Buffer).c_str(),name)) {
			if (!p->Owner.Reference) { break; }
			if (!p->Owner.Reference->obj) { break; }
			if (!p->Owner.Reference->obj->As_SoldierGameObj()) { break; }
			return (GameObject *)p->Owner.Reference->obj;
		}
	}
	return NULL;
}
GameObject *Player_GameObj(int ID) {
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		Stewie_cPlayer *p = Node->NodeData;
		if (!p) { continue; }
		if (p->IsInGame == false || p->IsActive == false) { continue; }
		if (p->PlayerId == ID) {
			if (!p->Owner.Reference) { break; }
			if (!p->Owner.Reference->obj) { break; }
			if (!p->Owner.Reference->obj->As_SoldierGameObj()) { break; }
			return (GameObject *)p->Owner.Reference->obj;
		}
	}
	return NULL;
}
void Evict_Player(int ID) {
	if (!Stewie_cPlayerManager::Is_Player_Present(ID)) { return; }
	ConsoleIn("kick %d",ID);

	if (!Stewie_cPlayerManager::Is_Player_Present(ID)) { return; }
	Stewie_cNetwork::Server_Kill_Connection(ID);
	Stewie_cNetwork::Cleanup_After_Client(ID);

	vPlayer *Kicked = vPManager->Get_Player(ID);
	if (Kicked) { Kicked->ShouldKick = false; }
}
void New_Chat_Message(int sender, int receiver, const char *string, bool popup, int type) {
	char cstr[512];
	sprintf(cstr,"%s",string);
	wchar_t wcs[256];
	swprintf(wcs,L"%S",cstr);
	Stewie_WideStringClass WideStr(wcs);
	Stewie_cScTextObj &newobj = Stewie_cScTextObj::Create();
	if (type == 1) {
		for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
			if (!Node->NodeData) { continue; }
			if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
			int ID = Node->NodeData->PlayerId;
			GameObject *o = Player_GameObj(ID);
			int team = Commands->Get_Player_Type(o);
			if (!o || !Is_Soldier(o) || team == Commands->Get_Player_Type(Player_GameObj(sender))) { continue; }
			if (!stricmp(Get_Preset_Name(o),"CnC_Nod_FlameThrower_2SF") && Is_Script_Attached(o,"Invisible")) {
				//newobj.Set_Object_vDIRTYBIT(ID,vDBALL,true);
				char m[512];
				sprintf(m,"%s: %s",Player_Name_From_ID(sender).c_str(),string);
				Send_Message_Player(o,255,team == 1 ? 255 : 0,0,m);
			}
		}
	}
	newobj.Init(WideStr,type,popup,sender,receiver);
}
void PrivMsgColoredVA(int receiver, int team, unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...) {
	char b[512];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if (team != 2 && Node->NodeData->PlayerType.Get() != team) { continue; }
		if (receiver < 0 && Node->NodeData->PlayerId == -1 * receiver) { continue; }
		else if (receiver > 0 && Node->NodeData->PlayerId != receiver) { continue; }
		vPlayer *p = vPManager->Get_Player(Node->NodeData->PlayerId);
		if (p) {
			if (p->bhsVersion >= 2.6) { Send_Message_Player(Player_GameObj(p->ID),red,green,blue,b); }
			else { HostPrivatePageNoVA(p->ID,b); }
		} else {
			HostPrivatePageNoVA(Node->NodeData->PlayerId,b);
		}
	}
}
void PrivColoredVA(int receiver, int team, unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...) {
	char b[512];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if (team != 2 && Node->NodeData->PlayerType.Get() != team) { continue; }
		if (receiver < 0 && Node->NodeData->PlayerId == -1 * receiver) { continue; }
		else if (receiver > 0 && Node->NodeData->PlayerId != receiver) { continue; }
		vPlayer *p = vPManager->Get_Player(Node->NodeData->PlayerId);
		if (p && p->bhsVersion >= 2.6) { Send_Message_Player(Player_GameObj(p->ID),red,green,blue,b); }
	}
}
void Send_Message_VA(unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...) {
	char b[512];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);
	Send_Message(red,green,blue,b);
}
int Get_KBPS(int ID) {
	int k;
	__asm {
		push ID
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov ecx,[eax]
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		test eax,eax
		jz loc1
		mov ecx,eax
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		mov ecx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		push eax
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		mov k,eax
		jmp loc2
		loc1:
			mov k,0
		loc2:
	}
	return k >> 0x0A;
}
float Get_KD_Ratio(Stewie_cPlayer *player) {
	if (!player || player->IsInGame == false) { return -1.0f; }
	int kills = (int)(player->Kills);
	int deaths = (int)(player->Deaths);
	if (deaths <= 0) { return -1.0; }
	return (float)kills/(float)deaths;
}
int GetMaxPlayerID() {
	return (int)Stewie_cNetwork::PServerConnection->RemoteHostCount;
}
void Set_Kills(Stewie_cPlayer *player, int Kills) {
	if (!player) { return; }
	player->Kills.Set(Kills);
}
void Set_Deaths(Stewie_cPlayer *player, int Deaths) {
	if (!player) { return; }
	player->Deaths.Set(Deaths);
}
void Inc_Kills(Stewie_cPlayer *player, int Kills) {
	if (!player) { return; }
	Kills += Get_Kills(player->PlayerId);
	player->Kills.Set(Kills);
}
void Inc_Deaths(Stewie_cPlayer *player, int Deaths) {
	if (!player) { return; }
	Deaths += Get_Deaths(player->PlayerId);
	player->Deaths.Set(Deaths);
}
void Request_Serial(int ID) {
	Stewie_cPlayer *p = Stewie_cPlayerManager::Find_Player(ID);
	if (!p) { return; }
	const char *key = Stewie_CCDKeyAuth::GenChallenge(8);
	Stewie_StringClass k;
	k.m_Buffer = (char *)key;
	p->GameSpyChallengeString = k;
	__asm {
		mov eax, 0x000000 // For security and licensing purposes, an address has been hidden from this line
		push 0x6B8
		call eax
		mov ecx, eax
		mov eax, 0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		mov ecx, eax
		mov eax, 0x000000 // For security and licensing purposes, an address has been hidden from this line
		lea edx, key
		push edx
		push ID
		call eax
	}
}
bool Serial_Is_Valid(const char *serial) {
	if (!serial) { return false; }
	if (strlen(serial) != 32) { return false; }
	char* in = new char[strlen(serial) + 1];
	strcpy(in,serial);
	for (int i = 0; i < (int)strlen(serial); i++) {
		if ((in[i] < 48 || in[i] > 57) && (in[i] < 97 || in[i] > 102)) {
			delete[] in;
			return false;
		}
	}
	delete[] in;
	return true;
}
int Check_Reserved_Slot(const char *criteria) {
	for (int totry = 1; totry <= 64; totry++) {
		char entry[32];
		sprintf(entry,"Nick%d",totry);
		char result[512];
		if (getProfileString("ReservedSlot",(const char *)entry,"",result,512,"vgm_users.ini")) {
			if (wildcmp(result,criteria)) {
				return 1;
			}
		} else { break; }
	}
	return 0;
}
int Check_Moderator_IP(const char *player, const char *ip) {
	char result[512];
	if (getProfileString("ModIPs",player,"",result,512,"vgm_users.ini")) {
		if (wildcmp(result,ip)) { return 1; }
		else { return 2; }
	}
	return 0;
}
int Check_IPR_Bans(const char *criteria) {
	for (int totry = 1; totry <= 64; totry++) {
		char entry[32];
		sprintf(entry,"Ban%d",totry);
		char result[512];
		if (getProfileString("IPRBans",(const char *)entry,"",result,512,"vgm_bans.ini")) {
			if (wildcmp(result,criteria)) {
				return 1;
			}
		} else { break; }
	}
	return 0;
}
void Give_Money_To_All_Players(int amount, int team) {
	Give_Money_To_All_Players((float)amount,team);
}
void Give_Money_To_All_Players(float amount, int team) {
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if ((int)(Node->NodeData->PlayerType) == team || team == 2) {
			Node->NodeData->Increment_Money(amount);
		}
	}
}
void Send_Private_Message_Team(int team, const char *message) {
	Send_Private_Message_Team_Except(team,message,-1);
}
void Send_Private_Message_Team_Except(int team, const char *message, int Exclude) {
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if (Node->NodeData->PlayerId == Exclude) { continue; }
		if ((int)(Node->NodeData->PlayerType) == team || team == 2) {
			HostPrivatePage(Node->NodeData->PlayerId,"%s",message);
		}
	}
}

// ---------------------------
//
//  vPlayer Class Functions
//
// ---------------------------
vPlayer::vPlayer(int PlayerID) {
	Set_ID(PlayerID);
	Stewie_cPlayer *cp = Stewie_cPlayerManager::Find_Player(ID);
	if (cp) { name = WCharToStr((&cp->PlayerName)->m_Buffer); }
	else { name = std::string(""); }
	serial = true;
	muted = false;
	qspec = false;
	CheatPending = 0;
	SerialTimeout = -1;
	SecondsIngame = 0;
	LastAirborne = 0;

	VetPoints = 0.0f;
	VetRank = 1;

	ChatMsgs = 0;
	CSpamExpire = 0;
	RadioMsgs = 0;
	RSpamExpire = 0;

	TotalValueRepaired = 0.0f;
	KillsWithoutDying = 0;
	SquishesWithoutDying = 0;
	BuildingsKilled = 0;
	HSWithoutDying500 = 0;
	HSWithoutDying1000 = 0;
	KillSpreeStart = 0;
	KillsSinceSpreeStart = 0;
	ProxyC4Kills = 0;

	LastVetCmds.clear();
	InitVCmds();

	for (unsigned int j = 0; j < (int)vMManager->vLAST; j++) { Medals[j] = 0; }
}
vPlayer::~vPlayer() {
	Left_Game();
}
void vPlayer::Join_Game(int PID) {
	ReInit();
	Set_ID(PID);
	serial = false;
	SerialTimeout = (vManager.DurationAsInt() + Config->SerialTimeoutDuration);
	ChatMsgs = 0;
	CSpamExpire = 0;
	FirstCSpam = 0;
	RadioMsgs = 0;
	RSpamExpire = 0;
	FirstRSpam = 0;
	bhsVersion = 0.0f;
	LastRecommend = -60;
	LastVehType = -1;
	Find_All_Seeable_Objects();
}
void vPlayer::Left_Game() {
	Set_ID(-1);
	if (vPManager->RequestedTC == ID) { vPManager->ResetRTC(); }
	SerialTimeout = -1;
	Clear_Seen_Objects();
	Clear_Hits();
	WeaponBag.clear();
	InitVCmds();
	ShouldKick = false;
}
void vPlayer::ReInit() {
	SerialTimeout = -1;
	Clear_Seen_Objects();
	Clear_Hits();
	WeaponBag.clear();
	InitVCmds();
	ShouldKick = false;
}
void vPlayer::Destroy() {
	Clear_Seen_Objects();
	Clear_Hits();
	LastVetCmds.clear();
	WeaponBag.clear();
	delete this;
}
void vPlayer::InitVCmds() {
	LastVetCmds.clear();
	int Offset = -1 * Config->VetCmdCooldown;
	LastVetCmds.push_back(vManager.DurationAsInt() + Offset);
	LastVetCmds.push_back(vManager.DurationAsInt() + Offset);
	LastVetCmds.push_back(vManager.DurationAsInt() + Offset);
}
void vPlayer::Add_Hit(int TargetID, int weapon) {
	if (this->Hits.size() > 1000) { return; }
	vHitEvent Event;
	Event.ShooterID = ID;
	Event.TargetID = TargetID;
	Event.weapon = weapon;
	Event.HitTime = clock();
	this->Hits.push_back(Event);
}
int vPlayer::Get_Hits(int TargetID, float secs, int WeaponID) {
	int hits = 0;
	int maxhits = int(secs) * 100;
	float CompareTime = float(clock()) - (secs * float(CLOCKS_PER_SEC));
	FOREACH (this->Hits,vHitEvent,HitIter) {
		if (hits >= maxhits) { return hits; }
		vHitEvent& EvtIter = (vHitEvent&)HitIter;
		if (CompareTime > float(EvtIter.HitTime)) {
			if (EvtIter.TargetID == TargetID && EvtIter.weapon == WeaponID) { hits++; }
		}
	}
	return hits;
}
void vPlayer::Clear_Hits() {
	this->Hits.clear();
}
void vPlayer::Clear_Hits_By_Target_ID(int TargetID) {
	for (unsigned int i = 0; i < this->Hits.size(); i++) {
		if (this->Hits[i].TargetID == TargetID) {
			this->Hits.erase(this->Hits.begin() + i);
			i--;
		}
	}
}
void vPlayer::Clear_Hits_By_Time(float Ago) {
	float CompareTime = float(clock()) - (Ago * float(CLOCKS_PER_SEC));
	for (unsigned int i = 0; i < this->Hits.size(); i++) {
		if (CompareTime > this->Hits[i].HitTime) {
			this->Hits.erase(this->Hits.begin() + i);
			i--;
		}
	}
}
void vPlayer::Add_Seeable_Object(int TargetObjID, bool CheckExisting) {
	if (!Config->LogSpectator) { return; }
	if (CheckExisting) {
		for (unsigned int i = 0; i < this->SeenList.size(); i++) {
			if (this->SeenList[i].ID == TargetObjID) { return; }
		}
	}
	vSeenObj Obj;
	Obj.ID = TargetObjID;
	Obj.SeenTime = -1;
	this->SeenList.push_back(Obj);
}
void vPlayer::Check_Seens() {
	if (!Config->LogSpectator) { return; }
	for (unsigned int i = 0; i < this->SeenList.size(); i++) {
		Stewie_BaseGameObj *Target = (Stewie_BaseGameObj *)Commands->Find_Object(this->SeenList[i].ID);
		if (Target && Target->As_PhysicalGameObj()) {
			Stewie_BaseGameObj* Player = (Stewie_BaseGameObj *)Player_GameObj(this->ID);
			if (!Player) { continue; }
			if (Player == Target || Player->NetworkID == Target->NetworkID) { continue; }
			if (Player->As_SmartGameObj() && Commands->Is_Object_Visible((GameObject *)Player,(GameObject *)Target)) {
				this->SeenList[i].SeenTime = (int)clock();
			}
		}
	}
}
void vPlayer::Find_All_Seeable_Objects() {
	Clear_Seen_Objects();
	if (!Config->LogSpectator) { return; }
	for (GenericSLNode *Node = SmartGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *obj = (GameObject *)Node->NodeData;
		if (!obj) { continue; }
		Add_Seeable_Object(Commands->Get_ID(obj),false);
	}
}
int vPlayer::Get_Seen_Time(int TargetObjID) {
	if (!Config->LogSpectator) { return -1; }
	for (unsigned int i = 0; i < this->SeenList.size(); i++) {
		if (this->SeenList[i].ID == TargetObjID) {
			return this->SeenList[i].SeenTime;
		}
	}
	return -1;
}
void vPlayer::Clear_Seen_Objects() {
	this->SeenList.clear();
}
void vPlayer::Increment_Ingame_Time() {
	if (!GameDataObj()->Is_Gameplay_Permitted()) { return; }
	SecondsIngame++;
}

void vPlayerManager::Init() {
	RequestedTC = -1;
	this->VersionTags.clear();
	this->Bounties.clear();
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		this->Data[i]->Destroy();
	}
	this->Data.clear();
	if (Stewie_cPlayerManager::Count() > 0) {
		for (int i = 0; i < 127; i++) {
			if (Stewie_cPlayerManager::Is_Player_Present(i)) {
				Add_Player(i);
				ConsoleIn("version %d",i);
			}
		}
	}
}
vPlayer* vPlayerManager::Add_Player(vPlayer* player) {
	Data.push_back(player);
	return player;
}
vPlayer* vPlayerManager::Add_Player(int ID) {
	vPlayer *p = new vPlayer(ID);
	return Add_Player(p);
}
void vPlayerManager::Think(bool FullSecond) {
	if (this->Data.empty()) { return; }
	if (FullSecond) {
		this->Check_BHS_Versions();
		if (Config->OutputDamageEvents) { this->Check_Cheat_Messages(); }
	}
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (!Stewie_cPlayerManager::Is_Player_Present(this->Data[i]->ID)) { continue; }
		if (Is_Soldier_Airborne(Player_GameObj(this->Data[i]->ID))) { this->Data[i]->LastAirborne = (int)clock(); }
		if (FullSecond) {
			if (this->Data[i]->ShouldKick) {
				Evict_Player(this->Data[i]->ID);
				continue;
			}
			this->Data[i]->Check_Seens();
			this->Data[i]->Increment_Ingame_Time();
			if (!this->Data[i]->serial && this->Data[i]->SerialTimeout > 0 && this->Data[i]->SerialTimeout <= vManager.DurationAsInt()) {
				if (Config->SerialTimeoutDuration != -1) {
					vLogger->Log(vLoggerType::vVGM,"_SERIAL","Player %s has been kicked for not sending a serial to the server.",this->Data[i]->Get_Name());
					this->Data[i]->ShouldKick = true;
				}
			}
			if (vManager.DurationAsInt() % 60 == 0) {
				this->Data[i]->Clear_Hits();
				this->Check_For_Veteran_Promotions(this->Data[i]);
			}
		}
	}
}
void vPlayerManager::Check_End_Game_Medals() {
	if (Config->MedalsEnabled == false) { return; }

	if (this->Data.empty()) { return; }
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }

		if (this->Data[i]->BuildingsKilled == 1) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL1,false); }
		else if (this->Data[i]->BuildingsKilled == 2) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL2,false); }
		else if (this->Data[i]->BuildingsKilled == 3) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL3,false); }
		else if (this->Data[i]->BuildingsKilled == 4) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL4,false); }
		else if (this->Data[i]->BuildingsKilled == 5) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL5,false); }
		else if (this->Data[i]->BuildingsKilled >= 6) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vBLDGKILL6,false); }

		Stewie_cPlayer *p = Stewie_cPlayerManager::Find_Player(this->Data[i]->ID);
		if (p) {
			if ((int)(p->Kills) >= 10 && (int)(p->Deaths) <= 0 && vManager.DurationAsInt() >= 600) { vMManager->Got_Medal(this->Data[i]->Get_Name(),vMManager->vPERFECTION,false); }
		}
		
		for (unsigned int j = 0; j < (int)vMManager->vLAST; j++) {
			char m[16],v[16],f[128];
			sprintf(m,"Medal%d",j);
			sprintf(f,"vgm\\medals_%s.ini",this->Data[i]->Get_Name());
			int Medals = getProfileInt("Medals",(const char*)m,0,(const char*)f);
			Medals += this->Data[i]->Medals[j];
			sprintf(v,"%d",Medals);
			writeProfileString("Medals",(const char*)m,(const char*)v,(const char*)f);
		}
	}
	Clear_All_Medals();
}
void vPlayerManager::Check_For_Veteran_Promotions(int ID) {
	Check_For_Veteran_Promotions(vPManager->Get_Player(ID));
}
void vPlayerManager::Check_For_Veteran_Promotions(vPlayer *p) {
	if (!p) { return; }
	int OkLevel = 0;
	for (int i = 0; i < Config->VetLevels; i++) {
		if (i >= (int)vVetManager->Promo_TimeIngameReqd.size()) { break; }
		if (p->SecondsIngame >= vVetManager->Promo_TimeIngameReqd[i]
			&& p->VetPoints >= vVetManager->Promo_VetPtsReqd[i]
			&& Get_Score(p->ID) >= vVetManager->Promo_ScoreReqd[i]) { OkLevel = i+1; }
	}
	if (OkLevel > p->VetRank) {
		p->VetRank = OkLevel;
		if (OkLevel > 1) {
			char entry[64],status[128];
			sprintf(entry,"Level%d",p->VetRank);
			getProfileString("VeteranPresets",entry,"NULL",status,128,"vgm_presets.ini");
			int random = Random(1,5);
			if (random == 1)
				HostMessage("[VGM] Give a round of applause for %s, promoted to %s!",p->Get_Name(),status);
			else if (random == 2)
				HostMessage("[VGM] W00t! %s has joined the Veteran team, promoted to %s!",p->Get_Name(),status);
			else if (random == 3)
				HostMessage("[VGM] %s received a Veteran rank of %s!",p->Get_Name(),status);
			else if (random == 4)
				HostMessage("[VGM] n00bs be afraid, %s was just promoted to %s!",p->Get_Name(),status);
			else if (random == 5)
				HostMessage("[VGM] %s %s is now reporting for duty!",status,p->Get_Name());

			GameObject *obj = Player_GameObj(p->ID);
			
			vVetManager->Upgrade(obj,p->VetRank - 1,1);
			vVetManager->Upgrade(Get_Vehicle(obj),p->VetRank - 1,2);

			if (p->VetRank >= Config->VetLevels) {
				Create_2D_WAV_Sound_Player(obj,"00-n048e.wav");
				ConsoleIn("icon %d p_keycrd_red.w3d",p->ID);
				Commands->Give_Money(obj,1000.0f,false);
			} else if (p->VetRank == Config->VetLevels - 1) {
				Create_2D_WAV_Sound_Player(obj,"00-n050e.wav");
				ConsoleIn("icon %d p_keycrd_yel.w3d",p->ID);
				Commands->Give_Money(obj,500.0f,false);
			} else if (p->VetRank == Config->VetLevels - 2) {
				Create_2D_WAV_Sound_Player(obj,"00-n052e.wav");
				ConsoleIn("icon %d p_keycrd_grn.w3d",p->ID);
				Commands->Give_Money(obj,250.0f,false);
			} else if (p->VetRank <= Config->VetLevels - 3) {
				Create_2D_WAV_Sound_Player(obj,"bonus_complete.wav");
			}
		}
	}
}
void vPlayerManager::Add_Cheat_Message(vCheatMessage& Data) {
	int Deleted = Del_Cheat_Message(Data.ID,Data.Type);
	Data.DetectionsTriggered += Deleted;
	this->CheatMessages.push_back(Data);
}
int vPlayerManager::Del_Cheat_Message(int ID, int Type) {
	if (this->CheatMessages.empty()) { return 0; }
	int x = 0;
	for (unsigned int i = 0; i < this->CheatMessages.size(); i++) {
		if (this->CheatMessages[i].ID == ID && this->CheatMessages[i].Type == Type) {
			x += this->CheatMessages[i].DetectionsTriggered;
			this->CheatMessages.erase(this->CheatMessages.begin() + i);
			i--;
		}
	}
	return x;
}
void vPlayerManager::Check_Cheat_Messages() {
	if (this->CheatMessages.empty()) { return; }
	for (unsigned int i = 0; i < this->CheatMessages.size(); i++) {
		int ID = this->CheatMessages[i].ID;
		if (!Get_GameObj(ID) || Commands->Get_Health(Player_GameObj(ID)) <= 0.0f || this->CheatMessages[i].Time <= vManager.DurationAsUint()) {
			std::string pname = Player_Name_From_ID(ID);
			Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(ID);
			if (ID <= 0 || !player) { return; }
			Stewie_DefinitionClass *Def = Stewie_DefinitionMgrClass::Find_Definition((unsigned long)(this->CheatMessages[i].WeaponID));
			if (!Def) { return; }
			int ping = player->Ping;
			int Detections = this->CheatMessages[i].DetectionsTriggered;
			if (this->CheatMessages[i].Type == vCheatMsgType::vDAMAGE) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT","Invalid Weapon Damage detected from %s. [Damage: %.2f; Expected: %.2f; Weapon: %s; Ping: %d; DT: %d]",
						pname.c_str(),
						this->CheatMessages[i].Damage,
						this->CheatMessages[i].ExpectedDamage,
						Get_Pretty_Name(Def->Get_Name()).c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vRANGE) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT","Invalid Firing Distance detected from %s. [Distance: %.2f; Expected: %.2f; Weapon: %s; Ping: %d; DT: %d]",
						pname.c_str(),
						this->CheatMessages[i].Distance,
						this->CheatMessages[i].ExpectedDistance,
						Get_Pretty_Name(Def->Get_Name()).c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vROF) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT",
						"Invalid Rate of Fire detected from %s. [Fired: %d; Expected: %d; MeasureTime: %.2f; Weapon: %s; Ping: %d; DT: %d]",
						pname.c_str(),
						this->CheatMessages[i].Hits,
						this->CheatMessages[i].ExpectedHits,
						this->CheatMessages[i].HitMeasureTime,
						Get_Pretty_Name(Def->Get_Name()).c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vWARHEAD) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT",
						"Invalid Weapon Warhead detected from %s. [Warhead: %d; Expected: %d; Weapon: %s; Ping: %d; DT: %d]",
						pname.c_str(),
						this->CheatMessages[i].Warhead,
						this->CheatMessages[i].ExpectedWarhead,
						Get_Pretty_Name(Def->Get_Name()).c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vBIGHEAD) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT",
						"Player %s is using bighead. [Ping: %d; DT: %d]",
						pname.c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vSCOPE) {
				if (Config->OutputDamageEvents)
					vLogger->Log(vLoggerType::vVGM,"_CHEAT",
						"Player %s used scope on nonscopable weapon. [Weapon: %s; Ping: %d; DT: %d]",
						pname.c_str(),
						Get_Pretty_Name(Def->Get_Name()).c_str(),
						ping,Detections
					);
			} else if (this->CheatMessages[i].Type == vCheatMsgType::vSPECTATOR) {
				if (Config->OutputDamageEvents && this->CheatMessages[i].TargetObjDef->Get_Class_ID() == Soldier) {
					if (this->CheatMessages[i].NeverSeen) {
						vLogger->Log(vLoggerType::vVGM,"_CHEAT",
							"Player %s damaged %s which was never seen. [Weapon: %s; Ping: %d; DT: %d]",
							pname.c_str(),
							Get_Pretty_Name(this->CheatMessages[i].TargetObjDef->Get_Name()).c_str(),
							Get_Pretty_Name(Def->Get_Name()).c_str(),
							ping,Detections
						);
					} else {
						vLogger->Log(vLoggerType::vVGM,"_CHEAT",
							"Player %s damaged %s which was last seen %.1fs ago. [Weapon: %s; Ping: %d; DT: %d]",
							pname.c_str(),
							Get_Pretty_Name(this->CheatMessages[i].TargetObjDef->Get_Name()).c_str(),
							this->CheatMessages[i].TimeSeenAgo,
							Get_Pretty_Name(Def->Get_Name()).c_str(),
							ping,Detections
						);
					}
				}
			}
			if (this->CheatMessages[i].EvictAfterMessage && Config->Autokick) {
				vPlayer *p = vPManager->Get_Player(ID);
				p->ShouldKick = false;
			}
			this->CheatMessages.erase(this->CheatMessages.begin() + i);
			i--;
		}
	}
}
int vPlayerManager::Get_Current_Recs(int ID) {
	vPlayer *t = Get_Player(ID);
	if (ID <= 0 || !t) { return false; }
	char f[128];
	sprintf(f,"vgm\\medals_%s.ini",t->Get_Name());
	return getProfileInt("Medals","Recs",0,(const char*)f);
}
bool vPlayerManager::Recommend(int pID, int tID, int recs, vRecType Type) {
	vPlayer *p = Get_Player(pID);
	if (pID >= 1 && p && p->LastRecommend > (int)clock() - 15 * CLOCKS_PER_SEC) { return false; }
	vPlayer *t = Get_Player(tID);
	if (tID <= 0 || !t) { return false; }

	if (pID > 0 && tID > 0) {
		for (unsigned int i = 0; i < Recommendations.size(); i++) {
			if (Recommendations[i].pID == pID &&
				Recommendations[i].tID == tID &&
				Recommendations[i].value == recs &&
				Recommendations[i].time >= (int)clock() - 86400 * CLOCKS_PER_SEC)
					return false;
			if (Recommendations[i].time > (int)clock() - 86400 * CLOCKS_PER_SEC) {
				Recommendations.erase(Recommendations.begin() + i);
				i--;
			}
		}

		vRecTrail Rec;
		Rec.pID = pID;
		Rec.tID = tID;
		Rec.value = recs;
		Rec.time = (int)clock();
		Recommendations.push_back(Rec);
	}

	char v[16],f[128];
	sprintf(f,"vgm\\medals_%s.ini",t->Get_Name());

	int Recs = getProfileInt("Medals","Recs",0,(const char*)f) + recs;
	sprintf(v,"%d",Recs);
	writeProfileString("Medals","Recs",(const char*)v,(const char*)f);

	const char *Entry = "";
	if (Type == vRECMISC) { Entry = "MiscRecs"; }
	else if (Type == vRECMVP) { Entry = "MVPRecs"; }
	else if (Type == vRECKILL) { Entry = "KillRecs"; }
	else if (Type == vRECKD) { Entry = "KDRecs"; }
	else { return false; }

	Recs = getProfileInt("Medals",Entry,0,(const char*)f) + recs;
	sprintf(v,"%d",Recs);
	writeProfileString("Medals",Entry,(const char*)v,(const char*)f);
	return true;
}
void vPlayerManager::Check_All_Repair_Recommendations() {
	if (this->Data.empty()) { return; }
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (this->Data[i]->TotalValueRepaired >= 2000.0f) {
			this->Data[i]->TotalValueRepaired -= 2000.0f;
			Recommend(-1,this->Data[i]->ID,1,vRECMISC);
			PrivMsgColoredVA(this->Data[i]->ID,2,0,200,0,"[VGM] You have been recommended for overall repairing.");
		}
	}
}
void vPlayerManager::RTC(int ID) {
	GameObject *r = Player_GameObj(ID);
	int rteam = Commands->Get_Player_Type(r);
	if (ID == RequestedTC) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have revoked your Team Change request.");
		ResetRTC();
		return;
	}
	if (rteam != 0 && rteam != 1) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You are not on a team that allows team changing.");
		return;
	}
	if (Get_Player_Count() <= 2) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] There are not enough people in game to change team.");
		return;
	}
	if (RequestedTC <= 0 || !Stewie_cPlayerManager::Is_Player_Present(RequestedTC)) {
		RequestedTC = ID;
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have requested to switch teams to %s.",Get_Translated_Team_Name(1 - rteam).c_str());
		HostMessage("[VGM] %s has requested a team change to %s! If you are on that team and want to switch with them, type !rtc.",Player_Name_From_ID(ID).c_str(),Get_Translated_Team_Name(1 - rteam).c_str());
	} else {
		GameObject *o = Player_GameObj(RequestedTC);
		if (Commands->Get_Player_Type(o) == rteam) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Someone on your team has already requested a team change. Please wait until they revoke their team change request or they switch teams.");
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have switched teams with %s.",Player_Name_From_ID(RequestedTC).c_str());
			PrivMsgColoredVA(RequestedTC,2,0,200,0,"[VGM] You have switched teams with %s.",Player_Name_From_ID(ID).c_str());
			RequestedTC = -1;
			Change_Team(r,Commands->Get_Player_Type(o));
			Change_Team(o,rteam);
		}
	}
}

void vVeteranManager::Add_Pending_VetPoints(Stewie_ScriptableGameObj *shooter, int victimID, float damage, float Max, bool IgnoreTeams) {
	if (!Config->VetEnabled || damage == 0.0f) { return; }
	if (GameDataObj()->Is_Game_Over()) { return; }

	if (shooter && shooter->As_VehicleGameObj()) {
		Stewie_SoldierGameObj *Driver = Vehicle_Driver(shooter->As_VehicleGameObj());
		if (Driver) { shooter = Driver; }
	}
	
	Stewie_BaseGameObj *victim = vObjects::Get_Object(victimID);
	if (!victim) { return; }
	Stewie_DamageableGameObj *VictimObj = NULL;
	Stewie_ScriptableGameObj *VictimAsScriptable = victim->As_ScriptableGameObj();
	if (VictimAsScriptable) {
		VictimObj = VictimAsScriptable->As_DamageableGameObj();
	} else if (victim->As_PhysicalGameObj()) {
		VictimObj = victim->As_PhysicalGameObj()->As_DamageableGameObj();
	}
	if (!VictimObj) { return; }
	Stewie_DamageableGameObj *ShooterObj = (shooter ? shooter->As_DamageableGameObj() : NULL);

	float dmg = damage;
	int vteam = (VictimObj ? VictimObj->PlayerType : 2), steam = (ShooterObj ? ShooterObj->PlayerType : (1 - vteam));
	if (steam != 0 && steam != 1) { shooter = NULL; }
	else if (vteam != 0 && vteam != 1) { shooter = NULL; }
	if (vteam == steam) {
		if (damage >= 0.0f && IgnoreTeams == false) { shooter = NULL; }
	}

	std::string shootername = Player_Name_From_GameObj(shooter);
	for (unsigned int i = 0; i < Pending_VetPoints.size(); i++) {
		if (this->Pending_VetPoints[i].StorageObjID != victimID) { continue; }
		if (!stricmp(shootername.c_str(),this->Pending_VetPoints[i].Shooter.c_str())) {
			this->Pending_VetPoints[i].Damage += dmg;
			if (Max > 0.0f && this->Pending_VetPoints[i].Damage > Max) { this->Pending_VetPoints[i].Damage = Max; }
			return;
		}
	}

	if (Max > 0.0f && dmg > Max) { dmg = Max; }
	vVetPointStruct Points;
	Points.Shooter = shootername;
	Points.StorageObjID = victimID;
	Points.Damage = dmg;
	Pending_VetPoints.push_back(Points);
}
void vVeteranManager::Object_Destroyed(int GameObjID) {
	if (!Config->VetEnabled) { return; }
	GameObject *o = Commands->Find_Object(GameObjID);
	//float multiplier = Get_Damage_Multiplier(o);
	//float rmultiplier = Get_Repair_Multiplier(o);
	float ptsSplit = Get_VetPts_For_Kill(o);
	float totalpts = 0.0f;
	for (unsigned int i = 0; i < Pending_VetPoints.size(); i++) {
		if (this->Pending_VetPoints[i].StorageObjID <= 0) { continue; }
		if (this->Pending_VetPoints[i].StorageObjID == GameObjID) {
			if (this->Pending_VetPoints[i].Damage <= 0.0f) { continue; }
			totalpts += this->Pending_VetPoints[i].Damage;
		}
	}
	for (unsigned int i = 0; i < Pending_VetPoints.size(); i++) {
		if (this->Pending_VetPoints[i].StorageObjID <= 0) { continue; }
		if (this->Pending_VetPoints[i].StorageObjID == GameObjID) {
			float pts = this->Pending_VetPoints[i].Damage;
			if (pts <= 0.0f) { continue; }
			const char *name = this->Pending_VetPoints[i].Shooter.c_str();
			vPlayer *v = vPManager->Get_Player(name);
			if (v && v->ID > 0 && stricmp(name,"None")) {
				float prcnt = pts / totalpts;
				float ptsGranted = ptsSplit * prcnt;
				if (prcnt >= 1.0f && ptsGranted >= 0.1f && this->Pending_VetPoints[i].Damage > 0.0f) {
					if (Config->Sounds) { Commands->Create_3D_WAV_Sound_At_Bone("promote.wav",Player_GameObj(v->ID),"C HEAD"); }
					if (Is_Building(o)) { PrivMsgColoredVA(v->ID,2,255,119,51,"[VGM] You destroyed %.0f%% of the %s and have received %.2f veteran points.",prcnt * 100,Get_Pretty_Name(o).c_str(),ptsGranted); }
					else if (Is_Soldier(o)) { PrivMsgColoredVA(v->ID,2,255,119,51,"[VGM] You %s a %s and have received %.2f veteran points.",(prcnt >= 0.5f ? "killed" : "helped to kill"),Get_Pretty_Name(o).c_str(),ptsGranted); }
					else if (Is_C4(o)) { PrivMsgColoredVA(v->ID,2,255,119,51,"[VGM] You disarmed %.0f%% of a %s C4 and have received %.2f veteran points.",prcnt * 100,Get_Translated_C4_Mode(Get_C4_Mode(o)).c_str(),ptsGranted); }
					else if (Is_Beacon(o)) { PrivMsgColoredVA(v->ID,2,255,119,51,"[VGM] You disarmed %.0f%% of a(n) %s and have received %.2f veteran points.",prcnt * 100,Get_Pretty_Name(o).c_str(),ptsGranted); }
					else { PrivMsgColoredVA(v->ID,2,255,119,51,"[VGM] You destroyed %.0f%% of a %s and have received %.2f veteran points.",prcnt * 100,Get_Pretty_Name(o).c_str(),ptsGranted); }
				}
				v->Increment_VetPoints(ptsGranted);
				vPManager->Check_For_Veteran_Promotions(v->ID);
			}
			this->Pending_VetPoints.erase(this->Pending_VetPoints.begin() + i);
			i--;
		}
	}
}
void vVeteranManager::Upgrade(GameObject *o, int Rank, int Type) {
	if (!o || Rank < 0 || Rank >= Config->VetLevels || Rank >= (int)vVetManager->Promo_ArmorBonusGained.size()) { return; }
	float bonus = vVetManager->Promo_ArmorBonusGained[Rank];
	int AutoRepSecs = vVetManager->Promo_AutoHealSeconds[Rank];
	if (AutoRepSecs > 0) {
		char spn[16];
		sprintf(spn,"1,%d",AutoRepSecs);
		Remove_Script(o,"VetSelfRepair");
		Attach_Script_Once(o,"VetSelfRepair",spn);
	}
	if (Type == 1) { // Soldier
		bool IsGod = false;
		if (Is_Script_Attached(o,"IAmGod")) { IsGod = true; }
		Stewie_SoldierGameObjDef *sdef = Get_Soldier_Definition(o);
		if (sdef) {
			float DeficientHealth = Commands->Get_Max_Health(o) - Commands->Get_Health(o);
			float DeficientShield = Commands->Get_Max_Shield_Strength(o) - Commands->Get_Shield_Strength(o);
			Set_Max_Health(o,(IsGod ? 500.0f : sdef->HealthMax.Get()) + ((IsGod ? 500.0f : sdef->HealthMax.Get()) * (bonus / 100)));
			Set_Max_Shield_Strength(o,(IsGod ? 500.0f : sdef->ShieldStrengthMax.Get()) + ((IsGod ? 500.0f : sdef->ShieldStrengthMax.Get()) * (bonus / 100)));
			Commands->Set_Health(o,Commands->Get_Max_Health(o) - DeficientHealth);
			Commands->Set_Shield_Strength(o,Commands->Get_Max_Shield_Strength(o) - DeficientShield);
		}
		if (Rank >= Config->VetLevels - 1 && !Is_Stealth(o)) {
			if (Commands->Get_Player_Type(o) == 1) { Commands->Give_Powerup(o,"POW_PersonalIonCannon_Player",true); }
			else if (Commands->Get_Player_Type(o) == 0) { Commands->Give_Powerup(o,"POW_Railgun_Player",true); }
		}
		if (Rank >= Config->VetLevels - 2) {
			if (Commands->Get_Player_Type(o) == 1) { Commands->Give_Powerup(o,"POW_Chaingun_Player",true); }
			else if (Commands->Get_Player_Type(o) == 0) { Commands->Give_Powerup(o,"POW_Chaingun_Player_Nod",true); }
		}
		if (Rank >= Config->VetLevels - 3) {
			Commands->Give_Powerup(o,"POW_TiberiumAutoRifle_Player",true);
		}
		if (Rank >= Config->VetLevels - 4 && !Has_Weapon(o,"Weapon_RepairGun_Player") && !Has_Weapon(o,"CnC_Weapon_RepairGun_Player_Special")) {
			Commands->Give_Powerup(o,"POW_RepairGun_Player",true);
		}
	} else if (Type == 2) { // Vehicle
		Stewie_VehicleGameObjDef *vdef = Get_Vehicle_Definition(o);
		if (vdef) {
			float DeficientHealth = Commands->Get_Max_Health(o) - Commands->Get_Health(o);
			float DeficientShield = Commands->Get_Max_Shield_Strength(o) - Commands->Get_Shield_Strength(o);
			Set_Max_Health(o,vdef->HealthMax.Get() + (vdef->HealthMax.Get() * (bonus / 100)));
			Set_Max_Shield_Strength(o,vdef->ShieldStrengthMax.Get() + (vdef->ShieldStrengthMax.Get() * (bonus / 100)));
			Commands->Set_Health(o,Commands->Get_Max_Health(o) - DeficientHealth);
			Commands->Set_Shield_Strength(o,Commands->Get_Max_Shield_Strength(o) - DeficientShield);
		}
	}
}
float vVeteranManager::Get_VetPts_For_Kill(GameObject *obj) {
	if (!obj) { return 0.0f; }
	else if (Is_Building(obj)) { return 50.0f; }
	else if (Is_Harvester(obj)) { return 5.0f; }
	else if (Is_Vehicle(obj) && Is_Script_Attached(obj,"M00_Disable_Transition")) {
		const char *preset = Get_Preset_Name(obj);
		if (isin(preset,"_site")) { return 5.0f; }
		else if (isin(preset,"turret") || isin(preset,"tower")) { return 10.0f; }
		float maxhealth = Commands->Get_Max_Health(obj) + Commands->Get_Max_Shield_Strength(obj);
		return maxhealth / 10.0f;
	} else if (Is_Vehicle(obj)) {
		const char *preset = Get_Preset_Name(obj);
		if (Is_VTOL_Vehicle(obj)) { return 11.0f; } // Flying (Medium)
		else if (isin(preset,"mrls")) { return 10.0f; } // MRLS (Light)
		else if (isin(preset,"artillery")) { return 10.0f; } // Artillery (Light)
		else if (isin(preset,"light")) { return 0.004f; } // Light Tank (Medium)
		else if (isin(preset,"apc")) { return 9.0f; } // APC (Medium)
		else if (isin(preset,"medium")) { return 12.0f; } // Medium Tank (Heavy)
		else if (isin(preset,"mammoth")) { return 16.0f; } // Mammoth Tank (Heavy)
		else if (isin(preset,"flame")) { return 15.0f; } // Flame Tank (Heavy)
		else if (isin(preset,"stealth")) { return 14.0f; } // Stealth Tank (Heavy)
		return 5.0f;
	} else if (Is_Soldier(obj)) {
		float maxhealth = Commands->Get_Max_Health(obj) + Commands->Get_Max_Shield_Strength(obj);
		float Base = 1.0f;
		float Addition = 1.0f;
		if (maxhealth >= 350.0f) { Addition = 3.0f; } // Advanced
		else if (maxhealth <= 200.0f) { Addition = 0.0f; } // Free
		vPlayer *p = vPManager->Get_Player(Get_Player_ID(obj));
		if (p) { return (float(p->VetRank) * 0.5f - 0.25f) + Addition; }
		return (0.25f) + Addition;
	} else if (Is_C4(obj)) {
		if (Get_C4_Mode(obj) == 2) { return 1.5f; } // Timed
		return 1.0f; // Other
	} else if (Is_Beacon(obj)) { return 3.0f; }
	return 0.0f;
}
float vVeteranManager::Get_Damage_Multiplier(GameObject *obj) {
	if (!obj) { return 0.0f; }
	else if (Is_Building(obj)) { return 0.1f; }
	else if (Is_Harvester(obj)) { return 0.001f; }
	else if (Is_Vehicle(obj) && Is_Script_Attached(obj,"M00_Disable_Transition")) {
		const char *preset = Get_Preset_Name(obj);
		if (isin(preset,"_site")) { return 0.05f; }
		else if (isin(preset,"turret") || isin(preset,"tower")) { return 0.035f; }
		return 0.005f;
	} else if (Is_Vehicle(obj)) {
		const char *preset = Get_Preset_Name(obj);
		if (Is_VTOL_Vehicle(obj)) { return 0.004f; } // Flying (Medium)
		else if (isin(preset,"light")) { return 0.004f; } // Light Tank (Medium)
		else if (isin(preset,"apc")) { return 0.004f; } // APC (Medium)
		else if (isin(preset,"medium")) { return 0.005f; } // Medium Tank (Heavy)
		else if (isin(preset,"mammoth")) { return 0.005f; } // Mammoth Tank (Heavy)
		else if (isin(preset,"flame")) { return 0.005f; } // Flame Tank (Heavy)
		else if (isin(preset,"stealth")) { return 0.005f; } // Stealth Tank (Heavy)
		return 0.0025f;
	} else if (Is_Soldier(obj)) {
		float maxhealth = Commands->Get_Max_Health(obj) + Commands->Get_Max_Shield_Strength(obj);
		float Addition = 0.25f;
		if (maxhealth >= 350.0f) { Addition = 0.5f; } // Advanced
		else if (maxhealth <= 200.0f) { Addition = 0.0f; } // Free
		vPlayer *p = vPManager->Get_Player(Get_Player_ID(obj));
		if (p) { return ((float(p->VetRank) * 0.5f - 0.25f) + Addition) / maxhealth; }
		return ((0.25f) + Addition) / maxhealth;
	} else if (Is_C4(obj)) {
		if (Get_C4_Mode(obj) == 2) { return 0.015f; } // Timed
		return 0.01f; // Other
	} else if (Is_Beacon(obj)) { return 0.0075f; }
	return 0.0f;
}
float vVeteranManager::Get_Repair_Multiplier(GameObject *obj) {
	return Get_Damage_Multiplier(obj) / 3.0f;
}
