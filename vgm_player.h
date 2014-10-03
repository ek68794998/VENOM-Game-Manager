#pragma once

#include "vgm_engine.h"
#include "vgm_game.h"
#include "vgm_commander.h"
#include "vgm_medals.h"
#include "vgm_obj.h"
#include "vgm_string.h"
#include "vgm_vehicle.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vPlayer;
class vPlayerManager;
class vVeteranManager;
struct vHitEvent;

extern vPlayerManager *vPManager;
extern vVeteranManager *vVetManager;

void __stdcall PlayerJoinHook(Stewie_cBioEvent *Event);
void __stdcall ClientDisconnectHook(int PlayerID);
void __stdcall PlayerLeaveHook(int PlayerID);
void __stdcall TeamScoreSwitchFixHook(Stewie_cPlayer &Player);
void __stdcall PlayerTeamHook(Stewie_cPlayer *Player, int Team);
void __stdcall SerialRetrievedHook(Stewie_cGameSpyCsChallengeResponseEvent* data);
void __stdcall PlayerKillMsgHook(Stewie_cPlayerKill &e, int Killer, int Victim);

int Player_ID_From_GameObj(Stewie_BaseGameObj *obj);
int Player_ID_From_Name(const char *name);
std::string Player_Name_From_GameObj(GameObject *obj);
std::string Player_Name_From_GameObj(Stewie_BaseGameObj *obj);
std::string Player_Name_From_ID(int ID);
GameObject *Player_GameObj_By_Name(const char *name);
GameObject *Player_GameObj_By_Part_Name(const char *name);
GameObject *Player_GameObj(int ID);
void Evict_Player(int ID);
void New_Chat_Message(int sender, int receiver, const char *string, bool popup, int type);
void PrivMsgColoredVA(int receiver, int team, unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...);
void PrivColoredVA(int receiver, int team, unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...);
void Send_Message_VA(unsigned int red, unsigned int green, unsigned int blue, const char *msg, ...);
int Get_KBPS(int ID);
float Get_KD_Ratio(Stewie_cPlayer *player);
void Set_Kills(Stewie_cPlayer *player, int Kills);
void Set_Deaths(Stewie_cPlayer *player, int Deaths);
void Inc_Kills(Stewie_cPlayer *player, int Kills);
void Inc_Deaths(Stewie_cPlayer *player, int Deaths);
int GetMaxPlayerID();
void Request_Serial(int ID);
bool Serial_Is_Valid(const char *serial);
int Check_Reserved_Slot(const char *criteria);
int Check_Moderator_IP(const char *player, const char *ip);
int Check_IPR_Bans(const char *criteria);
void Give_Money_To_All_Players(int amount, int team);
void Give_Money_To_All_Players(float amount, int team);
void Send_Private_Message_Team(int team, const char *message);
void Send_Private_Message_Team_Except(int team, const char *message, int Exclude);

enum vRecType {
	vRECNONE,
	vRECMISC,
	vRECMVP,
	vRECKILL,
	vRECKD
};

enum vPlayerDmgMode {
	vNONE,
	vFALL,
	vTIBERIUM,
	vC4,
	vBEACON,
	vSQUISH,
	vSOLDIER,
	vAI
};

enum vCheatMsgType {
	vNOMSG,
	vDAMAGE,
	vRANGE,
	vROF,
	vWARHEAD,
	vSPECTATOR,
	vBIGHEAD,
	vSCOPE
};

struct vBhsVersionTag {
	int PID;
	float Version;
};

struct vBounty {
	int pID;
	int tID;
	int time;
	float amt;
};

struct vRecTrail {
	int pID;
	int tID;
	int time;
	int value;
};

class vCheatMessage {
public:
	vCheatMessage() {
		ID = 0;
		Type = vCheatMsgType::vNOMSG;
		WeaponID = 0;
		Time = vManager.DurationAsInt() + 2;
		EvictAfterMessage = false;
		DetectionsTriggered = 1;

		Damage = 0.0f;
		ExpectedDamage = 0.0f;
		Warhead = 0;
		ExpectedWarhead = 0;
		Distance = 0.0f;
		ExpectedDistance = 0.0f;
		Hits = 0;
		HitMeasureTime = 0.0f;
		ExpectedHits = 0;
	};

	int ID;
	vCheatMsgType Type;
	int WeaponID;
	unsigned int Time;
	bool EvictAfterMessage;
	int DetectionsTriggered;

	float Damage;
	float ExpectedDamage;

	int Warhead;
	int ExpectedWarhead;

	float Distance;
	float ExpectedDistance;

	int Hits;
	float HitMeasureTime;
	int ExpectedHits;

	Stewie_BaseGameObjDef* TargetObjDef;
	float TimeSeenAgo;
	bool NeverSeen;
};

struct vSeenObj {
	int ID; // GameObj ID
	clock_t SeenTime;
};

struct vHitEvent {
	int ShooterID; // Player ID
	int TargetID; // GameObj ID

	int weapon;

	clock_t HitTime;
};

class vPlayer {
public:
	int ID;
	std::string name;
	bool serial;
	bool muted;
	bool qspec;
	float VetPoints;
	int VetRank;
	int SecondsIngame;
	int CheatPending;
	int SerialTimeout;
	bool ShouldKick;
	float bhsVersion;
	int LastRecommend;
	int LastVehType;
	int LastAirborne;

	int ChatMsgs;
	int FirstCSpam;
	int CSpamExpire;
	int RadioMsgs;
	int FirstRSpam;
	int RSpamExpire;

	float TotalValueRepaired;
	int KillsWithoutDying;
	int SquishesWithoutDying;
	int BuildingsKilled;
	int HSWithoutDying500;
	int HSWithoutDying1000;
	int KillSpreeStart;
	int KillsSinceSpreeStart;
	int ProxyC4Kills;
	int Medals[(int)(vMManager->vLAST)];

	int LastDmgSpecial;
	float LastFallDistance;
	GameObject *LastDamager;
	float LastDamage;
	int LastFiringMode;

	std::vector<int> LastVetCmds;
	std::vector<vHitEvent> Hits;
	std::vector<vSeenObj> SeenList;
	std::vector<Stewie_WeaponDefinitionClass *> WeaponBag;

	vPlayer(int PlayerID);
	~vPlayer();

	void Join_Game(int PID);
	void Left_Game();
	void ReInit();
	void Destroy();
	void InitVCmds();
	const char *Get_Name() { return name.c_str(); }

	void Set_ID(int i) { this->ID = i; }
	void Set_Serial(bool Set) { serial = Set; }
	void Set_Muted(bool Set) { muted = Set; }
	void Set_Version(float Ver) { bhsVersion = Ver; }

	void Add_Hit(int TargetID, int weapon);
	int Get_Hits(int TargetID, float secs, int weapon);
	void Clear_Hits();
	void Clear_Hits_By_Target_ID(int TargetID);
	void Clear_Hits_By_Time(float Ago);

	void Add_Seeable_Object(int TargetObjID, bool CheckExisting = true);
	void Check_Seens();
	void Find_All_Seeable_Objects();
	int Get_Seen_Time(int TargetObjID);
	void Clear_Seen_Objects();

	void Set_VetPoints(float Points) { VetPoints = Points; }
	void Increment_VetPoints(float Points) { VetPoints += Points; }
	void Increment_Ingame_Time();
	void Reset_KillSprees() {
		KillsWithoutDying = 0;
		SquishesWithoutDying = 0;
	}
};

class vPlayerManager {
public:
	std::vector<vPlayer *> Data;
	std::vector<vCheatMessage> CheatMessages;
	std::vector<vBhsVersionTag> VersionTags;
	std::vector<vBounty> Bounties;
	std::vector<vRecTrail> Recommendations;
	int RequestedTC;

public:
	vPlayerManager() {
		RequestedTC = -1;
	};
	void Init();
	vPlayer* Add_Player(vPlayer* player);
	vPlayer* Add_Player(int ID);
	vPlayer* Get_Player(int ID) {
		if (Stewie_cPlayerManager::Is_Player_Present(ID) == false) { return NULL; }
		if (this->Data.empty()) { return NULL; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			if (this->Data[i]->ID > 0 && this->Data[i]->ID == ID) { return this->Data[i]; }
		}
		return NULL;
	};
	vPlayer* Get_Player(const char *name) {
		if (this->Data.empty()) { return NULL; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			if (!stricmp(this->Data[i]->Get_Name(),name)) { return this->Data[i]; }
		}
		return NULL;
	};
	unsigned int Get_Player_Count() {
		return this->Data.size();
	};
	void Think(bool FullSecond);
	
	void Clear_Hits_By_Target_ID(int TargetID) {
		if (this->Data.empty()) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			this->Data[i]->Clear_Hits_By_Target_ID(TargetID);
		}
	}
	
	void Add_Seen_Object(int ID) {
		if (!Config->LogSpectator) { return; }
		if (this->Data.empty()) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			this->Data[i]->Add_Seeable_Object(ID);
		}
	}
	void Clear_Seen() {
		if (!Config->LogSpectator) { return; }
		if (this->Data.empty()) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			this->Data[i]->Clear_Seen_Objects();
		}
	}

	void Check_End_Game_Medals();
	void Clear_All_Medals() {
		if (this->Data.empty()) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			this->Data[i]->KillsWithoutDying = 0;
			this->Data[i]->BuildingsKilled = 0;
			for (unsigned int j = 0; j < (int)vMManager->vLAST; j++) {
				this->Data[i]->Medals[j] = 0;
			}
		}
	};

	void Check_For_Veteran_Promotions() {
		for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
			if (!Node->NodeData) { continue; }
			if (!Node->NodeData->IsActive || !Node->NodeData->IsInGame) { continue; }
			Check_For_Veteran_Promotions(Node->NodeData->PlayerId);
		}
	};
	void Check_For_Veteran_Promotions(int ID);
	void Check_For_Veteran_Promotions(vPlayer *p);
	void Reset_All_VetPoints() {
		if (this->Data.empty()) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			this->Data[i]->Set_VetPoints(0.0f);
		}
	};

	void Add_Cheat_Message(vCheatMessage& Data);
	int Del_Cheat_Message(int ID, int Type);
	void Check_Cheat_Messages();

	void Add_BHS_Version(vBhsVersionTag& Version) {
		this->VersionTags.push_back(Version);
	}
	void Check_BHS_Versions() {
		if (this->VersionTags.empty()) { return; }
		for (unsigned int i = 0; i < this->VersionTags.size(); i++) {
			int ID = this->VersionTags[i].PID;
			vPlayer *p = this->Get_Player(ID);
			if (p) {
				if (p->bhsVersion <= 1.0f) { p->Set_Version(this->VersionTags[i].Version); }
				this->VersionTags.erase(this->VersionTags.begin() + i);
				i--;
			}
		}
	}

	int Get_Current_Recs(int ID);
	bool Recommend(int pID, int tID, int Recommendations, vRecType Type = vRECMISC);
	void Check_All_Repair_Recommendations();
	void Check_For_Expired_Timed_Recs() {
	}

	bool Place_Bounty(int pID, int tID, float amount) {
		if (!Stewie_cPlayerManager::Is_Player_Present(pID)) { return false; }
		if (!Stewie_cPlayerManager::Is_Player_Present(tID)) { return false; }
		if (amount < 0.0f) { return false; }
		vBounty bounty;
		bounty.pID = pID;
		bounty.tID = tID;
		bounty.time = (int)clock();
		bounty.amt = amount;
		Bounties.push_back(bounty);
		return true;
	}
	void Remove_Bounty(int pID, int tID) {
		if (this->Bounties.empty()) { return; }
		for (unsigned int i = 0; i < this->Bounties.size(); i++) {
			if ((pID == -1 || Bounties[i].pID == pID) && Bounties[i].tID == tID) {
				this->Bounties.erase(this->Bounties.begin() + i);
				i--;
			}
		}
	}
	void Refund_Bounty(int tID) {
		if (this->Bounties.empty()) { return; }
		for (unsigned int i = 0; i < this->Bounties.size(); i++) {
			if (Bounties[i].tID == tID) {
				Commands->Give_Money(Player_GameObj(Bounties[i].pID),Bounties[i].amt,false);
				this->Bounties.erase(this->Bounties.begin() + i);
				i--;
			}
		}
	}
	float Get_Bounty(int pID, int tID) {
		if (this->Bounties.empty()) { return -1.0f; }
		float Amount = 0.0f;
		for (unsigned int i = 0; i < this->Bounties.size(); i++) {
			if ((pID == -1 || Bounties[i].pID == pID) && Bounties[i].tID == tID) { Amount += Bounties[i].amt; }
		}
		return Amount;
	}
	void Check_Bounties() {
		if (this->VersionTags.empty()) { return; }
		for (unsigned int i = 0; i < this->Bounties.size(); i++) {
			if (!Stewie_cPlayerManager::Is_Player_Present(Bounties[i].pID) || !Stewie_cPlayerManager::Is_Player_Present(Bounties[i].tID)) {
				this->Bounties.erase(this->Bounties.begin() + i);
				i--;
				continue;
			}
			if (Bounties[i].time < (int)clock() - 300 * CLOCKS_PER_SEC) {
				Commands->Give_Money(Player_GameObj(Bounties[i].pID),Bounties[i].amt,false);
				const char *pl = Player_Name_From_ID(Bounties[i].pID).c_str();
				const char *ta = Player_Name_From_ID(Bounties[i].tID).c_str();
				PrivMsgColoredVA(Bounties[i].pID,2,128,0,0,"[VGM] %s escaped the wrath of your bounty! You have been refunded.",ta);
				PrivMsgColoredVA(Bounties[i].pID,2,128,0,0,"[VGM] You escaped the wrath of %s's bounty!",pl);
				HostMessage("[VGM] The bounty placed on %s was not achieved in 5 minutes, so it has been refunded.",ta);
				this->Bounties.erase(this->Bounties.begin() + i);
				i--;
			}
		}
	}

	void RTC(int ID);
	void ResetRTC() {
		if (RequestedTC < 0) { RequestedTC = -1; return; }
		vPlayer *p = Get_Player(RequestedTC);
		if (p) { HostMessage("[VGM] %s has revoked his team change request. !rtc is free for use now.",p->Get_Name()); }
		else { HostMessage("[VGM] The existing team change request has been revoked. !rtc is free for use now."); }
		RequestedTC = -1;
	}
};

struct vVetPointStruct {
	std::string Shooter; // Player ID
	int StorageObjID; // GameObj ID
	float Damage;
};

class vVeteranManager {
public:
	std::vector<vVetPointStruct> Pending_VetPoints;
	std::vector<float> Promo_VetPtsReqd;
	std::vector<float> Promo_ScoreReqd;
	std::vector<int> Promo_TimeIngameReqd;
	std::vector<float> Promo_ArmorBonusGained;
	std::vector<int> Promo_AutoHealSeconds;

	void Init() {
		Clear_All_Pending_VetPoints();
		Clear_All_VetPoints();
	}
	void Clear_All_Pending_VetPoints() { Pending_VetPoints.clear(); };
	void Clear_All_VetPoints() { vPManager->Reset_All_VetPoints(); };
	void Add_Pending_VetPoints(Stewie_ScriptableGameObj *shooter, int victim, float damage, float Max = -1.0f, bool IgnoreTeams = false);
	void Object_Destroyed(int GameObjID);
	void Object_Destroyed(GameObject *o) { Object_Destroyed(Commands->Get_ID(o)); };
	void Upgrade(GameObject *o, int Rank, int Type);
	float Get_VetPts_For_Kill(GameObject *obj);
	float Get_Damage_Multiplier(GameObject *obj);
	float Get_Repair_Multiplier(GameObject *obj);
};
