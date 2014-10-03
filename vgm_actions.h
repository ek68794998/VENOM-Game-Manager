#pragma once

#include "vgm_engine.h"
#include "vgm_weapon.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

void Building_Status();
void Do_Damage(Stewie_BaseGameObj *obj, float Damage, const char* Warhead = "None", Stewie_BaseGameObj *Damager = NULL);
void Damage_All_Soldiers_Area_Team(float Damage, const char *Warhead, Vector3 Position, float Distance, GameObject *Host, GameObject *Damager, int Team);
void Give_Weapon(int ID, const char *preset);
void Give_All_Weapons(int ID, bool AllDefs = true);
bool Is_Spectating(Stewie_BaseGameObj *obj);
bool Is_Spectating(GameObject *o);
void Kill_Player(int ID);
void Kill_Player(GameObject *o);
void Move_Player(int ID, float X, float Y, float Z, bool relative, bool vehicle = false);
void Set_Speed(Stewie_SoldierGameObj *obj, float spd);
bool Spectate_Player(int ID, bool quiet);
void Toggle_Fly_Mode(Stewie_SoldierGameObj *obj);
