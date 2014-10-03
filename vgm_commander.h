#pragma once

#include "vgm_engine.h"
#include "vgm_presets.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

extern int GDICommander;
extern int NodCommander;
extern int GDITeamPool;
extern int NODTeamPool;

int Get_Next_Commander(int team);
bool Is_Listed_Commander_Available(int team);
bool Has_Commander(int team);
void Set_Team_Commander(int ID, int team);
void Remove_Team_Commander(int team);
bool Is_Commander(int ID, int team, bool potential);
bool Is_Commander(const char *name, int team, bool potential);
