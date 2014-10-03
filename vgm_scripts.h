#pragma once

#include "vgm_engine.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

#define STSCRIPT(scr, name, scn, par) \
	Stewie_ScriptRegistrant<scr> name(scn,par);

bool Script_Attach(Stewie_ScriptableGameObj *obj, const char *Name, const char *Params = "", bool Once = false);
void Script_Remove_All(Stewie_ScriptableGameObj *obj);
void Script_Remove(Stewie_ScriptableGameObj *obj, const char *Name);
void Script_Remove_Duplicate(Stewie_ScriptableGameObj *obj, const char *Name);
bool Script_Attached(Stewie_ScriptableGameObj *obj, const char *Name);
int Script_Count(Stewie_ScriptableGameObj *obj, const char *Name);
