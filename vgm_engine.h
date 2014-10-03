#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <ddeml.h>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#if (_MSC_VER == 1400)
	#pragma comment(lib, "except.lib")
	#pragma comment(linker, "/include:?__CxxSetUnhandledExceptionFilter@@YAHXZ")
#endif

#ifdef DEBUG
	#include "mmgr.h"
#endif

#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"
#include "vgm_dll.h"
#include "vgm_logging.h"
#include "vgm_profile.h"
#include "vgm_scripts.h"

//#define _DEV_ 1
#define SV "v1.5.6b"
#define SLV SV " [" __DATE__ " " __TIME__ "]"
#define GreaterOfTwo(a,b) (a > b ? a : b)
#define LesserOfTwo(a,b) (a > b ? b : a)

class vGameManager;
class vConfig;

extern vGameManager vManager;

extern bool ServerInit;
extern bool EmergencyShutdown;
extern bool FirstKillOfMap;
extern vConfig *Config;
extern Stewie_ScriptableGameObj *InvObj;

#define CONCATENATE(x, y) CONCATENATE_(x, y)
#define CONCATENATE_(x, y) x ## y

#define LOGFUNC(r) { \
		ConsoleOut("Function was logged: %s@%d (reason: %s), Time: %s",__FUNCTION__,__LINE__,r,__TIMESTAMP__); \
		vLogger->Log(vLoggerType::vVGM,"_FDSERR","Requested log - Function: %s@%d (reason: %s)",__FUNCTION__,__LINE__,r); \
	}

#define FOREACH(Vector, Type, Iter) \
	std::vector<##Type##>::iterator Iter = (Vector).begin(); \
	std::vector<##Type##>::iterator CONCATENATE(EndIter_, __LINE__) = (Vector).end(); \
	for (; Iter != CONCATENATE(EndIter_, __LINE__); Iter++)

void __stdcall ThinkHook();
void LevelLoadHook();
void LevelEndHook();

void Set_Auto_Restart(bool restart);
int Set_SFPS_Limit(int NewFps);

void ConsoleInNoVA(const char *in);
void ConsoleIn(const char *in, ...);
void ConsoleOut(const char *out, ...);
void DebugMessage(const char *msg, ...);
void HostMessage(const char *msg, ...);
void HostPrivateMessage(int receiver, const char *msg, ...);
void HostPrivateAdminMessage(int receiver, const char *msg, ...);
void HostPrivatePage(int receiver, const char *msg, ...);
void HostPrivatePageNoVA(int receiver, const char *msg);
void datafromdde(char In[]);

int Get_Script_Int_Parameter(GameObject *obj, const char *script, const char *parameter);

std::string Duration(int secs);
std::string Duration(float secs);
int Random(int Lower, int Upper, bool Default = false);
float MinZero(float f);
int MinZero(int i);

using namespace std;
using std::vector;

class vGameManager {
public:
	float GameDuration;
	float QuarterSecTimer;
	float FullSecTimer;
	float ClearHitsTimer;

	void Init();
	void Shutdown();
	void Think();
	void Level_Loaded();
	void Level_Ended();
	void Game_Reset();
	int DurationAsInt() { return (int)(floor(GameDuration)); }
	unsigned int DurationAsUint() { return (unsigned int)(floor(GameDuration)); }
	float DurationAsFloat() { return GameDuration; }
	
	vGameManager() {
		GameDuration = 0.0f;
	};
};

class vConfig {
public:
	enum GAMEMODE {
		vAOW = 1,
		vSNIPER,
		vMONEY,
		vINFONLY
	};

	vConfig();
	void Rehash(bool First, bool Level, bool Console);
	int Get_GameMode() { return (int)GameMode; }
	void Set_GameMode(int mode) {
		if (mode < 1 || mode > 4) { mode = 1; }
		GameMode = (GAMEMODE)mode;
	}

	// Base Settings
	GAMEMODE GameMode;
	bool EnableShells;
	bool EnableHShells;
	int MaxFriendlyVeh;
	int MaxEnemyVeh;
	bool Autobind;
	bool Weather;
	bool AutoRestart;
	int IncomePerSec;
	int GDIHarvesterDump;
	int NodHarvesterDump;
	int GDIVehicleLimit;
	int NodVehicleLimit;
	char vLogFile[256];
	bool Crates;
	bool Crystal;
	bool CustomRadio;
	bool Sounds;
	bool MedalsEnabled;
	bool MedalsRestricted;
	bool ExtraDefenses;

	// Commander Stuff
	int TotalCommanders;

	// Veteran Stuff
	bool VetEnabled;
	int VetLevels;

	// Spawn Stuff
	int CrateSpawnTotal;
	std::vector<Stewie_Vector3> CrateSpawnPositions;
	std::vector<std::string> EngiSpawnWeapons;
	std::vector<std::string> AdvEngiSpawnWeapons;
	std::vector<std::string> OtherSpawnWeapons;

	// C4 Stuff
	int GDIRemoteC4Limit;
	int GDITimedC4Limit;
	int GDIProximityC4Limit;
	int NODRemoteC4Limit;
	int NODTimedC4Limit;
	int NODProximityC4Limit;

	// Anti-Cheat
	int SerialTimeoutDuration;
	bool EnableAntiBighead;
	bool CheckDamageEvents;
	bool Autokick;
	bool OutputDamageEvents;
	bool BlockDamageEvents;
	bool OutputPurchases;
	bool BlockPurchases;
	bool BlockAllPurchases;
	int MaximumPurchaseDistance;
	float StoreHitDataSecs;
	bool DetectDmgHack;
	bool DetectWHHack;
	bool DetectScopeHack;
	bool DetectROFHack;
	bool DetectDistHack;
	bool LogSpectator;

	// Other
	int OffensiveModeTime;
	float FirstBloodCreds;
	bool ShowPrivateChat;
	bool BlockNoSerialActions;
	bool EnableConnectionHook;
	bool ResolveHostnames;
	int ResolveTimeoutDuration;
	bool CrazyDefensesEnabled;
	bool MapSpecificSettings;
	int MapTimeLimit;
	int DonateTime;
	float SBHGetGoodCrates;
	int VetCmdCooldown;
};

template <class T> class vVector {
public:
	T* Vector;
};

class vDate {
public:
	int month;
	int day;
	int year;
	static char out_string[25];
	static char last_date[25];
	int days_this_month();

	vDate() { refreshdate(); }
	void refreshdate() {
		time_t time_date;
		struct tm *current_date;
		time_date = time(NULL);
		current_date = localtime(&time_date);
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year + 1900;
	}
	char *get_date_string() {
		if (day < 10) { sprintf(out_string, "%d-%d-%04d", month, day, year); }
		else { sprintf(out_string, "%d-%02d-%04d", month, day, year); }
		strcpy(last_date,out_string);
		return out_string;
	}
};
