#include "vgm_weapon.h"

vWeaponManager *vWManager;

float __stdcall GetDmgMultiplier(Stewie_DefenseObjectClass& Defense, Stewie_OffenseObjectClass& Offense) {
	return Stewie_ArmorWarheadManager::Get_Damage_Multiplier(Defense.ShieldType.Get(),Offense.warhead);
}

void Beacon_All_Buildings(int Team, int type, bool preanim) {
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_BuildingGameObj((GameObject *)Node->NodeData);
		if (o && Is_Building(o)) {
			if (Get_Object_Type(o) == Team || Team == 2) {
				if (preanim) {
					if (type == 1) { Commands->Create_Object("Beacon_Ion_Cannon_Anim_Pre",Commands->Get_Position(o)); }
					else if (type == 2) { Commands->Create_Object("Beacon_Nuke_Strike_Anim_Pre",Commands->Get_Position(o)); }
				} else {
					if (type == 1) {
						Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",Commands->Get_Position(o));
						Commands->Create_Explosion("Explosion_IonCannonBeacon",Commands->Get_Position(o),0);
						Do_Damage((Stewie_BaseGameObj *)o,2500.0f,"None");
					} else if (type == 2) {
						Commands->Create_Object("Beacon_Nuke_Strike_Anim_Post",Commands->Get_Position(o));
						Commands->Create_Explosion("Explosion_NukeBeacon",Commands->Get_Position(o),0);
						Do_Damage((Stewie_BaseGameObj *)o,2500.0f,"None");
					}
				}
			}
		}
	}
}
std::string Buildings_Beacon_Will_Hit(GameObject *obj, int team, bool showdamage) {
	bool first = true;
	const char *n = "Explosion_Mine_Proximity_01";
	if (strstr(Get_Preset_Name(obj),"Ion")) { n = "Explosion_IonCannonBeacon"; }
	else if (strstr(Get_Preset_Name(obj),"Nuke")) { n = "Explosion_NukeBeacon"; }
	char q[512];
	sprintf(q,"");
	const char *mt;
	ExplosionDefinitionClass *explosion = Get_Explosion(n);
	for (Stewie_SLNode<Stewie_BuildingGameObj>* Node = Stewie_GameObjManager::BuildingGameObjList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		Stewie_BuildingGameObj *o = Node->NodeData;
		if (!o) { continue; }
		GameObject *bldg = (GameObject *)(o->As_ScriptableGameObj());
		if (Commands->Get_Health(bldg) <= 0.0f) { continue; }
		if (Get_Object_Type(bldg) != team && (team == 0 || team == 1) && GameDataObj()->IsFriendlyFirePermitted == false) { continue; }
		float PolyDistance = 0.0f;
		Vector3 posV = Commands->Get_Position(obj);
		Stewie_Vector3 pos; pos.X = posV.X; pos.Y = posV.Y; pos.Z = posV.Z;
		o->Find_Closest_Poly(pos,&PolyDistance);
		if (PolyDistance <= pow(explosion->DamageRadius,2)) {
			float dmg = ((0.0127262f) * (float)PolyDistance) - ((58.626928f) * pow((float)PolyDistance,0.5f)) + (876.52768f);
			if (dmg <= 0.0f) { continue; }
			if (first == false) {
				mt = (const char *)q;
				if (showdamage == false) { sprintf(q,"%s, %s",mt,Get_Pretty_Name(bldg).c_str()); }
				else { sprintf(q,"%s, %s(%d)",mt,Get_Pretty_Name(bldg).c_str(),(int)dmg); }
			} else {
				if (showdamage == false) { sprintf(q,"%s",Get_Pretty_Name(bldg).c_str()); }
				else { sprintf(q,"%s(%d)",Get_Pretty_Name(bldg).c_str(),(int)dmg); }
				first = false;
			}
		}
	}
	if (strlen(q) < 3) { sprintf(q,"None"); }
	std::string Retn = q;
	return Retn;
}

Stewie_WeaponClass *Get_Held_Weapon(Stewie_SoldierGameObj *s) {
	Stewie_WeaponBagClass *wb = Get_Weapon_Bag(s->As_ArmedGameObj());
	if (wb) {
		Stewie_WeaponClass *c = wb->Vector[wb->Current];
		return c;
	}
	return NULL;
}
Stewie_WeaponBagClass *Get_Weapon_Bag(Stewie_ArmedGameObj* o) {
	if (!o) { return NULL; }
	Stewie_WeaponBagClass *w = o->WeaponBag;
	return w;
}
std::string Get_Weapon_List(Stewie_ArmedGameObj *o, const char *separator) {
	vTokenParser *WeaponPack = new vTokenParser("");
	WeaponPack->Parse(separator);
	Stewie_WeaponBagClass *bag = Get_Weapon_Bag(o);
	if (bag) {
		int x = bag->Vector.Count();
		for (int i = 0; i < x; i++) {
			if (bag->Vector[i]) {
				WeaponPack->Addtok(bag->Vector[i]->Definition->Get_Name());
			}
		}
	}
	std::string Ret = WeaponPack->Get();
	WeaponPack->Delete();
	return Ret;
}
void Force_Refill(Stewie_ArmedGameObj *o) {
	if (!o) { return; }
	Stewie_WeaponBagClass *bag = Get_Weapon_Bag(o);
	if (!bag) { return; }
	int x = bag->Vector.Count();
	for (int i = 0; i < x; i++) {
		if (bag->Vector[i]) {
			Stewie_WeaponClass *Weapon = bag->Vector[i];
			Stewie_WeaponDefinitionClass *Definition = Weapon->Definition;
			int maxbullets = Definition->DSClipSize.Get();
			int maxclipbullets = Definition->DSMaxClipBullets.Get();
			Weapon->Bullets.Set(maxbullets);
			Weapon->ClipBullets.Set(maxclipbullets);
		}
	}
	o->Set_Object_Dirty_Bit(vDBOCCASIONAL,true);
}
void Powerup(Stewie_SoldierGameObj *o, const char *preset, bool hud) {
	if (!o || !o->As_SmartGameObj() || !preset) { return; }
	Stewie_PowerupGameObjDef *Def = (Stewie_PowerupGameObjDef *)Stewie_DefinitionMgrClass::Find_Typed_Definition(preset,PowerUpCID,true);
	Def->Grant(o->As_SmartGameObj(),NULL,hud);
}

void __stdcall SelectWeaponHook(Stewie_WeaponClass* w) {
	return;
}

void vWeaponManager::Think(bool FullSecond) {
	if (FullSecond) {
		if (Packs.empty()) { return; }
		for (unsigned int i = 0; i < Packs.size(); i++) {
			if (!Commands->Find_Object(Packs[i].PackID)) {
				Packs.erase(Packs.begin() + i);
				i--;
			} else if (Packs[i].CreationTime < (int)clock() - 30 * CLOCKS_PER_SEC) {
				Stewie_BaseGameObj *Obj = (Stewie_BaseGameObj *)(Stewie_NetworkObjectMgrClass::Find_Object(Packs[i].PackID));
				Obj->Set_Delete_Pending();
			}
		}
	}
}
bool vWeaponManager::Collect(int PID, int PackID) {
	if (Packs.empty()) { return true; }
	GameObject *obj = Player_GameObj(PID);
	if (!obj) { return false; }
	vPlayer *p = vPManager->Get_Player(PID);
	if (!p) { return false; }
	for (unsigned int i = 0; i < Packs.size(); i++) {
		if (Packs[i].PackID == PackID) {
			if (Packs[i].CreationTime > (int)clock()) { return false; }
			Stewie_SoldierGameObj *o = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(PID);
			Stewie_WeaponBagClass *bag = Get_Weapon_Bag(o->As_ArmedGameObj());
			for (unsigned int j = 0; j < Packs[i].Weapons.size(); j++) {
				Stewie_WeaponDefinitionClass *Definition = Packs[i].Weapons[j];
				if (isin(Definition->Get_Name(),"ramjet") && isin(Get_Pretty_Name(Player_GameObj(PID)).c_str(),"stealth")) {
					Definition = Stewie_WeaponManager::Find_Weapon_Definition("Weapon_RamjetRifle_Player");
				}
				p->WeaponBag.push_back(Definition);
				int bullets = Definition->DSClipSize.Get() + Definition->DSMaxClipBullets.Get();
				if (Definition->DSMaxClipBullets.Get() == -1) { bullets = -1; }
				bag->Add_Weapon(Definition,bullets,true);
			}
			Commands->Create_Object("Spawner Created Special Effect",Commands->Get_Position(Commands->Find_Object(PackID)));
			Commands->Destroy_Object(Commands->Find_Object(PackID));
			Packs.erase(Packs.begin() + i);
			return true;
		}
	}
	return true;
}
void vWeaponManager::Create_Pack(int PID, bool DefaultWeapons) {
	vPlayer *p = vPManager->Get_Player(PID);
	if (!p) { return; }
	GameObject *o = Player_GameObj(PID);
	if (!o || !Is_Soldier(o)) { return; }
	Stewie_WeaponDefinitionClass *CWep = Stewie_WeaponManager::Find_Weapon_Definition(((Stewie_ArmedGameObjDef *)((Stewie_SoldierGameObj *)o)->As_ArmedGameObj()->Definition)->WeaponDefID);
	if (p->WeaponBag.empty()) {
		Stewie_Vector3 Pos = World_Position((Stewie_BaseGameObj *)o);
		const char *CName = CWep->Get_Name();
		if (!stricmp(CName,"Weapon_RepairGun_Player")) { CName = "CnC_Weapon_MineRemote_Player_2Max"; }
		Create_Pack_Weapon(Pos,CName,false,true);
		return;
	}

	Stewie_Vector3 Pos = World_Position((Stewie_BaseGameObj *)o);
	Stewie_ScriptableGameObj *pack = Create_Object("POW_Backpack",Pos)->As_ScriptableGameObj();
	Script_Attach(pack,"ExpirePowerup","30",true);

	vWeaponPack WPack;
	WPack.PackID = pack->NetworkID;
	WPack.CreationTime = (int)clock();

	WPack.Weapons.push_back(CWep);
	for (unsigned int i = 0; i < p->WeaponBag.size(); i++) {
		WPack.Weapons.push_back(p->WeaponBag[i]);
	}

	Packs.push_back(WPack);
}
void vWeaponManager::Create_Pack_Weapon(Stewie_Vector3 Pos, const char *Weapon, bool UseBackpack, bool Expires) {
	const char *PowPickup = "POW_Backpack";
	if (UseBackpack == false) {
		Stewie_DefinitionClass *PowDef = Get_Powerup_For_Weapon(Weapon);
		if (PowDef) { PowPickup = PowDef->Get_Name(); }
	}
	Stewie_ScriptableGameObj *pack = Create_Object(PowPickup,Pos)->As_ScriptableGameObj();
	if (Expires) { Script_Attach(pack,"ExpirePowerup","30",true); }

	Stewie_WeaponDefinitionClass* Definition = Stewie_WeaponManager::Find_Weapon_Definition(Weapon);
	vWeaponPack WPack;
	WPack.PackID = pack->NetworkID;
	WPack.CreationTime = (int)clock() + ((Expires ? 0 : 5) * CLOCKS_PER_SEC);
	WPack.Weapons.push_back(Definition);

	Packs.push_back(WPack);
}
void vWeaponManager::Give_Weapon(int ID, const char *preset) {
	vPlayer *p = vPManager->Get_Player(ID);
	if (!p) { return; }
	Stewie_DefinitionClass *Definition = Stewie_DefinitionMgrClass::Find_Definition(preset);
	if (!Definition) { return; }
	if (Definition->Get_Class_ID() == PowerUpCID) {
		Stewie_PowerupGameObjDef *PowerupDef = (Stewie_PowerupGameObjDef *)Definition;
		if (!PowerupDef) { return; }
		Stewie_WeaponDefinitionClass *WeaponDef = Stewie_WeaponManager::Find_Weapon_Definition((int)PowerupDef->GrantWeaponDefID);
		if (!WeaponDef) { return; }
		p->WeaponBag.push_back(WeaponDef);
		Commands->Give_Powerup(Player_GameObj(ID),preset,true);
	} else if (Definition->Get_Class_ID() == WeaponCID) {
		Stewie_WeaponDefinitionClass *WeaponDef = (Stewie_WeaponDefinitionClass *)Definition;
		if (!WeaponDef) { return; }
		p->WeaponBag.push_back(WeaponDef);
		Stewie_SoldierGameObj *o = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
		Stewie_WeaponBagClass *bag = Get_Weapon_Bag(o->As_ArmedGameObj());
		if (bag) {
			int bullets = WeaponDef->DSClipSize.Get() + WeaponDef->DSMaxClipBullets.Get();
			if (WeaponDef->DSMaxClipBullets.Get() == -1) { bullets = -1; }
			bag->Add_Weapon(WeaponDef,bullets,true);
		}
	}
}
