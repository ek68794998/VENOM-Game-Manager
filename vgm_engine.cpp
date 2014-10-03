#include "vgm_presets.h"
#include "vgm_player.h"
#include "vgm_crate.h"
#include "vgm_crystal.h"
#include "vgm_cloning.h"
#include "vgm_hooks.h"
#include "vgm_actions.h"
#include "vgm_threads.h"
#include "vgm_engine.h"

vGameManager vManager;

bool ServerInit;
bool EmergencyShutdown = false;
bool FirstKillOfMap;
vConfig *Config;
Stewie_ScriptableGameObj *InvObj;
char vDate::out_string[25];
char vDate::last_date[25];

void vGameManager::Init() {
	ConsoleOut("VGM " SLV " - Written by Stewie");
	srand((unsigned)time(0));
#ifdef _DEV_
	ConsoleOut("[VGM] Warning: Running in Development environment.");
	//ConsoleOut("[VGM] Addr: %X",AddVersionHook);
	ConsoleOut("[VGM] Offset: %X, %X",
		offsetof(Stewie_GameObjObserverClass,ID),
		offsetof(Stewie_ScriptImpClass,ID)
	);
#endif
#ifdef DEBUG
	ConsoleOut("[VGM] WARNING! Running in Debug environment.");
#endif
}
void vGameManager::Shutdown() {
	ConsoleOut("[VGM] Venom General Manager is safely shutting down the Dedicated Server...");
	ConsoleOut("[VGM] Evicting players...");
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		Evict_Player(Node->NodeData->PlayerId);
	}
	ConsoleOut("[VGM] Player eviction completed.");
	ConsoleOut("Terminating game on demand...");
	ConsoleOut("Terminating slaves on demand...");
	__asm {
		mov ecx,[0x000000] // For security and licensing purposes, an address has been hidden from this line
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		add esp,0x8
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
void vGameManager::Think() {
	Stewie_cGameData *GameObj = GameDataObj();
	if (!GameObj || !Config) { return; }

	if (Config->EnableConnectionHook && Config->ResolveHostnames && Resolvers.empty() == false) {
		for (unsigned int i = 0; i < Resolvers.size(); i++) {
			if (Resolvers[i]->Resolved >= 0 || (int)(Resolvers[i]->HostConnection->timeout) <= (int)clock()) {
				if (Resolvers[i]->Resolved == 0) { Resolvers[i]->HostConnection->Resolved = true; }
				else { Resolvers[i]->HostConnection->Resolved = false; }
				Resolvers[i]->HostConnection->Host = (const char *)Resolvers[i]->host;
				Resolvers[i]->HostConnection->OnHostnameResolved();
				delete Resolvers[i];
				Resolvers.erase(Resolvers.begin() + i);
				i--;
			}
		}
	}

	if (!GameObj->Is_Gameplay_Permitted()) { return; }

	this->GameDuration += Stewie_TimeManager::FrameSeconds;

	if (this->DurationAsInt() < 2) { return; }

	this->QuarterSecTimer -= Stewie_TimeManager::FrameSeconds;
	this->FullSecTimer -= Stewie_TimeManager::FrameSeconds;
	this->ClearHitsTimer -= Stewie_TimeManager::FrameSeconds;

	bool IsQuarterSecond = false;
	bool IsFullSecond = false;
	bool ClearHitsNow = false;
	if (this->QuarterSecTimer <= 0.0f) {
		IsQuarterSecond = true;
		this->QuarterSecTimer += 0.25f;
	}
	if (this->FullSecTimer <= 0.0f) {
		IsFullSecond = true;
		this->FullSecTimer += 1.0f;
	}
	if (this->ClearHitsTimer <= 0.0f) {
		ClearHitsNow = true;
		this->ClearHitsTimer += Config->StoreHitDataSecs;
	}

	vWManager->Think(IsFullSecond);
	vPManager->Think(IsFullSecond);
	vCManager->Think(IsFullSecond);
	//vAManager->Think(IsFullSecond);

	if (ClearHitsNow) {
		unsigned int PCount = vPManager->Get_Player_Count();
		for (unsigned int i = 0; i < PCount; i++) {
			if (!vPManager->Data[i]) { continue; }
			vPManager->Data[i]->Clear_Hits_By_Time(Config->StoreHitDataSecs);
		}
	}

	if (IsQuarterSecond) {
#ifdef _DEV_
		for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
			if (!Node->NodeData) { continue; }
			if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
			Node->NodeData->Increment_Money(float(Random(25,35)));
			Force_Refill((Stewie_ArmedGameObj *)Player_GameObj(Node->NodeData->PlayerId));
		}
#else
		if (Config->GameMode == Config->vMONEY || Config->GameMode == Config->vSNIPER) {
			for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
				if (!Node->NodeData) { continue; }
				if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
				if (Config->GameMode == Config->vMONEY) { Node->NodeData->Increment_Money(float(Random(25,35))); } // Give_Money_To_All_Players Optimized
				else if (Config->GameMode == Config->vSNIPER) {
					Force_Refill((Stewie_ArmedGameObj *)Player_GameObj(Node->NodeData->PlayerId));
				}
			}
		}
#endif
	}

	if (IsFullSecond) {
		int HigherVehLimit = (Config->GDIVehicleLimit > Config->NodVehicleLimit ? Config->GDIVehicleLimit : Config->NodVehicleLimit);
		if (Stewie_VehicleFactoryGameObj::MaxVehiclesPerTeam != HigherVehLimit) { ConsoleIn("vlimit %d",HigherVehLimit); }
	}
}

void vGameManager::Level_Loaded() {
	this->GameDuration = 0.0f;
	this->QuarterSecTimer = 0.25f;
	this->FullSecTimer = 1.0f;

	if (ServerInit != true) {
		Config = new vConfig();
		DllInit();
		vPManager = new vPlayerManager();
		vVManager = new vVehicleManager();
		vMManager = new vMedalManager();
		vCManager = new vCrystalManager();
		vWManager = new vWeaponManager();
		vVetManager = new vVeteranManager();
		vAManager = new vAimbotManager();
		vLogger = new vLoggerClass();
		ServerInit = true;
		Config->Rehash(true,true,false);
	} else {
		Config->Rehash(false,true,false);
	}
	if (Config->MapSpecificSettings) { Change_Time_Remaining(float(Config->MapTimeLimit)); }

	vPManager->Init();
	vVManager->Init();
	vMManager->Init();
	vCManager->Init();
	vWManager->Init();
	vVetManager->Init();
	//vAManager->Init();

	vLogger->Log(vLoggerType::vCHEAT,"","Level loaded: %s",GameDataObj()->MapName);

	Config->CrazyDefensesEnabled = false;

	InvObj = Create_Object("Invisible_Object",Stewie_Vector3(0.0f,0.0f,-10.0f),false)->As_ScriptableGameObj();
	if (Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY || Config->GameMode == Config->vINFONLY) {
		Script_Attach(InvObj,"TeamCommander","");
		Script_Attach(InvObj,"OffensiveScript","");
	}

	InvObj->Start_Custom_Timer(InvObj,2.0f,800002,0);
	InvObj->Start_Custom_Timer(InvObj,2.0f,800002,1);
	GDITeamPool = 0;
	NODTeamPool = 0;
	Offensive = false;
	EMPon = false;
	GGstarted = false;
	FirstKillOfMap = true;

	Set_Team_Commander(0,0);
	Set_Team_Commander(0,1);

	Destroy_All_Objects_By_Preset(2,"Arc Effect");
	for (int i = 0; i <= 1; i++) {
		GameObject *bayobj = Find_Repair_Bay(i);
		if (bayobj) {
			Vector3 pos = Commands->Get_Position(bayobj);
			pos.X -= 7;
			Commands->Create_Object("Arc Effect",pos);
		}
	}
	if ((Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY) && Config->ExtraDefenses) {
		#pragma warning (disable: 4305)
		char *CurrMap = GameDataObj()->MapName;
		if (!stricmp(CurrMap,"C&C_City.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-60.195648,-162.687363,2.702361),false);
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(51.983162,-165.196426,2.702361),false);
		} else if (!stricmp(CurrMap,"C&C_City_Flying.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-60.195648,-162.687363,2.702361),false);
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(51.983162,-165.196426,2.702361),false);
			Set_Up_SAM_Site(1,Stewie_Vector3(-22.848297,-126.480598,-3.535721));
			Set_Up_SAM_Site(1,Stewie_Vector3(32.772552,-122.783974,-3.539377));
			Set_Up_SAM_Site(0,Stewie_Vector3(48.338646,135.964920,-3.614238));
			Set_Up_SAM_Site(0,Stewie_Vector3(-48.239510,119.209290,-2.877841));
		} else if (!stricmp(CurrMap,"C&C_Field.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(5.988967,-69.915672,6.318894),false);
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(59.431580,-81.019783,6.297704),false);
		} else if (!stricmp(CurrMap,"C&C_Glacier_Flying.mix")) {
			Set_Up_SAM_Site(1,Stewie_Vector3(55.167370,-126.127098,0.019367));
			Set_Up_SAM_Site(1,Stewie_Vector3(103.987091,-64.958893,0.026177));
			Set_Up_SAM_Site(1,Stewie_Vector3(58.276257,-49.636219,0.021730));
			Set_Up_SAM_Site(0,Stewie_Vector3(-83.336678,125.197327,0.019530));
			Set_Up_SAM_Site(0,Stewie_Vector3(-42.794830,42.043888,0.020238));
			Set_Up_SAM_Site(0,Stewie_Vector3(-95.323135,-29.397167,0.019307));
		} else if (!stricmp(CurrMap,"C&C_Hourglass.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-33.760921,-111.110405,2.694288),false);
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(33.760921,-111.110405,2.694288),false);
		} else if (!stricmp(CurrMap,"C&C_Mesa.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-83.932518,-78.839905,6.318978),false);
		} else if (!stricmp(CurrMap,"C&C_Under.mix")) {
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-104.803200,-87.187141,3.254541),false);
			Create_Object("GDI_Guard_Tower",Stewie_Vector3(-138.766357,-117.636131,3.183046),false);
		} else if (!stricmp(CurrMap,"C&C_Walls_Flying.mix")) {
			Set_Up_SAM_Site(1,Stewie_Vector3(-18.495171,-170.185471,-3.575219));
			Set_Up_SAM_Site(0,Stewie_Vector3(19.741053,151.481537,-2.995648));
		}
	}
}
void vGameManager::Level_Ended() {
	vCManager->Reset();
	vPManager->Check_End_Game_Medals();
	vPManager->Clear_Seen();

	int MVPID = 0;
	int MKillsID = 0;
	int BestKDID = 0;
	int MKILLS = 10;
	float BESTKD = 1.0f, MVPSCORE = 1500.0f;
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if (Node->NodeData->Kills.Get() >= MKILLS) { MKILLS = Node->NodeData->Kills.Get(); MKillsID = Node->NodeData->PlayerId; }
		if (Node->NodeData->Score.Get() >= MVPSCORE) { MVPSCORE = Node->NodeData->Score.Get(); MVPID = Node->NodeData->PlayerId; }
		if (Node->NodeData->Deaths.Get() <= 0) { continue; }
		if ((float(Node->NodeData->Kills.Get()) / float(Node->NodeData->Deaths.Get())) >= BESTKD) { BESTKD = float(Node->NodeData->Kills.Get()) / float(Node->NodeData->Deaths.Get()); BestKDID = Node->NodeData->PlayerId; }
	}
	vPManager->Recommend(-1,MVPID,1,vRECMVP);
	vPManager->Recommend(-1,MKillsID,1,vRECKILL);
	vPManager->Recommend(-1,BestKDID,1,vRECKD);

	Stewie_cGameData *data = GameDataObj();
	char *Type = "Unknown";
	if (data->WinType == 0) { Type = "Server Shutdown"; }
	else if (data->WinType == 2) { Type = "High Score when time limit expired"; }
	else if (data->WinType == 3) { Type = "Building Destruction"; }
	else if (data->WinType == 4) { Type = "Pedestal Beacon"; }
	float Gscore = Stewie_cTeamManager::Find_Team(1)->Score;
	float Nscore = Stewie_cTeamManager::Find_Team(0)->Score;
	if (Nscore > Gscore) {
    Console_Input("sndt 0 mxxdsgn_dsgn0049i1evan_snd.wav");
    Console_Input("sndt 1 mxxdsgn_dsgn0045i1evag_snd.wav");
		Console_Input("snda prodigy.wav");
	} else if (Gscore > Nscore) {
    Console_Input("sndt 0 mxxdsgn_dsgn0045i1evan_snd.wav");
    Console_Input("sndt 1 mxxdsgn_dsgn0051i1evag_snd.wav");
		Console_Input("snda endsound.mp3");
	} else {
    Console_Input("sndt 0 mxxdsgn_dsgn0049i1evan_snd.wav");
    Console_Input("sndt 1 mxxdsgn_dsgn0051i1evag_snd.wav");
		Console_Input("snda alternativejoinsound.mp3");
	}
	if (data->WinnerID == 0 || data->WinnerID == 1) {
		vLogger->Log(vLoggerType::vVGM,"_GENERAL","Game on %s has ended. %s won by %s after %s of gameplay.",data->MapName,Get_Translated_Team_Name(data->WinnerID).c_str(),Type,Duration(vManager.DurationAsInt()).c_str());
	} else if (Gscore != Nscore) {
		vLogger->Log(vLoggerType::vVGM,"_GENERAL","Game on %s has ended. %s won by %s after %s of gameplay.",data->MapName,Gscore > Nscore ? Get_Translated_Team_Name(1).c_str() : Get_Translated_Team_Name(0).c_str(),Type,Duration(vManager.DurationAsInt()).c_str());
	} else {
		vLogger->Log(vLoggerType::vVGM,"_GENERAL","Game on %s has ended. Game was terminated by %s after %s of gameplay.",data->MapName,Type,Duration(vManager.DurationAsInt()).c_str());
	}
}
void vGameManager::Game_Reset() {
	int PlayerCount = Get_Player_Count();
	if (PlayerCount == 0) { return; }

	if (GameDataObj()->RemixTeams) {
		Stewie_cPlayer** Players = new Stewie_cPlayer*[PlayerCount];
		memset(Players,0,sizeof(Stewie_cPlayer*)*PlayerCount);

		for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
			if (!Node->NodeData) { continue; }
			if (Node->NodeData->IsActive == false) { continue; }

			for (int i = 0; i < PlayerCount; i++) {
				if (!Players[i]) {
					// Slot is empty
					Players[i] = Node->NodeData;
					break;
				} else if ((float)(Players[i]->Score.Get()) < (float)(Node->NodeData->Score.Get())) {
					// Player's score is higher and should go here
					for (int j = PlayerCount - 1; j > i; j--) {
						Players[j] = Players[j - 1];
					}
					Players[i] = Node->NodeData;
					break;
				}
			}
		}

		int NewSidePlayers[2] = {0,0};
		float NewSideScore[2] = {0,0};
		int MaxTeamSize = PlayerCount - (int)(PlayerCount / 2);

		for (int i = 0; i < PlayerCount; i++) {
			if (!Players[i]) { continue; }

			int NewPlayerSide = Random(0,1); // Set to be random if we don't overwrite it
			if (NewSidePlayers[0] == MaxTeamSize) { NewPlayerSide = 1; } // Nod is full
			else if (NewSidePlayers[1] == MaxTeamSize) { NewPlayerSide = 0; } // GDI is full
			else if (NewSideScore[0] < NewSideScore[1]) { NewPlayerSide = 0; } // They're equal, but Nod is losing
			else if (NewSideScore[1] < NewSideScore[0]) { NewPlayerSide = 1; } // They're equal, but GDI is losing

			NewSidePlayers[NewPlayerSide]++;
			NewSideScore[NewPlayerSide] += (float)(Players[i]->Score.Get());

			Players[i]->Set_Player_Type(NewPlayerSide);
		}
		HostMessage("[VGM] Teams have been remixed and rebalanced.");

		delete[] Players;
	} else {
		if (Random(0,1) == 0) {
			for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
				int PlayerTeam = (int)(Node->NodeData->PlayerType.Get());
				if (PlayerTeam == 0 || PlayerTeam == 1) {
					Node->NodeData->Set_Player_Type(1 - PlayerTeam); // Should be cleaned up
				}
			}
			HostMessage("[VGM] Teams have been swapped.");
		}
	}
}

void ConsoleInNoVA(const char *in) {
	Stewie_ConsoleFunctionManager::Parse_Input(in);
}
void ConsoleIn(const char *in, ...) {
	char b[256];
	va_list args;
	va_start(args,in);
	vsnprintf(b,sizeof(b),in,args);
	va_end(args);
	ConsoleInNoVA(b);
}
void ConsoleOut(const char *out, ...) {
	char unformatted[256],b[256];
	sprintf(unformatted,"%s\n",out);
	va_list args;
	va_start(args,out);
	vsnprintf(b,sizeof(b),unformatted,args);
	va_end(args);
	The_Console.Print("%s",b);
}
void DebugMessage(const char *msg, ...) {
#ifdef _DEV_
	if (!msg) { return; }

	char b[256];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);

	vLogger->Log(vLoggerType::vVGM,"__DEBUG",b);

	HostMessage("[VGMD] %s",b);
#endif
}
void HostMessage(const char *msg, ...) {
	if (!msg) { return; }

	char b[256];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);

	ConsoleIn("msg %s",b);
	/*wchar_t wcs[256];
	swprintf(wcs,L"%S",b);
	Stewie_WideStringClass WideStr(wcs);
	Stewie_cScTextObj &newobj = Stewie_cScTextObj::Create();
	newobj.Init(WideStr,0,false,-1,-1);*/
}
void HostPrivateMessage(int receiver, const char *msg, ...) {
	if (!Stewie_cPlayerManager::Is_Player_Present(receiver) || !msg) { return; }

	char b[256];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);

	wchar_t wcs[256];
	swprintf(wcs,L"%S",b);
	Stewie_WideStringClass WideStr(wcs);
	Stewie_cScTextObj &newobj = Stewie_cScTextObj::Create();
	newobj.Init(WideStr,0,false,-1,receiver);
}
void HostPrivateAdminMessage(int receiver, const char *msg, ...) {
	if (!Stewie_cPlayerManager::Is_Player_Present(receiver) || !msg) { return; }
	
	char b[256];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);

	wchar_t wcs[256];
	swprintf(wcs,L"%S",b);
	Stewie_WideStringClass WideStr(wcs);
	Stewie_cScTextObj &newobj = Stewie_cScTextObj::Create();
	newobj.Init(WideStr,2,true,-1,receiver);
}
void HostPrivatePage(int receiver, const char *msg, ...) {
	if (!Stewie_cPlayerManager::Is_Player_Present(receiver) || !msg) { return; }

	char b[256];
	va_list args;
	va_start(args,msg);
	vsnprintf(b,sizeof(b),msg,args);
	va_end(args);

	HostPrivatePageNoVA(receiver,b);
}
void HostPrivatePageNoVA(int receiver, const char *msg) {
	if (!Stewie_cPlayerManager::Is_Player_Present(receiver) || !msg) { return; }

	/*
	char m[512];
	sprintf(m,"ppage %d %s",receiver,msg);
	ConsoleInNoVA(m);
	*/
	wchar_t wcs[256];
	swprintf(wcs,L"%S",msg);
	Stewie_WideStringClass WideStr(wcs);
	Stewie_cScTextObj &newobj = Stewie_cScTextObj::Create();
	newobj.Init(WideStr,2,false,-1,receiver);
}
void datafromdde(char *In) {
	vTokenParser *String = new vTokenParser(In);
	std::string cmd = String->Gettok(1);
	if (!stricmp(cmd.c_str(),"building_info")) {
		Building_Status();
		return;
	} else if (!stricmp(cmd.c_str(),"s_pinfo") || !stricmp(cmd.c_str(),"pinfo")) {
		ConsoleInNoVA("v_pinfo");
		return;
	} else if (!stricmp(cmd.c_str(),"message") || !stricmp(cmd.c_str(),"msg")) {
		HostMessage(String->Gettok(2,-1).c_str());
	}
	ConsoleInNoVA((const char *)In);
	String->Delete();
}

void Set_Auto_Restart(bool restart) {
	__asm {
		push restart
		mov ecx,[0x000000] // For security and licensing purposes, an address has been hidden from this line
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
	}
}
int Set_SFPS_Limit(int NewFps) {
	signed char Delay = 0;
	if (NewFps != 0) {
		if (NewFps < 8) { NewFps = 8; }
		Delay = (signed char)(1000 / NewFps);
	}

	DWORD OldProtect;
	HANDLE Process = GetCurrentProcess();
	VirtualProtectEx(Process, (LPVOID)0x000000, 1, PAGE_EXECUTE_READWRITE, &OldProtect); // For security and licensing purposes, an address has been hidden from this line
	if (!WriteProcessMemory(Process, (LPVOID)0x000000, (LPVOID)&Delay, 1, NULL)) { return -1; } // For security and licensing purposes, an address has been hidden from this line
	VirtualProtectEx(Process, (LPVOID)0x000000, 1, OldProtect, &OldProtect); // For security and licensing purposes, an address has been hidden from this line
	VirtualProtectEx(Process, (LPVOID)0x000000, 1, PAGE_EXECUTE_READWRITE, &OldProtect); // For security and licensing purposes, an address has been hidden from this line
	if (!WriteProcessMemory(Process, (LPVOID)0x000000, (LPVOID)&Delay, 1, NULL)) { return -1; } // For security and licensing purposes, an address has been hidden from this line
	VirtualProtectEx(Process, (LPVOID)0x000000, 1, OldProtect, &OldProtect); // For security and licensing purposes, an address has been hidden from this line

	if (Delay != 0) { return (int)(1000 / Delay); }
	return 0;
}

int Get_Script_Int_Parameter(GameObject *obj, const char *script, const char *parameter) {
	if (!Commands->Get_ID(obj) || !obj) {
		return false;
	}
	void *ptr = (void *)(obj+0x6EC);
	SimpleDynVecClass<GameObjObserverClass *> *observers = (SimpleDynVecClass<GameObjObserverClass *>*)ptr;
	int x = observers->Count();
	for (int i = 0; i < x; i++) {
		if (!stricmp((*observers)[i]->Get_Name(),script)) {
			return ((ScriptImpClass *)(*observers)[i])->Get_Int_Parameter(parameter);
		}
	}
	return 0;
}

std::string Duration(int secs) {
	return Duration(float(secs));
}
std::string Duration(float secs) {
	if (secs <= 0) { return "0secs"; }
	char m[64],m1[64],m2[64],m3[64],m4[64],m5[64],m6[64];
	float years = 0,weeks = 0,days = 0,hours = 0,minutes = 0;
	years = floor((float)(secs / 31557600));
	secs = secs - (years * 31557600);
	weeks = floor((float)(secs / 604800));
	secs = secs - (weeks * 604800);
	days = floor((float)(secs / 86400));
	secs = secs - (days * 86400);
	hours = floor((float)(secs / 3600));
	secs = secs - (hours * 3600);
	minutes = floor((float)(secs / 60));
	secs = secs - (minutes * 60);
	if (years > 1) { sprintf(m1,"%dyrs ",(int)years); }
	else if (years == 1) { sprintf(m1,"%dyr ",(int)years); }
	else { sprintf(m1,""); }
	if (weeks > 1) { sprintf(m2,"%dwks ",(int)weeks); }
	else if (weeks == 1) { sprintf(m2,"%dwk ",(int)weeks); }
	else { sprintf(m2,""); }
	if (days > 1) { sprintf(m3,"%ddays ",(int)days); }
	else if (days == 1) { sprintf(m3,"%dday ",(int)days); }
	else { sprintf(m3,""); }
	if (hours > 1) { sprintf(m4,"%dhrs ",(int)hours); }
	else if (hours == 1) { sprintf(m4,"%dhr ",(int)hours); }
	else { sprintf(m4,""); }
	if (minutes > 1) { sprintf(m5,"%dmins ",(int)minutes); }
	else if (minutes == 1) { sprintf(m5,"%dmin ",(int)minutes); }
	else { sprintf(m5,""); }
	if (secs > 1) { sprintf(m6,"%dsecs",(int)secs); }
	else if (secs == 1) { sprintf(m6,"%dsec",(int)secs); }
	else { sprintf(m6,""); }
	sprintf(m,"%s%s%s%s%s%s",(const char *)m1,(const char *)m2,(const char *)m3,(const char *)m4,(const char *)m5,(const char *)m6);
	std::string Retn = m;
	return Retn;
}
int Random(int Lower, int Upper, bool Default) {
	// If Default is true, then Random(0,5) will be 0-4. Otherwise, it will be 0-5.
	if (Default) { Upper -= 1; }
	if (Lower == Upper) { return Lower; }
	else if (Lower > Upper) { return 0; }
	GetRand:
		int Rand = (Lower + int((Upper - Lower + 1) * rand() / (RAND_MAX + 1.0f)));
		if (Rand < Lower || Rand > Upper) {
			LOGFUNC("Random is out of range, for whatever reason...");
			goto GetRand;
		}
	return Rand;
}
float MinZero(float f) { return (f > 0.0f ? f : 0.0f); }
int MinZero(int i) { return (i > 0 ? i : 0); }

vConfig::vConfig() {
}
void vConfig::Rehash(bool First, bool Level, bool Console) {
	char *ConfigFile = "vgm.ini";
	srand((unsigned)time(0));

	if (Level || Console) {
		if (Level) {
			Config->GDIHarvesterDump = 300;
			Config->NodHarvesterDump = 300;
			this->Set_GameMode(getProfileInt("Settings","GameMode",1,ConfigFile));
		}

		int NSFPS = getProfileInt("Settings","SFPS",60,ConfigFile);
		if (NSFPS >= 0) { Set_SFPS_Limit(NSFPS); }

		IncomePerSec = getProfileInt("GameSettings","RefineryPerSec",2,ConfigFile);
		EnableShells = getProfileInt("GameSettings","VehShells",1,ConfigFile);
		EnableHShells = getProfileInt("GameSettings","HarvShells",1,ConfigFile);
		MaxFriendlyVeh = getProfileInt("GameSettings","MaxFriendVeh",1,ConfigFile);
		MaxEnemyVeh = getProfileInt("GameSettings","MaxEnemyVeh",4,ConfigFile);
		if (GameMode == 2 || GameMode == 4) { Autobind = false; }
		else { Autobind = (getProfileInt("GameSettings","Autobind",1,ConfigFile) == 1 ? true : false); }
		Weather = (getProfileInt("Settings","Weather",0,ConfigFile) == 1 ? true : false);
		if (Level) {
			AutoRestart = (getProfileInt("Settings","AutoRestart",0,ConfigFile) == 1 ? true : false);
			Set_Auto_Restart(AutoRestart);
			GDIHarvesterDump = NodHarvesterDump = getProfileInt("GameSettings","HarvesterDump",300,ConfigFile);
			GDIVehicleLimit = NodVehicleLimit = (getProfileInt("GameSettings","VehicleLimit",7,ConfigFile) + 1);
			getProfileString("Settings","LogFile","vgm",vLogFile,256,ConfigFile);
		}
		Crates = (getProfileInt("Settings","Crates",0,ConfigFile) == 1 ? true : false);
		Crystal = (getProfileInt("Settings","Crystal",0,ConfigFile) == 1 ? true : false);
		CustomRadio = (getProfileInt("Settings","CustomRadio",0,ConfigFile) == 1 ? true : false);
		Sounds = (getProfileInt("Settings","Sounds",0,ConfigFile) == 1 ? true : false);
		MedalsEnabled = (getProfileInt("Settings","Medals",0,ConfigFile) == 1 ? true : false);
		MedalsRestricted = (getProfileInt("MedalUsers","Restricted",0,"vgm_users.ini") == 1 ? true : false);

		TotalCommanders = getProfileInt("CommanderList","TotalCommanders",0,"vgm_users.ini");

		VetEnabled = (getProfileInt("Veteran","Enable",1,ConfigFile) == 1 ? true : false);
		VetLevels = getProfileInt("Veteran","Levels",5,"vgm_presets.ini");
		vVetManager->Promo_VetPtsReqd.clear();
		vVetManager->Promo_ScoreReqd.clear();
		vVetManager->Promo_TimeIngameReqd.clear();
		vVetManager->Promo_ArmorBonusGained.clear();
		vVetManager->Promo_AutoHealSeconds.clear();
		for (int i = 1; i <= VetLevels; i++) {
			char entry[64]; float f = 0.0f; int j = 0;
			sprintf(entry,"Points%d",i);
			f = getProfileFloat("VeteranPresets",entry,0.0f,"vgm_presets.ini");
			vVetManager->Promo_VetPtsReqd.push_back(f);
			sprintf(entry,"Score%d",i);
			f = getProfileFloat("VeteranPresets",entry,0.0f,"vgm_presets.ini");
			vVetManager->Promo_ScoreReqd.push_back(f);
			sprintf(entry,"Time%d",i);
			j = getProfileInt("VeteranPresets",entry,0,"vgm_presets.ini");
			vVetManager->Promo_TimeIngameReqd.push_back(j);
			sprintf(entry,"Bonus%d",i);
			f = getProfileFloat("VeteranPresets",entry,0.0f,"vgm_presets.ini");
			vVetManager->Promo_ArmorBonusGained.push_back(f);
			sprintf(entry,"AHeal%d",i);
			j = getProfileInt("VeteranPresets",entry,0,"vgm_presets.ini");
			vVetManager->Promo_AutoHealSeconds.push_back(j);
		}

		if (Level) {
			GDIRemoteC4Limit = NODRemoteC4Limit = getProfileInt("GameSettings","RemoteC4Limit",20,ConfigFile);
			GDITimedC4Limit = NODTimedC4Limit = getProfileInt("GameSettings","TimedC4Limit",-1,ConfigFile);
			GDIProximityC4Limit = NODProximityC4Limit = getProfileInt("GameSettings","ProximityC4Limit",35,ConfigFile);
		}

		SerialTimeoutDuration = getProfileInt("AntiCheat","SerialTimeout",10,ConfigFile);
		EnableAntiBighead = (getProfileInt("AntiCheat","AntiBighead",0,ConfigFile) == 1 ? true : false);
		CheckDamageEvents = (getProfileInt("AntiCheat","CheckDamageEvents",0,ConfigFile) == 1 ? true : false);
		Autokick = (getProfileInt("AntiCheat","Autokick",0,ConfigFile) == 1 ? true : false);
		OutputDamageEvents = (getProfileInt("AntiCheat","OutputInvalidDamageEvents",0,ConfigFile) == 1 ? true : false);
		BlockDamageEvents = (getProfileInt("AntiCheat","BlockInvalidDamageEvents",0,ConfigFile) == 1 ? true : false);
		OutputPurchases = (getProfileInt("AntiCheat","OutputInvalidPurchases",0,ConfigFile) == 1 ? true : false);
		BlockPurchases = (getProfileInt("AntiCheat","BlockInvalidPurchases",0,ConfigFile) == 1 ? true : false);
		BlockAllPurchases = (getProfileInt("AntiCheat","BlockAllPurchases",0,ConfigFile) == 1 ? true : false);
		MaximumPurchaseDistance = getProfileInt("AntiCheat","MaxDist",15,ConfigFile);
		StoreHitDataSecs = getProfileFloat("AntiCheat","StoreHitDataSecs",1.0f,ConfigFile) + 1.0f; // Add 1.0 to account for time differences during ::Think.
		
		DetectDmgHack = (getProfileInt("AntiCheat","DetectDmgHack",0,ConfigFile) == 1 ? true : false);
		DetectWHHack = (getProfileInt("AntiCheat","DetectWHHack",0,ConfigFile) == 1 ? true : false);
		DetectScopeHack = (getProfileInt("AntiCheat","DetectScopeHack",0,ConfigFile) == 1 ? true : false);
		DetectROFHack = (getProfileInt("AntiCheat","DetectROFHack",0,ConfigFile) == 1 ? true : false);
		DetectDistHack = (getProfileInt("AntiCheat","DetectDistHack",0,ConfigFile) == 1 ? true : false);
		LogSpectator = (getProfileInt("AntiCheat","LogSpectator",0,ConfigFile) == 1 ? true : false);

		ExtraDefenses = (getProfileInt("GameSettings","ExtraDefenses",0,ConfigFile) == 1 ? true : false);
		OffensiveModeTime = getProfileInt("GameSettings","Offensive",-1,ConfigFile);
		FirstBloodCreds = getProfileFloat("GameSettings","FirstBlood",250.0f,ConfigFile);
		ShowPrivateChat = (getProfileInt("Settings","ShowPrivChat",0,ConfigFile) == 1 ? true : false);
		BlockNoSerialActions = (getProfileInt("Settings","BlockNoSerial",0,ConfigFile) == 1 ? true : false);
		EnableConnectionHook = (getProfileInt("Settings","ConnectHook",0,ConfigFile) == 1 ? true : false);
		ResolveHostnames = (getProfileInt("Settings","ResolveHostnames",0,ConfigFile) == 1 ? true : false);
		ResolveTimeoutDuration = getProfileInt("Settings","ResolveTimeout",3,ConfigFile);

		MapSpecificSettings = (getProfileInt("Settings","UseMapSettings",0,ConfigFile) == 1 ? true : false);
		MapTimeLimit = getProfileInt(GameDataObj()->MapName,"MapTimeLimit",-1,"vgm_rvc.ini") * 60;
		if (MapSpecificSettings) { DonateTime = getProfileInt(GameDataObj()->MapName,"DonationLimit",0,"vgm_rvc.ini") * 60; }
		else { DonateTime = 0; }
		SBHGetGoodCrates = getProfileFloat("Settings","SBHGetGoodCrate",0.0f,"vgm_crates.ini");
#ifdef _DEV_
		VetCmdCooldown = 10;
#else
		VetCmdCooldown = 300;
#endif

		char m[512];
		vTokenParser *Str = new vTokenParser();
		EngiSpawnWeapons.clear();
		getProfileString("SpawnWeapons","BasicEngineer","",m,512,"vgm_presets.ini");
		Str->Set(m);
		Str->Parse(",");
		for (int i = 1; i <= Str->Numtok(); i++) {
			EngiSpawnWeapons.push_back(Str->Gettok(i).c_str());
		}
		AdvEngiSpawnWeapons.clear();
		getProfileString("SpawnWeapons","AdvEngineer","",m,512,"vgm_presets.ini");
		Str->Set(m);
		Str->Parse(",");
		for (int i = 1; i <= Str->Numtok(); i++) {
			AdvEngiSpawnWeapons.push_back(Str->Gettok(i).c_str());
		}
		OtherSpawnWeapons.clear();
		getProfileString("SpawnWeapons","Other","",m,512,"vgm_presets.ini");
		Str->Set(m);
		Str->Parse(",");
		for (int i = 1; i <= Str->Numtok(); i++) {
			OtherSpawnWeapons.push_back(Str->Gettok(i).c_str());
		}
		Str->Delete();
	}

	if (First) {
		Config->CrateSpawnPositions.clear();
		if (Crates) {
			CrateSpawnTotal = 0;
			char *CurrMap = GameDataObj()->MapName;
			int SpawnTot = getProfileInt(CurrMap,"LocNum",0,"vgm_crates.ini");
			for (int i = 1; i <= SpawnTot; i++) {
				char RandLoc[512];
				char SpawnLoc[512];
				sprintf(RandLoc,"Loc%d",i);
				getProfileString(CurrMap,RandLoc,"0 0 0",SpawnLoc,512,"vgm_crates.ini");
				const char *NewSpawn = (char *)SpawnLoc;
				char* mess = new char[strlen(NewSpawn) + 1];
				strcpy(mess,NewSpawn);
				int tokens = numtok(mess,32);
				if (tokens != 3) { NewSpawn = "0 0 0"; }
				float SpawnX = (float)atof(strtok(mess," "));
				float SpawnY = (float)atof(strtok(NULL," "));
				float SpawnZ = (float)atof(strtok(NULL," "));
				Stewie_Vector3 NewLocation(SpawnX,SpawnY,SpawnZ);
				CrateSpawnPositions.push_back(NewLocation);
				CrateSpawnTotal++;
				delete[] mess;
			}
		}
	}
}

typedef bool (*DataSafeSetUInt) (void *Handle, unsigned int &Data);
typedef bool (*DataSafeSetInt) (void *Handle, int &Data);
typedef bool (*DataSafeSetFloat) (void *Handle, float &Data);
DataSafeSetUInt vDataSafeSetUInt = (DataSafeSetUInt)0x000000; // For security and licensing purposes, an address has been hidden from this line
DataSafeSetInt vDataSafeSetInt = (DataSafeSetInt)0x000000; // For security and licensing purposes, an address has been hidden from this line
DataSafeSetFloat vDataSafeSetFloat = (DataSafeSetFloat)0x000000; // For security and licensing purposes, an address has been hidden from this line

bool Stewie_UIntDataSafeClass::Set(unsigned int &data) {
	return vDataSafeSetUInt(Data,data);
}
bool Stewie_IntDataSafeClass::Set(int &data) {
	return vDataSafeSetInt(Data,data);
}
bool Stewie_FloatDataSafeClass::Set(float &data) {
	return vDataSafeSetFloat(Data,data);
}