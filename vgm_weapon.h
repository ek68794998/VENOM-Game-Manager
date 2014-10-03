#pragma once

#include "vgm_engine.h"
#include "vgm_player.h"
#include "vgm_presets.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vWeaponManager;

extern vWeaponManager *vWManager;

struct vWeaponPack {
	int PackID;
	int CreationTime;
	std::vector<Stewie_WeaponDefinitionClass*> Weapons;
};

enum WeaponState {
	wsNone = 0,
	wsReady,
	wsUnknown2,
	wsPrimaryFiring,
	wsSecondaryFiring,
	wsReloading,
	wsSwitching,
	wsWaiting
};

float __stdcall GetDmgMultiplier(Stewie_DefenseObjectClass& Defense, Stewie_OffenseObjectClass& Offense);

void Beacon_All_Buildings(int Team, int type, bool preanim);
std::string Buildings_Beacon_Will_Hit(GameObject *obj, int team, bool showdamage);

Stewie_WeaponClass *Get_Held_Weapon(Stewie_SoldierGameObj *s);
Stewie_WeaponBagClass *Get_Weapon_Bag(Stewie_ArmedGameObj* o);
std::string Get_Weapon_List(Stewie_ArmedGameObj *o, const char *separator = ",");
void Force_Refill(Stewie_ArmedGameObj *o);
void Powerup(Stewie_SoldierGameObj *o, const char *preset, bool hud = true);

void __stdcall SelectWeaponHook(Stewie_WeaponClass* w);

class vWeaponManager {
private:
	std::vector<vWeaponPack> Packs;

public:
	void Init() { Packs.clear(); }
	void Think(bool FullSecond);
	bool Collect(int PID, int PackID);
	void Create_Pack(int PID, bool DefaultWeapons);
	void Create_Pack_Weapon(Stewie_Vector3 Pos, const char *Weapon, bool UseBackpack = false, bool Expires = false);
	static void Give_Weapon(int ID, const char *preset);
};
