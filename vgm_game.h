#pragma once

#include "vgm_engine.h"
#include "vgm_actions.h"
#include "vgm_bighead.h"
#include "vgm_commander.h"
#include "vgm_keys.h"
#include "vgm_medals.h"
#include "vgm_obj.h"
#include "vgm_player.h"
#include "vgm_presets.h"
#include "vgm_scripts.h"
#include "vgm_string.h"
#include "vgm_weapon.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

extern bool Offensive;
extern bool EMPon;
extern bool GGstarted;

void InitiateOffensive();

void __stdcall ChatMsgHook(Stewie_WideStringClass *string, int vtype, bool popup, int sender, int receiver);
void ChatMsgProcessing(std::string string, int vtype, bool popup, int sender, int receiver);
bool Execute_AOWCommand(int ID, vTokenParser *Str, int type);
bool Execute_SniperCommand(int ID, vTokenParser *Str, int type);
bool Execute_OtherCommand(int ID, vTokenParser *Str, int type);
std::string Execute_Autocomplete(const char *str);
bool Execute_SniperCommand(int ID, vTokenParser *Str, int type);
bool Hide_Command(int ID, vTokenParser *Str, int type);
bool __stdcall RadioHook(Stewie_CSAnnouncement *Event);

int Check_Purchase_Event(Stewie_SoldierGameObj *obj, int vtype, int pressed, int alt);
int __cdecl OnPurchaseHook(Stewie_SoldierGameObj *obj, int vtype, int pressed, int alt, bool unk);
int __cdecl OnPurchaseCharacterHook(Stewie_BaseControllerClass *base, Stewie_SoldierGameObj *obj, int Price, int definition, bool Unk);

bool __stdcall VehicleTransitionHook(GameObject *s, Stewie_TransitionInstanceClass *Transition);
void __stdcall ExitDeadVehicleHook(GameObject *s);
void __stdcall HarvesterDockedHook(Stewie_RefineryGameObj *Owner, Stewie_HarvesterClass *Harvester);

void __stdcall C4InitHook(Stewie_C4GameObj *obj);
void __cdecl C4LimitHook(int team);
bool __stdcall C4ShouldExplodeHook(Stewie_DamageableGameObj *enemy);
bool __stdcall C4DefuseHook(GameObject *obj, Stewie_OffenseObjectClass &offense);
bool __stdcall C4DetonationHook(GameObject *obj);
void __stdcall C4PlantedHook(GameObject *obj);

bool __stdcall EnemySeenHook(Stewie_DamageableGameObj *enemy);

void __stdcall BeaconStateHook(GameObject *obj, int State);
bool __stdcall BeaconRequestedHook(Stewie_BeaconGameObj *obj, Stewie_Vector3& pos);
void __stdcall BeaconDisarmedHook(GameObject *obj, Stewie_OffenseObjectClass *Disarmer);
bool __stdcall BeaconOwnerInterruptedHook(Stewie_BeaconGameObj *beacon);
bool __stdcall PedestalTimerHook(Stewie_BeaconGameObj *beacon);
bool __stdcall PedestalHook(GameObject *beacon);

void Create_2D_WAV_Sound(const char *sound);

class vPlayerObjectHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
	void Killed(GameObject *obj, GameObject *shooter);
	void Destroyed(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
	Vector3 SpawnPos;
	Vector3 DeathPlace;
	bool WasKilled;
};

class vVehicleObjectHandler : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Damaged(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *damager, float damage);
	void Killed(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *shooter);
	void Destroyed(Stewie_ScriptableGameObj *obj);
	void Custom(Stewie_ScriptableGameObj *obj, int message, int param, Stewie_ScriptableGameObj *sender);
	unsigned int DamageExpire;
	float DeathFacing;
	Stewie_Vector3 DeathPlace;
	bool WasKilled;
	int IconID;
	int IconID2;
	bool HarvesterShell;
};

class vTowerObjectHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
	void Killed(GameObject *obj, GameObject *shooter);
	void Destroyed(GameObject *obj);
	float DeathFacing;
	Vector3 DeathPlace;
	bool WasKilled;
};

class vVehicleShellHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
};

class vHarvesterShellHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
	int Team;
};

class vBuildingObjectHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
	void Killed(GameObject *obj, GameObject *shooter);
	void Timer_Expired(GameObject *obj, int number);
	unsigned int DamageExpire;
	unsigned int DamageExpire2;
	float LastDamage;
};

class vSAMSite : public ScriptImpClass {
	int EnemyID;
	float deathdamage;
	float CurrHealth;

	void Created(GameObject *obj);
	void Damaged(GameObject *obj, GameObject *damager, float damage);
	void Destroyed(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
	void Enemy_Seen(GameObject *obj, GameObject *enemy);

	bool IsValidEnemy(GameObject *obj, GameObject *enemy);
};

class SelfRepair : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};
class VetSelfRepair : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class ExpirePowerup : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class Parachute : public ScriptImpClass {
	GameObject *floater;
	GameObject *chute;
	bool Attached;
	Vector3 StartPos;
	int Owner;
	float LastZPos;

	void Created(GameObject *obj);
	void Custom(GameObject *obj, int message, int param, GameObject *sender);
	void Killed(GameObject *obj, GameObject *shooter);
	void Destroyed(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class OffensiveScript : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class TeamCommander : public ScriptImpClass {
	void Custom(GameObject *obj, int message, int param, GameObject *sender);
};

class C4PokeScript : public ScriptImpClass {
	void Poked(GameObject *obj, GameObject *poker);
};

class Ion_Storm : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class IAmGod : public Stewie_ScriptImpClass {
};

class Invisible : public Stewie_ScriptImpClass {
};
