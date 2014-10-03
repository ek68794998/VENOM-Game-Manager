#include "vgm_presets.h"

std::string Get_Translated_Name(unsigned int ID) {
	ID -= 1000;
	if (ID < 0) { return "No String"; }
	Stewie_TDBObjClass *obj = Stewie_TranslateDBClass::Get_Object(ID);
	if (!obj) { return "No String"; }
	return WCharToStr(obj->Get_String()->m_Buffer);
}
std::string Get_Translated_Team_Name(int t) {
	if (t == 0) { return Get_Translated_Name(7198); }
	else if (t == 1) { return Get_Translated_Name(7199); }
	return "Neutral";
}
std::string Get_Pretty_Name(GameObject *obj) {
	if (!obj) { return "No String"; }
	return Get_Pretty_Name(Get_Preset_Name(obj));
}
std::string Get_Pretty_Name(const char *preset) {
	if (!preset) { return "No String"; }
	char TransName[256];
	getProfileString("PresetList",preset,preset,TransName,256,"vgm_presets.ini");
	std::string Retn = TransName;
	if (!stricmp(Retn.c_str(),preset)) {
		Stewie_DamageableGameObjDef *Def = (Stewie_DamageableGameObjDef *)(Stewie_DefinitionMgrClass::Find_Definition(preset));
		if (!Def || Def->EncyclopediaID <= 0) { return Retn; }
		unsigned int NameID = Def->NameID;
		std::string TDB = Get_Translated_Name(NameID);
		if (stricmp(TDB.c_str(),"No String") && stricmp(TDB.c_str(),"")) {
			if ((Def->DefaultPlayerType == 1 && !isin(TDB.c_str(),Get_Translated_Team_Name(Def->DefaultPlayerType).c_str())) || 
				(Def->DefaultPlayerType == 0 && !isin(TDB.c_str(),Get_Translated_Team_Name(Def->DefaultPlayerType).c_str()))) {
				char m[512];
				sprintf(m,"%s %s",Get_Translated_Team_Name(Def->DefaultPlayerType).c_str(),TDB.c_str());
				TDB = m;
			}
			return TDB;
		}
	}
	return Retn;
}
std::string Get_Preset_Info(GameObject *obj) {
	if (!obj || Commands->Get_ID(obj) <= 0) { return "None"; }
	else if (Is_Vehicle(obj)) { return Get_Pretty_Name(obj); }
	else if (Get_Vehicle(obj)) { return Get_Pretty_Name(Get_Vehicle(obj)); }
	else if (Is_Soldier(obj)) {
		char m[512];
		std::string wname = Get_Pretty_Name(Get_Current_Weapon(obj));
		if (!stricmp(wname.c_str(),"No String")) { sprintf(m,"%s",Get_Pretty_Name(obj).c_str()); }
		else { sprintf(m,"%s/%s",Get_Pretty_Name(obj).c_str(),wname.c_str()); }
		std::string Retn = m;
		return Retn;
	}
	return Get_Pretty_Name(obj);
}
std::string Get_Translated_C4_Mode(int t) {
	if (t == 1) { return "Remote"; }
	else if (t == 2) { return "Timed"; }
	else if (t == 3) { return "Proximity"; }
	return "Unknown";
}
std::string Get_Purchase_Unit(int ptype, int pressed, int alt, int team) {
	unsigned int PresetID = 0;
	bool PrependTeam = false;
	Stewie_TeamPurchaseSettingsDefClass *TeamPurchaseDef = Stewie_TeamPurchaseSettingsDefClass::Get_Definition(1 - team);
	if (ptype == (int)vPREFILL) {
		PresetID = TeamPurchaseDef->RefillStringID;
	} else if (ptype == (int)vPBEACON) {
		PresetID = TeamPurchaseDef->BeaconStringID;
	} else if (ptype == (int)vPBASICCHAR) {
		PresetID = TeamPurchaseDef->StringIDs[pressed];
	} else {
		Stewie_PurchaseSettingsDefClass *PurchaseDef = Stewie_PurchaseSettingsDefClass::Find_Definition(ptype,1 - team);
		PresetID = PurchaseDef->StringIDs[pressed];
	}
	if (ptype == (int)vPBASICCHAR || ptype == (int)vPADVCHAR) { PrependTeam = true; }
	char m[512];
	if (PrependTeam) { sprintf(m,"%s %s",Get_Translated_Team_Name(team).c_str(),Get_Translated_Name(PresetID).c_str()); }
	else { sprintf(m,"%s",Get_Translated_Name(PresetID).c_str()); }
	return std::string((const char *)m);
	/*if (team == 0) {
		if (ptype == 0) {
			switch (pressed) {
				case 0: return "Nod Officer";
				case 1: return "Nod Rocket Soldier Officer";
				case 2: return "Chem Warrior";
				case 3: return "Black Hand Sniper";
				case 4: return "LCG Black Hand";
				case 5: return "Stealth Black Hand";
				case 6: return "Sakura";
				case 7: {
					switch (alt) {
						case -1: return "Raveshaw";
						case 0: return "Mutant Raveshaw";
						default: return "None";
					}
				}
				case 8: return "Mendoza";
				case 9: return "Technician";
				default: return "None";
			}
		} else if (ptype == 1) {
			switch (pressed) {
				case 0: return "Nod Buggy";
				case 1: return "Nod APC";
				case 2: return "Nod Mobile Artillery";
				case 3: return "Nod Flame Tank";
				case 4: return "Nod Light Tank";
				case 5: return "Nod Stealth Tank";
				case 6: return "Nod Transport Helicopter";
				case 7: return "Nod Apache";
				default: return "None";
			}
		} else if (ptype == 2) {
			switch (pressed) {
				case 0: return "Nod Minigunner";
				case 1: return "Nod Shotgunner";
				case 2: return "Nod Flamethrower";
				case 3: return "Nod Engineer";
			}
		} else if (ptype == 3) { return "Nuclear Strike Beacon"; }
		else if (ptype == 4) { return "Refill"; }
	} else if (team == 1) {
		if (ptype == 0) {
			switch (pressed) {
				case 0: return "GDI Officer";
				case 1: return "GDI Rocket Soldier Officer";
				case 2: return "Tiberium Sydney";
				case 3: return "Deadeye";
				case 4: return "Gunner";
				case 5: return "Patch";
				case 6: {
					switch (alt) {
						case -1: return "Commando Havoc";
						case 0: return "Nightops Havoc";
						case 1: return "Snowstorm Havoc";
						case 2: return "Desert Camo Havoc";
						default: return "None";
					}
				}
				case 7: {
					switch (alt) {
						case -1: return "PIC Sydney Prototype";
						case 0: return "PIC Sydney";
						default: return "None";
					}
				}
				case 8: {
					switch (alt) {
						case -1: return "Mobius";
						case 0: return "Mobius Prototype";
						default: return "None";
					}
				}
				case 9: return "Hotwire";
				default: return "None";
			}
		} else if (ptype == 1) {
			switch (pressed) {
				case 0: return "GDI Humm-vee";
				case 1: return "GDI APC";
				case 2: return "GDI MRLS";
				case 3: return "GDI Medium Tank";
				case 4: return "GDI Mammoth Tank";
				case 5: return "GDI Transport Helicopter";
				case 6: return "GDI Orca";
				default: return "None";
			}
		} else if (ptype == 2) {
			switch (pressed) {
				case 0: return "GDI Minigunner";
				case 1: return "GDI Shotgunner";
				case 2: return "GDI Grenadier";
				case 3: return "GDI Engineer";
			}
		} else if (ptype == 3) { return "Ion Cannon Beacon"; }
		else if (ptype == 4) { return "Refill"; }
	}
	return "None";*/
}
std::string Get_Purchase_Type(int ptype) {
	switch (ptype) {
		case 0: return "Adv. Inf.";
		case 1: return "Vehicle";
		case 2: return "Basic Inf.";
		case 3: return "Beacon";
		case 4: return "Refill";
		case 5: return "Secret Inf.";
		case 6: return "Secret Veh.";
		default: return "Unknown";
	}
}
std::string Get_Icon_Bone(const char *Preset, int IconNum) {
	if (IconNum == 1) {
		if (strstr(Preset,"CnC_Nod_Buggy")) return "muzzlea0";
		else if (strstr(Preset,"CnC_Nod_APC")) return "barrel";
		else if (strstr(Preset,"CnC_Nod_Mobile_Artillery")) return "muzzlea0";
		else if (strstr(Preset,"CnC_Nod_Light_Tank")) return "muzzlea0";
		else if (strstr(Preset,"CnC_Nod_Flame_Tank")) return "muzzlea0";
		else if (strstr(Preset,"CnC_Nod_Apache")) return "muzzleb0";
		else if (strstr(Preset,"CnC_GDI_Humm-vee")) return "muzzlea0";
		else if (strstr(Preset,"CnC_GDI_APC")) return "muzzlea0";
		else if (strstr(Preset,"CnC_GDI_MRLS")) return "muzzlea0";
		else if (strstr(Preset,"CnC_GDI_Medium_Tank")) return "muzzlea0";
		else if (strstr(Preset,"CnC_GDI_Mammoth_Tank")) return "muzzleb0";
		else if (strstr(Preset,"CnC_GDI_Orca")) return "muzzlea0";
	} else if (IconNum == 2) {
		if (strstr(Preset,"CnC_Nod_Flame_Tank")) return "muzzlea1";
		else if (strstr(Preset,"CnC_Nod_Apache")) return "muzzleb1";
		else if (strstr(Preset,"CnC_GDI_MRLS")) return "barrel";
		else if (strstr(Preset,"CnC_GDI_Mammoth_Tank")) return "muzzleb1";
		else if (strstr(Preset,"CnC_GDI_Orca")) return "muzzlea1";
	}
	return "NULL";
}
std::string Get_Preset_From_Short(const char* Area, const char* Preset, bool None) {
	std::string info = "None";
	if (None == false) { info = Preset; }
	char p[512];
	sprintf(p,"%s",To_Lowercase(Preset).c_str());
	if (!stricmp(Area,"veh")) {
		if (strstr(p,"orca")) info = "CnC_GDI_Orca";
		else if (strstr(p,"mrl")) info = "CnC_GDI_MRLS";
		else if (strstr(p,"mtank")) info = "CnC_GDI_Medium_Tank";
		else if (strstr(p,"mammoth")) info = "CnC_GDI_Mammoth_Tank";
		else if (strstr(p,"hummvee")) info = "CnC_GDI_Humm-vee";
		else if (strstr(p,"gdiapc")) info = "CnC_GDI_APC";
		else if (strstr(p,"gditransport")) info = "CnC_GDI_Transport";
		else if (strstr(p,"gdiharvester")) info = "CnC_GDI_Harvester";
		else if (strstr(p,"buggy")) info = "CnC_Nod_Buggy";
		else if (strstr(p,"nodapc")) info = "CnC_Nod_APC";
		else if (strstr(p,"apache")) info = "CnC_Nod_Apache";
		else if (strstr(p,"arty")) info = "CnC_Nod_Mobile_Artillery";
		else if (strstr(p,"ltank")) info = "CnC_Nod_Light_Tank";
		else if (strstr(p,"ftank")) info = "CnC_Nod_Flame_Tank";
		else if (strstr(p,"stank")) info = "CnC_Nod_Stealth_Tank";
		else if (strstr(p,"ssm")) info = "Nod_SSM_Launcher_Player";
		else if (strstr(p,"reconbike")) info = "CnC_Nod_Recon_Bike";
		else if (strstr(p,"chameleon")) info = "Nod_Chameleon";
		else if (strstr(p,"nodharvester")) info = "CnC_Nod_Harvester";
		else if (strstr(p,"truck")) info = "CnC_Civilian_Pickup01_Secret";
		else if (strstr(p,"sedan")) info = "CnC_Civilian_Sedan01_Secret";
	} else if (!stricmp(Area,"char")) {
		if (strstr(p,"gdisoldier")) info = "CnC_GDI_MiniGunner_0";
		else if (strstr(p,"gdishotgun")) info = "CnC_GDI_RocketSoldier_0";
		else if (strstr(p,"grenadier")) info = "CnC_GDI_Grenadier_0";
		else if (strstr(p,"gdiengineer")) info = "CnC_GDI_Engineer_0";
		else if (strstr(p,"gdiofficer")) info = "CnC_GDI_MiniGunner_1Off";
		else if (strstr(p,"gdirocketsoldier")) info = "CnC_GDI_RocketSoldier_1Off";
		else if (strstr(p,"sydney")) info = "CnC_Sydney";
		else if (strstr(p,"deadeye")) info = "CnC_GDI_MiniGunner_2SF";
		else if (strstr(p,"gunner")) info = "CnC_GDI_RocketSoldier_2SF";
		else if (strstr(p,"patch")) info = "CnC_GDI_Grenadier_2SF";
		else if (strstr(p,"havoc2")) info = "CnC_GDI_MiniGunner_3Boss_ALT2";
		else if (strstr(p,"havoc3")) info = "CnC_GDI_MiniGunner_3Boss_ALT3";
		else if (strstr(p,"havoc4")) info = "CnC_GDI_MiniGunner_3Boss_ALT4";
		else if (strstr(p,"havoc")) info = "CnC_GDI_MiniGunner_3Boss";
		else if (strstr(p,"pic2")) info = "CnC_Sydney_PowerSuit_ALT2";
		else if (strstr(p,"pic")) info = "CnC_Sydney_PowerSuit";
		else if (strstr(p,"mobius2")) info = "CnC_Ignatio_Mobius_ALT2";
		else if (strstr(p,"mobius")) info = "CnC_Ignatio_Mobius";
		else if (strstr(p,"hotwire")) info = "CnC_GDI_Engineer_2SF";
		else if (strstr(p,"nodsoldier")) info = "CnC_Nod_Minigunner_0";
		else if (strstr(p,"nodshotgun")) info = "CnC_Nod_RocketSoldier_0";
		else if (strstr(p,"flamethrower")) info = "CnC_Nod_FlameThrower_0";
		else if (strstr(p,"nodengineer")) info = "CnC_Nod_Engineer_0";
		else if (strstr(p,"nodofficer")) info = "CnC_Nod_Minigunner_1Off";
		else if (strstr(p,"nodrocketsoldier")) info = "CnC_Nod_RocketSoldier_1Off";
		else if (strstr(p,"chemwarrior")) info = "CnC_Nod_FlameThrower_1Off";
		else if (strstr(p,"blackhandsniper")) info = "CnC_Nod_Minigunner_2SF";
		else if (strstr(p,"laserchaingunner")) info = "CnC_Nod_RocketSoldier_2SF";
		else if (strstr(p,"sbh")) info = "CnC_Nod_FlameThrower_2SF";
		else if (strstr(p,"sakura2")) info = "CnC_Nod_MiniGunner_3Boss_ALT2";
		else if (strstr(p,"sakura")) info = "CnC_Nod_MiniGunner_3Boss";
		else if (strstr(p,"raveshaw2")) info = "CnC_Nod_RocketSoldier_3Boss_ALT2";
		else if (strstr(p,"raveshaw")) info = "CnC_Nod_RocketSoldier_3Boss";
		else if (strstr(p,"mendoza2")) info = "CnC_Nod_FlameThrower_3Boss_ALT2";
		else if (strstr(p,"mendoza")) info = "CnC_Nod_FlameThrower_3Boss";
		else if (strstr(p,"tech")) info = "CnC_Nod_Technician_0";
		else if (strstr(p,"initiate")) info = "CnC_Nod_Mutant_0_Mutant";
		else if (strstr(p,"acolyte")) info = "CnC_Nod_Mutant_1Off_Acolyte";
		else if (strstr(p,"templar")) info = "CnC_Nod_Mutant_2SF_Templar";
		else if (strstr(p,"kane")) info = "Nod_Kane";
		else if (strstr(p,"petrova2")) info = "Mutant_3Boss_Petrova";
		else if (strstr(p,"petrova")) info = "Nod_Scientist";
		else if (strstr(p,"logan")) info = "GDI_Logan_Sheppard";
		else if (strstr(p,"chicken")) info = "CnC_Chicken";
	} else if (!stricmp(Area,"weapons")) {
		if (strstr(p,"gdiautorifle")) info = "POW_AutoRifle_Player";
		else if (strstr(p,"nodautorifle")) info = "POW_AutoRifle_Player_Nod";
		else if (strstr(p,"gdichaingun")) info = "POW_Chaingun_Player";
		else if (strstr(p,"nodchaingun")) info = "POW_Chaingun_Player_Nod";
		else if (strstr(p,"chemsprayer")) info = "POW_ChemSprayer_Player";
		else if (strstr(p,"flamethrower")) info = "POW_Flamethrower_Player";
		else if (strstr(p,"grenadelauncher")) info = "POW_GrenadeLauncher_Player";
		else if (strstr(p,"10secion")) info = "POW_IonCannonBeacon_Player";
		else if (strstr(p,"ion")) info = "CnC_POW_IonCannonBeacon_Player";
		else if (strstr(p,"laserchaingun")) info = "POW_LaserChaingun_Player";
		else if (strstr(p,"laserrifle")) info = "POW_LaserRifle_Player";
		else if (strstr(p,"proxy")) info = "CnC_MineProximity_05";
		else if (strstr(p,"remotec42")) info = "CnC_POW_MineRemote_02";
		else if (strstr(p,"remotec41")) info = "CnC_POW_MineRemote_01";
		else if (strstr(p,"remotec4")) info = "POW_MineRemote_Player";
		else if (strstr(p,"timedc42")) info = "CnC_POW_MineTimed_Player_02";
		else if (strstr(p,"timedc41")) info = "CnC_POW_MineTimed_Player_01";
		else if (strstr(p,"timedc4")) info = "POW_MineTimed_Player";
		else if (strstr(p,"10secnuke")) info = "POW_Nuclear_Missle_Beacon";
		else if (strstr(p,"nuke")) info = "CnC_POW_Nuclear_Missle_Beacon";
		else if (strstr(p,"pic")) info = "POW_PersonalIonCannon_Player";
		else if (strstr(p,"pistol")) info = "POW_Pistol_Player";
		else if (strstr(p,"railgun")) info = "POW_Railgun_Player";
		else if (strstr(p,"ramjet")) info = "POW_RamjetRifle_Player";
		else if (strstr(p,"repairgunstrong")) info = "CnC_POW_RepairGun_Player";
		else if (strstr(p,"repairgun")) info = "POW_RepairGun_Player";
		else if (strstr(p,"rocketlauncherstrong")) info = "POW_RocketLauncher_Player";
		else if (strstr(p,"rocketlauncher")) info = "CnC_POW_RocketLauncher_Player";
		else if (strstr(p,"shotgun")) info = "POW_Shotgun_Player";
		else if (strstr(p,"gdisniperrifle")) info = "POW_SniperRifle_Player";
		else if (strstr(p,"nodsniperrifle")) info = "POW_SniperRifle_Player_Nod";
		else if (strstr(p,"tibautorifle")) info = "POW_TiberiumAutoRifle_Player";
		else if (strstr(p,"tibflech")) info = "POW_TiberiumFlechetteGun_Player";
		else if (strstr(p,"gdivolt")) info = "POW_VoltAutoRifle_Player";
		else if (strstr(p,"nodvolt")) info = "CnC_POW_VoltAutoRifle_Player_Nod";
		else if (strstr(p,"ubergun")) info = "POW_UltimateWeapon";
		else if (strstr(p,"ammo")) info = "CnC_POW_Ammo_ClipMax";
	} else if (!stricmp(Area,"obj")) {
		if (strstr(p,"Vehblock")) info = "Vehicle_Blocker";
		else if (strstr(p,"invobj")) info = "Invisible_Object";
	} else if (!stricmp(Area,"bldg")) {
		if (strstr(p,"nodobe")) info = "mp_Nod_Obelisk";
		else if (strstr(p,"nodhan")) info = "mp_Hand_of_Nod";
		else if (strstr(p,"nodair")) info = "mp_Nod_Airstrip";
		else if (strstr(p,"nodpow")) info = "mp_Nod_Power_Plant";
		else if (strstr(p,"nodref")) info = "mp_Nod_Refinery";
		else if (strstr(p,"gdiagt")) info = "mp_GDI_Advanced_Guard_Tower";
		else if (strstr(p,"gdibar")) info = "mp_GDI_Barracks";
		else if (strstr(p,"gdiwep")) info = "mp_GDI_War_Factory";
		else if (strstr(p,"gdipow")) info = "mp_GDI_Power_Plant";
		else if (strstr(p,"gdiref")) info = "mp_GDI_Refinery";
	} else if (!stricmp(Area,"model")) {
	}
	return info;
}
