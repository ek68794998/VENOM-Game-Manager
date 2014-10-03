#include "vgm_crate.h"

bool CrateExists = false;
int LastCratePickup = -181;
unsigned int CrateID = 0;

char *SoldierCrates[32] = {
	{ "Ammo" },
	{ "Armor" },
	{ "Blamo" },
	{ "C4Limit" },
	{ "Character" },
	{ "CrazyDefense" },
	{ "Death" },
	{ "EnemyThief" },
	{ "EMP" },
	{ "Fly" },
	{ "Frozen" },
	{ "GG" },
	{ "God" },
	{ "HarvDump" },
	{ "Health" },
	{ "Hover" },
	{ "Invincibility" },
	{ "Kamikaze" },
	{ "Money" },
	{ "MoneyMult" },
	{ "NoC4" },
	{ "Points" },
	{ "Power" },
	{ "Radar" },
	{ "RandNuke" },
	{ "Speed" },
	{ "Spy" },
	{ "Thief" },
	{ "TibSuit" },
	{ "VehicleLimit" },
	{ "Weapon" },
	{ "Weapons" }
};
char *VehicleCrates[7] = {
	{ "AntiC4" },
	{ "AntiEnemy" },
	{ "Death" },
	{ "Health" },
	{ "Locked" },
	{ "SelfRepair" },
	{ "Spy" }
};
char *SBHCrates[2] = {
	{ "Death" },
	{ "Thief" }
};

void Crate_Init() {
	LastCratePickup = -181;
	CrateExists = false;
}
void Place_Crate_Randomly(Stewie_ScriptableGameObj *obj) {
	if (Config->CrateSpawnTotal <= 0) { return; }
	int Rand = Random(0,Config->CrateSpawnTotal,true);
	obj->As_PhysicalGameObj()->Set_Position(Config->CrateSpawnPositions[Rand]);
}
std::string Pick_Random_Crate(int type) {
	int total = 0;
	int maxcount = 0;
	const char *ctype = "";
	char **clist;
	if (type == 1) {
		ctype = "VehicleCrates";
		clist = VehicleCrates;
		maxcount = _countof(VehicleCrates);
	} else if (type == 2) {
		ctype = "SoldierCrates";
		clist = SoldierCrates;
		maxcount = _countof(SoldierCrates);
	} else if (type == 3) {
		ctype = "SBHCrates";
		clist = SBHCrates;
		maxcount = _countof(SBHCrates);
	} else { return "None"; }

	for (int i = 0; i < maxcount; i++) {
		total += getProfileInt(ctype,clist[i],0,"vgm_crates.ini");
	}
	int rand = Random(0,total,true);
	int active = 0;
	std::string Retn = "None";
	for (int i = 0; i < maxcount; i++) {
		int lower = getProfileInt(ctype,clist[i],0,"vgm_crates.ini");
		if (rand >= (int)(active) && rand < (int)(lower + active)) {
			Retn = (const char *)(clist[i]);
			return Retn;
		}
		active += lower;
	}
	return Retn;
}

void Crate_Creator::Created(Stewie_ScriptableGameObj *obj) {
	obj->Start_Observer_Timer(this->ID,(float)(Random(15,30)),900001);
}
void Crate_Creator::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 900001) {
		if (Random(0,3) == 2) { Create_Object("CnC_Ammo_Crate"); }
		else { Create_Object("CnC_Money_Crate"); }
	}
}

void M00_CNC_Crate::Created(Stewie_ScriptableGameObj *obj) {
	if (Config->Crates) { Destroy_Script(); }
}

void vCrateHandler::Created(Stewie_ScriptableGameObj *obj) {
	if (Config->Crates == false) { Script_Remove(obj,"vCrateHandler"); }
	Script_Remove_Duplicate(obj,"vCrateHandler");
	Stewie_Vector3 pos;
	if (Config->GameMode == Config->vSNIPER) {
		obj->Set_Delete_Pending();
	} else if (CrateID != obj->NetworkID && stricmp(obj->Definition->Get_Name(),"POW_Uplink")) {
		Stewie_PhysicalGameObj *AsPhys = obj->As_PhysicalGameObj();
		if (!AsPhys) { return; }
		AsPhys->Phys->Set_Model_By_Name("vehcol2m");
		Place_Crate_Randomly(obj);
		if (CrateExists && vManager.DurationAsInt() < 5) {
			obj->Get_Position(&pos);
			vLogger->Log(vLoggerType::vVGM,"_CRATE","Created: %.2f %.2f %.2f",pos.X,pos.Y,pos.Z);
			Stewie_PowerupGameObjDef *def = (Stewie_PowerupGameObjDef *)(AsPhys->As_PowerupGameObj()->Definition);
			def->IsPersistant = true;
			CrateID = obj->NetworkID;
		} else if (CrateExists || (vManager.DurationAsInt() - LastCratePickup) < 15) {
			obj->Set_Delete_Pending();
		} else {
			CrateExists = true;
			obj->Get_Position(&pos);
			vLogger->Log(vLoggerType::vVGM,"_CRATE","Created: %.2f %.2f %.2f",pos.X,pos.Y,pos.Z);
			Stewie_PowerupGameObjDef *def = (Stewie_PowerupGameObjDef *)(AsPhys->As_PowerupGameObj()->Definition);
			def->IsPersistant = true;
			CrateID = obj->NetworkID;
		}
	}
}
void vCrateHandler::Custom(Stewie_ScriptableGameObj *obj, int message, int param, Stewie_ScriptableGameObj *sender) {
	if (message == 1000000026 && PickedUp != true) {
		if (sender->As_VehicleGameObj()) {
			if (!Vehicle_Driver(sender->As_VehicleGameObj())) { return; }
			else { sender = Vehicle_Driver(sender->As_VehicleGameObj()); }
		}
		if (!sender || !sender->As_SoldierGameObj()) { return; }

		Create_2D_WAV_Sound_Player((GameObject *)sender,"powerup_ammo.wav");
		if (stricmp(obj->Definition->Get_Name(),"POW_Uplink")) {
			CrateExists = false;
			LastCratePickup = vManager.DurationAsInt();
			Stewie_ScriptableGameObj *inv = Create_Object("Invisible_Object")->As_ScriptableGameObj();
			inv->As_PhysicalGameObj()->Set_Position(World_Position(sender));
			Script_Attach(inv,"Crate_Creator","",true);
		}
		PickedUp = true;
		char CrateTypeRaw[64];
		const char *CrateType;
		Stewie_ScriptableGameObj *o = (sender->As_SoldierGameObj()->VehicleOccupied ? sender->As_SoldierGameObj()->VehicleOccupied->As_ScriptableGameObj() : sender);
		int ID = Player_ID_From_GameObj(sender);
		int MyTeam = sender->As_SmartGameObj()->PlayerType;
		Stewie_Vector3 pos = World_Position(obj);
		std::string pname = Player_Name_From_ID(ID);

		bool SBHGetSoldier = false;
		int Rand = Random(1,100,false);
		if (Rand < (int)Config->SBHGetGoodCrates) { SBHGetSoldier = true; }

		if (o->As_VehicleGameObj()) {
			sprintf(CrateTypeRaw,"%s",Pick_Random_Crate(1).c_str());
			CrateType = (const char *)CrateTypeRaw;
			vLogger->Log(vLoggerType::vGAMELOG,"_NULL","VEHCRATE;%s;%d;%s;%f;%f;%f;%f;%d",CrateType,o->NetworkID,sender->Definition->Get_Name(),pos.Y,pos.X,pos.Z,o->As_PhysicalGameObj()->Get_Facing(),MyTeam);

			if (!stricmp(CrateType,"AntiC4")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Anti-C4 Crate. Enemy C4 will vanish when placed on this vehicle!");
				Script_Attach(o,"AntiMine","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Vehicle Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"AntiEnemy")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Anti-Enemy Crate. Any Enemy trying to enter your vehicle will be killed!");
				char params[16];
				sprintf(params,"%d",MyTeam);
				Script_Attach(o,"AntiEnemy",params,true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Vehicle Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Death")) {

				int rand = Random(1,5);
				if (rand >= 1 && rand < 4) {
					HostMessage("Crate: A %s just got annihilated by a Death Crate!",Get_Pretty_Name(o->Definition->Get_Name()).c_str());
					Do_Damage(o,2500.0f,"Shrapnel");
					Commands->Create_Explosion("Explosion_Mine_Proximity_01",Commands->Get_Position((GameObject *)o),0);
				} else if (rand == 4) {
					HostMessage("Crate: A %s just got disintegrated by an Ion Cannon!",Get_Pretty_Name(o->Definition->Get_Name()).c_str());
					Damage_All_Soldiers_Area_Team(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)obj,0,2);
					Damage_All_Vehicles_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)obj,0);
					Create_Object("Beacon_Ion_Cannon_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_IonCannonBeacon",Commands->Get_Position((GameObject *)o),0);
				} else if (rand == 5) {
					HostMessage("Crate: A %s just got obliterated by a Nuclear Strike!",Get_Pretty_Name(o->Definition->Get_Name()).c_str());
					Damage_All_Soldiers_Area_Team(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)obj,0,2);
					Damage_All_Vehicles_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)obj,0);
					Create_Object("Beacon_Nuke_Strike_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_NukeBeacon",Commands->Get_Position((GameObject *)o),0);
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Health")) {

				int rand = Random(5,25);
				int positive = Random(0,1);
				float HP = o->As_DamageableGameObj()->Defence.Get_Health();
				if (positive == 1) {
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Dehealth Crate. Your vehicle now has %d less health.",rand);
					rand = -1 * rand;
				} else {
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Health Crate. Your vehicle now has %d more health!",rand);
				}
				float NewVal = (HP + float(rand) >= 1.0f) ? (HP + float(rand)) : 1.0f;
				o->As_DamageableGameObj()->Defence.Set_Health(NewVal);
				o->As_DamageableGameObj()->Defence.Set_Health_Max(o->As_DamageableGameObj()->Defence.Get_Health_Max() + float(rand));

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Amt: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,rand);

			} else if (!stricmp(CrateType,"Locked")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Lock Crate. No one can enter or exit your vehicle, including you!");
				Script_Attach(o,"Locked","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Vehicle Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"SelfRepair")) {

				float rand = float(Random(1,4));
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] Experimental Nanobot Technology has been installed in your vehicle. It will now repair %.0f health every second.",rand);
				Script_Remove(o,"SelfRepair");
				char params[64];
				sprintf(params,"%f,%f",rand,1.0f);
				Script_Attach(o,"SelfRepair",params);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Vehicle Crate (%.2f %.2f %.2f). (Amt/Sec = %.0f)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,rand);

			} else if (!stricmp(CrateType,"Spy")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Vehicle Spy Crate. Base Defenses will ignore your vehicle until it is destroyed.");
				if (MyTeam == 1 || MyTeam == 0) {
					HostMessage("Crate: Some %s player's vehicle just got outfitted with Spy Technology! Watch out, %s...",Get_Translated_Team_Name(MyTeam).c_str(),Get_Translated_Team_Name(1 - MyTeam).c_str());
				} else {
					HostMessage("Crate: Some player's vehicle just got outfitted with Spy Technology! Watch out...");
				}
				Set_Is_Visible(o,false);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Vehicle Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			}
		} else if (!strcmp(o->Definition->Get_Name(),"CnC_Nod_FlameThrower_2SF") && !SBHGetSoldier) {
			sprintf(CrateTypeRaw,"%s",Pick_Random_Crate(3).c_str());
			CrateType = (const char *)CrateTypeRaw;
			vLogger->Log(vLoggerType::vGAMELOG,"_NULL","SBHCRATE;%s;%d;%s;%f;%f;%f;%f;%d",CrateType,o->NetworkID,sender->Definition->Get_Name(),pos.Y,pos.X,pos.Z,o->As_PhysicalGameObj()->Get_Facing(),MyTeam);

			if (!stricmp(CrateType,"Thief")) {

				Stewie_cPlayer *p = o->As_SoldierGameObj()->Player;
				if (!p) { return; }
				if (p->Money.Get() >= 0.0f) {
					p->Increment_Money(p->Money.Get() * -1);
				}
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Thief Crate. Enjoy the Stealth Black Hand present.");
				HostMessage("Crate: A poor Stealth Black Hand just lost all his dough!");
				if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00evag_dsgn0028i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) SBH %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Death")) {

				int rand = Random(1,5);
				if (rand >= 1 && rand < 4) {
					HostMessage("Crate: Some poor SBH just got roasted by a Death Crate!");
					Do_Damage(o,2500.0f,"Shrapnel");
					Commands->Create_Explosion("Explosion_Mine_Proximity_01",Commands->Get_Position((GameObject *)o),0);
				} else if (rand == 4) {
					Damage_All_Objects_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)o,0);
					Create_Object("Beacon_Ion_Cannon_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_IonCannonBeacon",Commands->Get_Position((GameObject *)o),0);
					HostMessage("Crate: Some poor SBH just got disintegrated by an Ion Cannon!");
				} else if (rand == 5) {
					Damage_All_Objects_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)o,0);
					Create_Object("Beacon_Nuke_Strike_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_NukeBeacon",Commands->Get_Position((GameObject *)o),0);
					HostMessage("Crate: Some poor SBH just got obliterated by a Nuclear Strike!");
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) SBH %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			}
		} else {
			sprintf(CrateTypeRaw,"%s",Pick_Random_Crate(2).c_str());
			CrateType = (const char *)CrateTypeRaw;

			if (EMPon && !stricmp(CrateType,"EMP")) { CrateType = "Death"; }
			else if (GGstarted && !stricmp(CrateType,"GG")) { CrateType = "Ammo"; }

			vLogger->Log(vLoggerType::vGAMELOG,"_NULL","CRATE;%s;%d;%s;%f;%f;%f;%f;%d",CrateType,o->NetworkID,sender->Definition->Get_Name(),pos.Y,pos.X,pos.Z,o->As_PhysicalGameObj()->Get_Facing(),MyTeam);

			if (!stricmp(CrateType,"Ammo")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Ammo crate. You have had your ammo refilled.");
				Powerup(sender->As_SoldierGameObj(),"CnC_POW_Ammo_ClipMax",true);
				if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00puar_aqob0002i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Armor")) {

				int rand = Random(5,50);
				int positive = Random(0,1);
				Stewie_DamageableGameObj *Damaged = o->As_DamageableGameObj();
				if (!Damaged) { return; }
				if (positive == 1) {
					rand = -1 * rand;
					Damaged->Defence.Set_Shield_Strength_Max(Damaged->Defence.Get_Shield_Strength_Max() + (float)rand);
					Damaged->Defence.Set_Shield_Strength(Damaged->Defence.Get_Shield_Strength() + (float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Dearmor Crate. You have lost %d max armor.",-1 * rand);
				} else {
					Damaged->Defence.Set_Shield_Strength_Max(Damaged->Defence.Get_Shield_Strength_Max() + (float)rand);
					Damaged->Defence.Set_Shield_Strength(Damaged->Defence.Get_Shield_Strength() + (float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Armor Crate. You have received %d max armor.",rand);
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Amt: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,(positive ? rand : -1 * rand));

			} else if (!stricmp(CrateType,"Blamo")) {

				//

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"C4Limit")) {

				int rand = Random(0,1);
				int oldlimit = rand == 1 ? Config->GDIProximityC4Limit : Config->NODProximityC4Limit;
				int newlimit = oldlimit;
				while (newlimit == oldlimit) {
					newlimit = Random(oldlimit - 25,oldlimit + 25);
				}
				if (newlimit < 1) { newlimit = 1; }
				else if (newlimit > 85) { newlimit = 85; }
				HostMessage("Crate: The Crate God decided to %s %s's Proximity C4 limit to %d...",oldlimit > newlimit ? "decrease" : "increase",Get_Translated_Team_Name(rand).c_str(),newlimit);
				if (rand == 1) { Config->GDIProximityC4Limit = newlimit; }
				else { Config->NODProximityC4Limit = newlimit; }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Character")) {

				FindChar:
					const char *ch = Find_Random(SoldierCID)->Get_Name();
					if (Get_Cost(ch) <= 0) { goto FindChar; }
				Set_Character(sender->As_SoldierGameObj(),ch);
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Random Character Crate. You have been given a(n) %s.",Get_Pretty_Name(ch).c_str());

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Character: %s)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,Get_Pretty_Name(ch).c_str());

			} else if (!stricmp(CrateType,"CrazyDefense")) {

				Attach_Script_Once(Find_Random_Building_By_Team(2,false),"Crazy_Defense","");

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Death")) {

				int rand = Random(1,5);
				if (rand >= 1 && rand < 4) {
					HostMessage("Crate: Someone just got roasted by a Death Crate!");
					Do_Damage(o,2500.0f,"Shrapnel");
					Commands->Create_Explosion("Explosion_Mine_Proximity_01",Commands->Get_Position((GameObject *)o),0);
				} else if (rand == 4) {
					HostMessage("Crate: Someone just got disintegrated by an Ion Cannon!");
					Damage_All_Objects_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)o,0);
					Create_Object("Beacon_Ion_Cannon_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_IonCannonBeacon",Commands->Get_Position((GameObject *)o),0);
				} else if (rand == 5) {
					HostMessage("Crate: Someone just got obliterated by a Nuclear Strike!");
					Damage_All_Objects_Area(2500.0f,"None",Commands->Get_Position((GameObject *)o),15.0f,(GameObject *)o,0);
					Create_Object("Beacon_Nuke_Strike_Anim_Post",World_Position(o));
					Commands->Create_Explosion("Explosion_NukeBeacon",Commands->Get_Position((GameObject *)o),0);
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"EMP")) {

				int rand = Random((int)getProfileFloat("EMPCrate","MinTime",5,"vgm_crates.ini"),(int)getProfileFloat("EMPCrate","MaxTime",30,"vgm_crates.ini"));
				HostMessage("Crate: Someone just picked up the EMP Crate. SBH suits are malfunctioning and players cannot enter vehicles for %d seconds.",rand);
				char params[512];
				sprintf(params,"%d",rand);
				Attach_Script_Once(Find_Random_Building_By_Team(2,false),"EMP",params);
				for (int i = 1; i <= GetMaxPlayerID(); i++) {
					GameObject *v = Get_Vehicle(Player_GameObj(i));
					if (Is_Soldier(Player_GameObj(i)) && v) {
						Force_Occupant_ID_Exit(v,i);
					}
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"EnemyThief")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got an Enemy Thief Crate! The next person you kill will lose half their money to you...");
				Script_Attach(sender,"EnemyThief","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Fly")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got a Fly Crate! Weeeeeee!");
				Script_Attach(sender,"Fly","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Frozen")) {

				//

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"GG")) {

				int rand = Random(0,1);
				GameObject *nObj = Find_Random_Building_By_Team(rand,true);
				int time = Random(getProfileInt("GGCrate","MinTime",40,"vgm_crates.ini"),getProfileInt("GGCrate","MaxTime",60,"vgm_crates.ini"));
				char params[512];
				sprintf(params,"%d",time);
				Attach_Script_Once(nObj,"GG",params);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Time: %dmins)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,time);

			} else if (!stricmp(CrateType,"God")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got a God Crate! You now have 500 health and armor, cannot be hurt by Tiberium, and heal constantly a little bit...");
				HostMessage("Crate: %s just picked up a God Crate...",pname.c_str());


				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"HarvDump")) {

				int rand = Random(getProfileInt("HarvDumpCrate","MinDump",100,"vgm_crates.ini"),getProfileInt("HarvDumpCrate","MaxDump",1000,"vgm_crates.ini"));
				int team = Random(0,1);
				if (team == 1) {
					Config->GDIHarvesterDump = rand;
					HostMessage("Crate: GDI will now be receiving %d credits per Harvester dump...",Config->GDIHarvesterDump);
				} else {
					Config->NodHarvesterDump = rand;
					HostMessage("Crate: Nod will now be receiving %d credits per Harvester dump...",Config->NodHarvesterDump);
				}
				Stewie_BaseControllerClass::Find_Base(team)->Find_Building(vRefinery)->As_RefineryGameObj()->TotalFunds = float(rand);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Health")) {

				int rand = Random(5,50);
				int positive = Random(0,1);
				Stewie_DamageableGameObj *Damageable = o->As_DamageableGameObj();
				if (!Damageable) { return; }
				if (positive == 1) {
					rand = -1 * rand;
					Damageable->Defence.Set_Health(Damageable->Defence.Get_Health() + (float)rand);
					Damageable->Defence.Set_Health_Max(Damageable->Defence.Get_Health_Max() + (float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Dehealth Crate. You have lost %d max health.",-1 * rand);
				} else {
					Damageable->Defence.Set_Health_Max(Damageable->Defence.Get_Health_Max() + (float)rand);
					Damageable->Defence.Set_Health(Damageable->Defence.Get_Health() + (float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Health Crate. You have received %d max health.",rand);
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Amt: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,(positive ? rand : -1 * rand));

			} else if (!stricmp(CrateType,"Hover")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got a Hover Crate! You can now walk over enemy proximity mines without detonating them.");
				Script_Attach(o,"Hover","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Invincibility")) {

				int rand = Random(10,40);
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Invincibility Crate! You cannot be damaged for %d seconds.",rand);
				HostMessage("Crate: %s just collected the Invincibility Crate! Better keep your eye out...",pname.c_str());
				char params[512];
				sprintf(params,"%d",rand);
				Script_Attach(o,"Invincibility",params,true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Time: %dsecs)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,rand);

			} else if (!stricmp(CrateType,"Kamikaze")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the KAMIKAZE! Crate. Upon death, you will generate a HUGE explosion!");
				int rand = Random(1,2);
				Set_Character(sender->As_SoldierGameObj(),"CnC_Nod_Flamethrower_0_Secret");
				char params[512];
				sprintf(params,"%d",rand);
				Script_Attach(sender,"Kamikaze",params,true);
				if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0027i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Money")) {

				int rand = Random((int)getProfileFloat("MoneyCrate","Min",50,"vgm_crates.ini"),(int)getProfileFloat("MoneyCrate","Max",1000,"vgm_crates.ini"));
				int positive = Random(0,1);
				Stewie_cPlayer *myP = Stewie_cPlayerManager::Find_Player(ID);
				if (!myP) { return; }
				if (positive == 1) {
					rand = -1 * rand;
					myP->Increment_Money(float(rand));
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Demoney Crate. You have lost %d credits.",-1 * rand);
				} else {
					myP->Increment_Money(float(rand));
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Money Crate. You have received %d credits.",rand);
					if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00pc$$_aqob0002i1evag_snd.wav"); }
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Amt: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,(positive ? rand : -1 * rand));

			} else if (!stricmp(CrateType,"MoneyMult")) {

				int rand = Random(2,4);
				Stewie_cPlayer *myP = Stewie_cPlayerManager::Find_Player(ID);
				if (myP) { myP->Increment_Money(myP->Money.Get() * float(rand - 1)); }
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Money Multiplier Crate. Your money has been multiplied by %d.",rand);
				if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00pc$$_aqob0002i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Multiplier: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,rand);

			} else if (!stricmp(CrateType,"NoC4")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got a No C4 Crate! You can not Detonate C4 until you die.");
				Script_Attach(o,"NoC4Allowed","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Points")) {

				int rand = Random((int)getProfileFloat("PointsCrate","Min",50,"vgm_crates.ini"),(int)getProfileFloat("PointsCrate","Max",700,"vgm_crates.ini"));
				int positive = Random(0,1);
				Stewie_cPlayer *myP = Stewie_cPlayerManager::Find_Player(ID);
				if (!myP) { return; }
				if (positive == 1) {
					rand = -1 * rand;
					myP->Increment_Score((float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Depoints Crate. You have lost %d points.",-1 * rand);
					HostMessage("[Crate] %s got the Points Crate and has lost %d points!",Player_Name_From_ID(ID).c_str(),-1 * rand);
				} else {
					myP->Increment_Score((float)rand);
					myP->Increment_Money(-1.0f * (float)rand);
					PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Points Crate. You have received %d points.",rand);
					HostMessage("[Crate] %s got the Points Crate and has received %d points for %s!",Player_Name_From_ID(ID).c_str(),rand,Get_Translated_Team_Name(MyTeam).c_str());
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Amt: %d)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,rand);

			} else if (!stricmp(CrateType,"Power")) {

				if (Find_Base_Defense(1) && Find_Base_Defense(0)) {
					int Team = Random(0,1);
					if (MyTeam == 1) { Team = 0; }
					else if (MyTeam == 0) { Team = 1; }
					int rand = Random(10,90);
					char params[512];
					sprintf(params,"%d",rand);
					Attach_Script_Once(Find_Soldier_Factory(Team),"Power_Down",params);
				}

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Radar")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got the Radar Crate! Your Radar has been disabled for 60secs.");
				Script_Attach(o,"Radar","",true);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"RandNuke")) {

				GameObject *nObj = Find_Random_Player_By_Team(2);
				HostMessage("Crate: Someone picked up the Random Nuke crate! Some random sucker's about to get nuked...");
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You will be nuked in a random amount of time... try to stay near enemy units!");
				Attach_Script_Once(nObj,"RandNuke","");

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Victim: %s)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,Player_Name_From_GameObj(nObj).c_str());

			} else if (!stricmp(CrateType,"Speed")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got the Speed crate! Your speed has been increased.");
				Set_Speed((Stewie_SoldierGameObj *)o,10);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Spy")) {

				Set_Character(sender->As_SoldierGameObj(),"CnC_Nod_FlameThrower_2SF");
				Set_Is_Visible(sender,false);
				if (MyTeam == 1 || MyTeam == 0) {
					HostMessage("Crate: Some %s player just got a Spy Crate! Better fire up the manual defenses, %s...",Get_Translated_Team_Name(MyTeam).c_str(),Get_Translated_Team_Name(1 - MyTeam).c_str());
				} else {
					HostMessage("Crate: Some player just got a Spy Crate! Better fire up the manual defenses...");
				}
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You have been given the Spy Crate. Base Defenses will ignore you until you die or change characters.");

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Thief")) {

				Stewie_cPlayer *myP = Stewie_cPlayerManager::Find_Player(ID);
				if (myP) { myP->Set_Money(0.0f); }
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Thief Crate. You have lost all your money.");
				HostMessage("Crate: Some poor hobo just lost all his dough!");
				if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00evag_dsgn0028i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"TibSuit")) {

				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You just got a Tiberium Suit Crate! You can no longer be hurt by Tiberium.");
				sender->As_DamageableGameObj()->Defence.Set_Shield_Type(Stewie_ArmorWarheadManager::Get_Armor_Type("SkinChemWarrior"));
				if (Config->Sounds) { Create_2D_WAV_Sound_Player((GameObject *)sender,"m00puts_aqob0002i1evag_snd.wav"); }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"VehicleLimit")) {

				int rand = Random(0,1);
				int oldlimit = rand == 1 ? Config->GDIVehicleLimit : Config->NodVehicleLimit;
				int newlimit = oldlimit;
				newlimit -= 1; // Discount Harvester for output/check
				while (newlimit == oldlimit) {
					newlimit = Random(oldlimit - 5,oldlimit + 5);
				}
				if (newlimit < 1) { newlimit = 1; }
				else if (newlimit > 12) { newlimit = 12; }
				HostMessage("Crate: The Crate God has set the decided to change %s's Vehicle Limit to %d...",Get_Translated_Team_Name(rand).c_str(),newlimit);
				newlimit += 1; // Add Harvester back in
				if (rand == 1) { Config->GDIVehicleLimit = newlimit; }
				else { Config->NodVehicleLimit = newlimit; }

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else if (!stricmp(CrateType,"Weapon")) {

				FindWep:
					Stewie_WeaponDefinitionClass *w = (Stewie_WeaponDefinitionClass *)Find_Random(WeaponCID);
					if (isin(w->Get_Name(),"beacon")) { goto FindWep; }
				vWeaponManager::Give_Weapon(ID,w->Get_Name());
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] You got the Weapon Crate. You have been given a(n) %s.",Get_Pretty_Name(w->Get_Name()).c_str());

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f). (Weapon: %s)",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z,Get_Pretty_Name(w->Get_Name()).c_str());

			} else if (!stricmp(CrateType,"Weapons")) {

				Give_All_Weapons(ID,false);
				PrivMsgColoredVA(ID,2,200,0,200,"[Crate] Jackpot! You have been given all weapons, including mass stockpiles of C4.");

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			} else {

				CrateType = "Death";

				HostMessage("Crate: Someone just got roasted by a Death Crate!");
				Do_Damage(o,2500.0f,"Shrapnel");
				Commands->Create_Explosion("Explosion_Mine_Proximity_01",Commands->Get_Position((GameObject *)o),0);

				vLogger->Log(vLoggerType::vVGM,"_CRATE","%s picked up a(n) %s Crate (%.2f %.2f %.2f).",pname.c_str(),CrateType,pos.X,pos.Y,pos.Z);

			}
		}

		obj->Set_Delete_Pending();
	}
}

void GG::Created(Stewie_ScriptableGameObj *obj) {
	GGstarted = true;
	obj->Start_Observer_Timer(this->ID,(float)((Get_Int_Parameter("time") - 25)*60),1);
	obj->Start_Observer_Timer(this->ID,(float)((Get_Int_Parameter("time") - 15)*60),2);
	obj->Start_Observer_Timer(this->ID,(float)((Get_Int_Parameter("time") - 10)*60),3);
	obj->Start_Observer_Timer(this->ID,(float)((Get_Int_Parameter("time") - 5)*60),4);
	obj->Start_Observer_Timer(this->ID,(float)((Get_Int_Parameter("time"))*60),5);
	AttackTeam = obj->As_SmartGameObj()->PlayerType;
	VictimTeam = (int)(1 - AttackTeam);
	AttackType = (AttackTeam == 1 ? "Ion Cannon Disaster" : "Nuclear Holocaust");
	char m[512];
	sprintf(m,"You have %d minutes to destroy the %s, or you will suffer %s.",Get_Int_Parameter("time"),Get_Pretty_Name(obj->Definition->Get_Name()).c_str(),AttackType);
	Send_Private_Message_Team(VictimTeam,m);
	if (VictimTeam == 1) { HostMessage("Crate: The Crate God has notified the Temple of Nod of GDI's large threat in the area. Their Nuclear Warheads are being prepared to launch in %d minutes...",Get_Int_Parameter("time")); }
	else { HostMessage("Crate: The Crate God has notified the Ion Control Center of Nod's large threat in the area. Their Ion Cannon Satellites are being prepared to activate in %d minutes...",Get_Int_Parameter("time")); }
}
void GG::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1) {
		HostMessage("Crate: %s has started preparing their %s. In 25 minutes, the %s begins...",Get_Translated_Team_Name(AttackTeam).c_str(),(AttackTeam == 1 ? "Ion Cannon Satellites" : "Nuclear Warheads"),AttackType);
	} else if (number == 2) {
		if (AttackTeam == 1) {
			HostMessage("Crate: The Ion Cannon Satellite is almost moved into position. Nod has only 15 minutes to stop it!");
		} else {
			HostMessage("Crate: The Nuclear Missiles are currently being primed to launch. GDI has only 15 minutes to stop it!");
		}
	} else if (number == 3) {
		if (AttackTeam == 1) {
			HostMessage("Crate: The Ion Cannon Satellite is in position and ready to fire. Only 10 minutes left, Nod!");
		} else {
			HostMessage("Crate: The Nuclear Missiles have been launched. GDI has only 10 minutes left to stop them.");
		}
	} else if (number == 4) {
		if (AttackTeam == 1) {
			HostMessage("Crate: The Ion Cannon Satellite is charging. It will be ready to fire in 5 minutes!");
		} else {
			HostMessage("Crate: The Nuclear Missiles are approaching fast, and will detonate over GDI in 5 minutes.");
		}
	} else if (number == 5) {
		HostMessage("Crate: %s was unable to destroy the transmitter, and now, %s is amongst us.",Get_Translated_Team_Name(VictimTeam).c_str(),AttackType);
		if (VictimTeam == 0) { Beacon_All_Buildings(0,1,true); }
		else if (VictimTeam == 1) { Beacon_All_Buildings(1,2,true); }
		obj->Start_Observer_Timer(this->ID,11.0f,6);
	} else if (number == 6) {
		if (VictimTeam == 0) { Beacon_All_Buildings(0,1,false); }
		else if (VictimTeam == 1) { Beacon_All_Buildings(1,2,false); }
		GGstarted = false;
		Script_Remove(obj,"GG");
	}
}
void GG::Killed(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *shooter) {
	HostMessage("Crate: %s destroyed %s's signal transmitter (%s) and the %s has been averted!",Get_Translated_Team_Name(VictimTeam).c_str(),Get_Translated_Team_Name(AttackTeam).c_str(),Get_Pretty_Name(obj->Definition->Get_Name()).c_str(),AttackType);
	GGstarted = false;
}

void Power_Down::Created(Stewie_ScriptableGameObj *obj) {
	if (!obj || !obj->As_SmartGameObj() || !obj->As_BuildingGameObj()) { return; }
	int team = obj->As_SmartGameObj()->PlayerType;
	if (Commands->Get_Building_Power(Find_Base_Defense(team))) {
		Commands->Set_Building_Power(Find_Base_Defense(team),false);
		if (Config->Sounds) {
			Create_2D_WAV_Sound("01-i004e.wav");
			Create_2D_WAV_Sound("amb_powerdown.wav");
		}
		HostMessage("Crate: Someone just picked up the Power Crate. %s just lost power to their base defenses for %d seconds.",Get_Translated_Team_Name(team).c_str(),Get_Int_Parameter("time"));
		obj->Start_Observer_Timer(this->ID,(float)Get_Int_Parameter("time"),1);
	} else {
		Commands->Set_Building_Power(Find_Base_Defense(team),true);
		HostMessage("Crate: Someone just picked up the Power Crate. %s just restored power to their base defenses for %d seconds.",Get_Translated_Team_Name(team).c_str(),Get_Int_Parameter("time"));
		obj->Start_Observer_Timer(this->ID,(float)Get_Int_Parameter("time"),2);
		if (Config->Sounds) {
			Console_Input("snda m00bnpp_dsgn0001i1evan_snd.wav");
			if (team == 0) { Create_2D_WAV_Sound("m00evag_dsgn0024i1evag_snd.wav"); }
			else { Create_2D_WAV_Sound("m00evan_dsgn0020i1evan_snd.wav"); }
		}
	}
}
void Power_Down::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (!obj || !obj->As_SmartGameObj() || !obj->As_BuildingGameObj()) { return; }
	int team = obj->As_SmartGameObj()->PlayerType;
	if (number == 1) {
		Stewie_BuildingGameObj *Defense = Team_Base_Defense(team);
		Stewie_PowerPlantGameObj *PPlant = Team_Power_Plant(team);
		if (Defense && Defense->Defence.Get_Health() > 0.0f && (PPlant ? (!PPlant->IsDestroyed && PPlant->Defence.Get_Health() > 0.0f && PPlant->Base->IsPowered) : true)) {
			Defense->Enable_Power(true);
		}
		HostMessage("Crate: %s fixed their Power Plant. Base Defense Power is now back online.",Get_Translated_Team_Name(team).c_str());
		Script_Remove(obj,"Power_Down");
	} else if (number == 2) {
		Stewie_BuildingGameObj *Defense = Team_Base_Defense(team);
		Defense->Enable_Power(false);
		if (Config->Sounds) {
			Create_2D_WAV_Sound("01-i004e.wav");
			Create_2D_WAV_Sound("amb_powerdown.wav");
		}
		HostMessage("Crate: The %s Power Plant's extra reserves have run out. Base Defense Power is now offline again.",Get_Translated_Team_Name(team).c_str());
		Script_Remove(obj,"Power_Down");
	}
}

void Crazy_Defense::Created(Stewie_ScriptableGameObj *obj) {
	Create_2D_WAV_Sound("amb_firetruck1.wav");
	HostMessage("Crate: Someone just picked up the Crazy Defenses crate; all base defenses will attack friendly units starting in 5 seconds.");
	obj->Start_Observer_Timer(this->ID,5.0f,1400001);
}
void Crazy_Defense::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1400001) {
		int rand = Random(10,30);
		HostMessage("Crate: Crazy Defenses are now enabled; they will disable in %d seconds.",rand);
		obj->Start_Observer_Timer(this->ID,(float)rand,1400002);
		bool &FriendlyFire = *(bool *)0x000000; // For security and licensing purposes, an address has been hidden from this line
		FriendlyFire = true;
		Config->CrazyDefensesEnabled = true;
	} else if (number == 1400002) {
		bool &FriendlyFire = *(bool *)0x000000; // For security and licensing purposes, an address has been hidden from this line
		FriendlyFire = GameDataObj()->IsFriendlyFirePermitted;
		Config->CrazyDefensesEnabled = false;
		HostMessage("Crate: The time for Crazy Defenses has ended. Base defenses are back to normal.");
		Script_Remove(obj,"Crazy_Defense");
	}
}

void EMP::Created(Stewie_ScriptableGameObj *obj) {
	EMPon = true;
	obj->Start_Observer_Timer(this->ID,(float)Get_Int_Parameter("time"),1);
}
void EMP::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1) {
		HostMessage("Crate: This area is no longer affected by EMP. All vehicles are accessable and SBH Suits back to normal.");
		EMPon = false;
	}
}

void Invincibility::Created(Stewie_ScriptableGameObj *obj) {
	obj->Start_Observer_Timer(this->ID,(float)(Get_Int_Parameter("time") - 5),1300004);
	obj->Start_Observer_Timer(this->ID,(float)Get_Int_Parameter("time"),1300005);
}
void Invincibility::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1300004) {
		PrivMsgColoredVA(obj->As_SoldierGameObj()->Player->PlayerId,2,200,0,200,"[Crate] Your invincibility is about to expire!");
	} else if (number == 1300005) {
		PrivMsgColoredVA(obj->As_SoldierGameObj()->Player->PlayerId,2,200,0,200,"[Crate] Your invincibility has expired.");
		Script_Remove(obj,"Invincibility");
	}
}

void Fly::Created(Stewie_ScriptableGameObj *obj) {
	if (!obj || !obj->As_SoldierGameObj()) { return; }
	obj->As_SoldierGameObj()->Toggle_Fly_Mode();
	obj->Start_Observer_Timer(this->ID,15.0f,1300002);
	obj->Start_Observer_Timer(this->ID,20.0f,1300003);
}
void Fly::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1300002) {
		PrivMsgColoredVA(obj->As_SoldierGameObj()->Player->PlayerId,2,200,0,200,"[Crate] Your flying is about to expire!");
	} else if (number == 1300003) {
		PrivMsgColoredVA(obj->As_SoldierGameObj()->Player->PlayerId,2,200,0,200,"[Crate] Your flying has expired.");
		obj->As_SoldierGameObj()->Toggle_Fly_Mode();
		Script_Remove(obj,"Fly");
	}
}

void Radar::Created(Stewie_ScriptableGameObj *obj) {
	Enable_Radar_Player((GameObject *)obj,false);
	obj->Start_Observer_Timer(this->ID,60.0f,1300001);
}
void Radar::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	if (number == 1300001) {
		Enable_Radar_Player((GameObject *)obj,true);
	}
}

void RandNuke::Created(Stewie_ScriptableGameObj *obj) {
	obj->Start_Observer_Timer(this->ID,float(Random(5,60)),Random(1,2));
}
void RandNuke::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
	Stewie_Vector3 Position = World_Position(obj);
	Vector3 pos = Vector3(Position.X,Position.Y,Position.Z);
	Damage_All_Soldiers_Area_Team(2500.0f,"None",pos,15.0f,(GameObject *)obj,0,2);
	Damage_All_Vehicles_Area(2500.0f,"None",pos,15.0f,(GameObject *)obj,0);
	if (number == 1) {
		Create_Object("Beacon_Ion_Cannon_Anim_Post")->As_PhysicalGameObj()->Set_Position(Position);
		Commands->Create_Explosion("Explosion_IonCannonBeacon",pos,0);
	} else if (number == 2) {
		Create_Object("Beacon_Nuke_Strike_Anim_Post")->As_PhysicalGameObj()->Set_Position(Position);
		Commands->Create_Explosion("Explosion_NukeBeacon",pos,0);
	} else {
		return;
	}
	HostMessage("BOOM!");
	Script_Remove(obj,"RandNuke");
	Create_2D_WAV_Sound("m00bntu_kill0037i1gcp3_snd.wav");
}

STSCRIPT(M00_CNC_Crate,CrateRegistrant,"M00_CNC_Crate","");
STSCRIPT(vCrateHandler,CrateHandlerRegistrant,"vCrateHandler","");
STSCRIPT(Crate_Creator,CrateCreatorRegistrant,"Crate_Creator","");
STSCRIPT(Crazy_Defense,DefenseCrateRegistrant,"Crazy_Defense","");
STSCRIPT(Power_Down,PowerCrateRegistrant,"Power_Down","time:int");
STSCRIPT(Kamikaze,KamiCrateRegistrant,"Kamikaze","type:int");
STSCRIPT(GG,GGCrateRegistrant,"GG","");
STSCRIPT(AntiMine,MineCrateRegistrant,"","");
STSCRIPT(AntiEnemy,EnemyCrateRegistrant,"AntiEnemy","team:int");
STSCRIPT(Locked,LockCrateRegistrant,"Locked","");
STSCRIPT(Hover,HoverCrateRegistrant,"Hover","");
STSCRIPT(EnemyThief,EnemyThiefCrateRegistrant,"EnemyThief","");
STSCRIPT(NoC4Allowed,NoC4CrateRegistrant,"NoC4Allowed","");
STSCRIPT(Fly,FlyCrateRegistrant,"Fly","");
STSCRIPT(Radar,RadarCrateRegistrant,"Radar","");
STSCRIPT(RandNuke,RandNukeCrateRegistrant,"RandNuke","");
STSCRIPT(EMP,EMPCrateRegistrant,"EMP","time:int");
STSCRIPT(Invincibility,InvinceCrateRegistrant,"Invincibility","time:int");
