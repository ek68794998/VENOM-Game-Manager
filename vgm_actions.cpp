#include "vgm_player.h"
#include "vgm_actions.h"

void Building_Status() {
	bool Gfirst = true,Nfirst = true;
	char G[1024] = "",N[1024] = "";
	const char *Gtemp = "",*Ntemp = "";
	for (GenericSLNode *x = BuildingGameObjList->HeadNode; x; x = x->NodeNext) {
		GameObject *o = As_BuildingGameObj((GameObject *)x->NodeData);
		if (!o) { continue; }
		const char *preset = Get_Preset_Name(o);
		int team = Get_Object_Type(o);
		if (team == 1) {
			if (Gfirst) {
				if (Commands->Get_Health(o) > 0.0f) { sprintf(G,"^%s Building Status^: %s(^%.0f^/^%.0f^) -",Get_Translated_Team_Name(team).c_str(),Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
				else { sprintf(G,"^%s Building Status^: %s(^Destroyed^) -",Get_Translated_Team_Name(team).c_str(),Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
			} else {
				if (Commands->Get_Health(o) > 0.0f) { sprintf(G,"%s %s(^%.0f^/^%.0f^) -",Gtemp,Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
				else { sprintf(G,"%s %s(^Destroyed^) -",Gtemp,Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
			}
			Gfirst = false;
			Gtemp = (const char *)G;
		} else if (team == 0) {
			if (Nfirst) {
				if (Commands->Get_Health(o) > 0.0f) { sprintf(N,"^%s Building Status^: %s(^%.0f^/^%.0f^) -",Get_Translated_Team_Name(team).c_str(),Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
				else { sprintf(N,"^%s Building Status^: %s(^Destroyed^) -",Get_Translated_Team_Name(team).c_str(),Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
			} else {
				if (Commands->Get_Health(o) > 0.0f) { sprintf(N,"%s %s(^%.0f^/^%.0f^) -",Ntemp,Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
				else { sprintf(N,"%s %s(^Destroyed^) -",Ntemp,Get_Pretty_Name(preset).c_str(),Commands->Get_Health(o),Commands->Get_Max_Health(o)); }
			}
			Nfirst = false;
			Ntemp = (const char *)N;
		}
	}
	vLogger->Log(vLoggerType::vVGM,"_BUILDINGS",G);
	vLogger->Log(vLoggerType::vVGM,"_BUILDINGS",N);
}
void Do_Damage(Stewie_BaseGameObj *obj, float Damage, const char* Warhead, Stewie_BaseGameObj *Damager) {
	//Stewie_OffenseObjectClass& Offense = Stewie_OffenseObjectClass::Create(); // Needs change, this CTOR is based on a reference.
	//Offense.damage = Damage;
	//Offense.warhead = (int)Stewie_ArmorWarheadManager::Get_Warhead_Type(Warhead);
	//Offense.dmgr = ReferencerClass(
	Commands->Apply_Damage((GameObject *)obj,Damage,Warhead,(GameObject *)Damager);
}
void Damage_All_Soldiers_Area_Team(float Damage, const char *Warhead, Vector3 Position, float Distance, GameObject *Host, GameObject *Damager, int Team) {
	if (!Commands->Get_ID(Host) || !Host) { return; }
	Vector3 TestPosition = Position;
	TestPosition.Z = 0;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		GameObject *obj = (GameObject *)Node->NodeData;
		if (obj && As_ScriptableGameObj(obj)) {
			Vector3 ObjPosition = Commands->Get_Position(obj);
			ObjPosition.Z = 0;
			if (Is_Soldier(obj) && (Commands->Get_Distance(ObjPosition,TestPosition) <= Distance) && (Commands->Get_ID(obj) != Commands->Get_ID(Host)) && (Get_Object_Type(obj) == Team || Team == 2)) {
				Do_Damage((Stewie_BaseGameObj *)obj,Damage,Warhead,(Stewie_BaseGameObj *)Damager);
			}
		}
	}
}
void Give_Weapon(int ID, const char *preset) {
	vWeaponManager::Give_Weapon(ID,preset);
	Powerup(Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID),"CnC_POW_Ammo_ClipMax",false);
}
void Give_All_Weapons(int ID, bool AllDefs) {
	if (AllDefs) {
		Stewie_SoldierGameObj *s = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
		if (s) { s->Give_All_Weapons(); }
	}
	vWeaponManager::Give_Weapon(ID,"POW_AutoRifle_Player");
	vWeaponManager::Give_Weapon(ID,"POW_AutoRifle_Player_Nod");
	vWeaponManager::Give_Weapon(ID,"POW_Chaingun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_Chaingun_Player_Nod");
	vWeaponManager::Give_Weapon(ID,"POW_ChemSprayer_Player");
	vWeaponManager::Give_Weapon(ID,"POW_Flamethrower_Player");
	vWeaponManager::Give_Weapon(ID,"POW_GrenadeLauncher_Player");
	vWeaponManager::Give_Weapon(ID,"POW_LaserChaingun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_LaserRifle_Player");
	vWeaponManager::Give_Weapon(ID,"CnC_MineProximity_05");
	vWeaponManager::Give_Weapon(ID,"POW_PersonalIonCannon_Player");
	vWeaponManager::Give_Weapon(ID,"POW_Pistol_Player");
	vWeaponManager::Give_Weapon(ID,"POW_Railgun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_RamjetRifle_Player");
	vWeaponManager::Give_Weapon(ID,"POW_RepairGun_Player");
	vWeaponManager::Give_Weapon(ID,"CnC_POW_RepairGun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_RocketLauncher_Player");
	vWeaponManager::Give_Weapon(ID,"CnC_POW_RocketLauncher_Player");
	vWeaponManager::Give_Weapon(ID,"POW_Shotgun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_SniperRifle_Player");
	vWeaponManager::Give_Weapon(ID,"POW_SniperRifle_Player_Nod");
	vWeaponManager::Give_Weapon(ID,"POW_TiberiumAutoRifle_Player");
	vWeaponManager::Give_Weapon(ID,"POW_TiberiumFlechetteGun_Player");
	vWeaponManager::Give_Weapon(ID,"POW_VoltAutoRifle_Player");
	vWeaponManager::Give_Weapon(ID,"CnC_POW_VoltAutoRifle_Player_Nod");
	vWeaponManager::Give_Weapon(ID,"CnC_POW_Ammo_ClipMax");
}
void Godify(Stewie_SoldierGameObj *obj) {
	const char *W = "Weapon_VoltAutoRifle_Player_Nod";
	if (obj->PlayerType == 1) {
		Set_Character(obj,"CnC_GDI_RocketSoldier_2SF_Secret");
		W = "Weapon_VoltAutoRifle_Player";
	} else {
		Set_Character(obj,"CnC_Nod_RocketSoldier_3Boss_Secret");
	}

	Powerup(obj,"CnC_POW_MineRemote_02",true);
	Powerup(obj,"CnC_POW_MineTimed_Player_02",true);
	Powerup(obj,"CnC_POW_MineProximity_05",true);
	Powerup(obj,"POW_Pistol_Player",true);
	Stewie_WeaponBagClass *bag = Get_Weapon_Bag(obj->As_ArmedGameObj());
	if (bag) {
		bag->Clear_Weapons();
		Stewie_WeaponDefinitionClass* Def = Stewie_WeaponManager::Find_Weapon_Definition(W);
		int bullets = Def->DSClipSize.Get() + Def->DSMaxClipBullets.Get();
		if (Def->DSMaxClipBullets.Get() == -1) { bullets = -1; }
		bag->Add_Weapon(Def,bullets,true);
		bag->Select_Index(1);
	}
	Grant_Refill((GameObject *)obj);

	float MH = obj->Defence.Get_Health_Max(), MSS = obj->Defence.Get_Shield_Strength_Max();
	Stewie_SoldierGameObjDef *SDef = (Stewie_SoldierGameObjDef *)obj->Definition;
	if (SDef) {
		MH = (MH / SDef->HealthMax.Get()) * 500.0f;
		MSS = (MSS / SDef->ShieldStrengthMax.Get()) * 500.0f;
	} else {
		MH = 500.0f;
		MSS = 500.0f;
	}
	obj->Defence.Set_Health_Max(MH);
	obj->Defence.Set_Shield_Strength_Max(MSS);
	obj->Defence.Set_Shield_Type(Stewie_ArmorWarheadManager::Get_Armor_Type("SkinChemWarrior"));
	Script_Attach(obj,"IAmGod","");

	Script_Remove(obj,"SelfRepair");
	char params[64];
	sprintf(params,"%f,%f",1.0f,1.0f);
	Script_Attach(obj,"SelfRepair",params);
	if (Config->Sounds) { Create_2D_WAV_Sound("m00gemg_atoc0001i1gemg_snd.wav"); }
}
bool Is_Spectating(Stewie_BaseGameObj *obj) {
	Stewie_ScriptableGameObj *AsScriptable = obj->As_ScriptableGameObj();
	if (AsScriptable && AsScriptable->As_SoldierGameObj()) {
		Stewie_PhysicalGameObj *AsPhys = obj->As_PhysicalGameObj();
		if (AsPhys && AsPhys->Phys && !stricmp(AsPhys->Phys->Model->Get_Name(),"NULL")) { return true; }
		vPlayer *player = vPManager->Get_Player(Player_ID_From_GameObj(obj));
		if (player && player->qspec) { return true; }
	}
	return false;
}
bool Is_Spectating(GameObject *o) {
	return Is_Spectating((Stewie_BaseGameObj *)o);
}
void Kill_Player(int ID) {
	GameObject *obj = Get_Vehicle_Return(Player_GameObj(ID));
	Do_Damage((Stewie_BaseGameObj *)obj,99999.0f,"Death",0);
}
void Kill_Player(GameObject *o) {
	Kill_Player(Get_Player_ID(o));
}
void Move_Player(int ID, float X, float Y, float Z, bool relative, bool vehicle) {
	Stewie_SoldierGameObj *s = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
	Stewie_VehicleGameObj *v = NULL;

	if (!s) { return; }
	if (vehicle) { v = s->VehicleOccupied; }
	Stewie_PhysicalGameObj *obj = s->As_PhysicalGameObj();
	if (v) { obj = v->As_PhysicalGameObj(); }

	Stewie_Vector3 pos;
	obj->Get_Position(&pos);
	if (relative) {
		pos.X += X;
		pos.Y += Y;
		pos.Z += Z;
	} else {
		pos.X = X;
		pos.Y = Y;
		pos.Z = Z;
	}
	obj->Set_Position(pos);
}
void Set_Speed(Stewie_SoldierGameObj *obj, float spd) {
	if (!obj) { return; }
	if (spd < 1.0f || spd > 100.0f) { return; }
	obj->Set_Speed(spd);
}
bool Spectate_Player(int ID, bool quiet) {
	if (Get_Vehicle(Player_GameObj(ID))) { return false; }
	bool spec = Is_Spectating(Player_GameObj(ID));
	if (spec) {
		Move_Player(ID,0,0,-5000,false);
		vPlayer *p = vPManager->Get_Player(ID);
		if (p) { p->qspec = false; }
		Inc_Deaths(Stewie_cPlayerManager::Find_Player(ID),-1);
	} else {
		Stewie_SoldierGameObj *s = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
		if (!s) { return false; }
		s->Toggle_Fly_Mode();
		Set_Model(s,"NULL");
		Set_Is_Visible(s,false);
		if (quiet) {
			s->DeletePending = true;
			for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
				Stewie_cPlayer *p = Node->NodeData;
				if (!p) { continue; }
				if (!p->IsActive || !p->IsInGame) { continue; }
				if (p->PlayerId == ID) { continue; }
				Stewie_cNetwork::Send_Object_Update(s,p->PlayerId);
			}
			s->DeletePending = false;
			vPlayer *p = vPManager->Get_Player(ID);
			if (p) { p->qspec = true; }
		}
		Stewie_WeaponBagClass *wb = Get_Weapon_Bag(s->As_ArmedGameObj());
		wb->Deselect();
		wb->Clear_Weapons();
		return true;
	}
	return false;
}
void Toggle_Fly_Mode(Stewie_SoldierGameObj *obj) {
	if (!obj) { return; }
	obj->Toggle_Fly_Mode();
	obj->As_PhysicalGameObj()->Set_Collision_Group(1);
}
