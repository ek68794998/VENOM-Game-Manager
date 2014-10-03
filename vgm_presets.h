#pragma once

#include "vgm_engine.h"
#include "vgm_obj.h"
#include "vgm_string.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

std::string Get_Translated_Name(unsigned int ID);
std::string Get_Translated_Team_Name(int t);
std::string Get_Pretty_Name(GameObject *obj);
std::string Get_Pretty_Name(const char *preset);
std::string Get_Preset_Info(GameObject *obj);
std::string Get_Translated_C4_Mode(int t);
std::string Get_Purchase_Unit(int ptype, int pressed, int alt, int team);
std::string Get_Purchase_Type(int ptype);
std::string Get_Icon_Bone(const char *Preset, int IconNum);
std::string Get_Preset_From_Short(const char* Area, const char* Preset, bool None = true);
