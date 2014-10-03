#include "vgm_player.h"
#include "vgm_presets.h"
#include "vgm_vehicle.h"
#include "vgm_crystal.h"
#include "vgm_obj.h"

vAimbotManager *vAManager;

bool vObjects::Valid(Stewie_BaseGameObj *o) {
	if (!o) { return false; }
	if (o->NetworkID <= 0) { return false; }
	if (!o->Definition) { return false; }
	return true;
}
Stewie_BaseGameObj* vObjects::Get_Object(int ID) {
	if (!ID) { return NULL; }
	for (Stewie_SLNode<Stewie_BaseGameObj> *Node = Stewie_BaseGameObj::BaseObjList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->NetworkID == (unsigned int)ID) { return Node->NodeData; }
	}
	return NULL;
}
bool vObjects::Is_Harvester(Stewie_BaseGameObj *o) {
	if (!o) { return false; }
	int NetID = o->NetworkID;
	int GDIHarvID = (Team_Harvester(1) ? Team_Harvester(1)->NetworkID : 0);
	int NodHarvID = (Team_Harvester(0) ? Team_Harvester(0)->NetworkID : 0);
	if (NetID == GDIHarvID || NetID == NodHarvID) { return true; }
	if (isin(o->Definition->Get_Name(),"Harv") && !isin(o->Definition->Get_Name(),"Destroyed")) { return true; }
	return false;
}
bool vObjects::Is_Drivable_Vehicle(Stewie_BaseGameObj *obj) {
	if (!obj) { return false; }
	if (!obj->As_VehicleGameObj()) { return false; }
	if (vObjects::Is_Harvester(obj)) { return false; }
	if (Vehicle_Occupants(obj->As_VehicleGameObj()).Length() <= 0) { return false; }
	Stewie_VehicleGameObjDef *def = (Stewie_VehicleGameObjDef *)obj->As_VehicleGameObj()->Definition;
	if (!def || def->VehicleType == 4) { return false; }
	return obj->As_VehicleGameObj()->TransitionsEnabled;
}

void vAimbotManager::Init() {
	this->Data.clear();
	for (int i = 0; i < 128; i++) {
		this->CreateQueue[i] = false;
	}
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		this->CreateQueue[Node->NodeData->PlayerId] = true;
	}
}
void vAimbotManager::Think(bool FullSecond) {
	for (int i = 0; i < 128; i++) {
		if (this->CreateQueue[i] && this->Create(i)) { this->CreateQueue[i] = false; }
	}
	if (this->Data.empty()) { return; }
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (this->Data[i].CanDelete) {
			vObjects::Get_Object(this->Data[i].ObjID)->Set_Delete_Pending();
			this->Data.erase(this->Data.begin() + i);
			i--;
		}
	}
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (this->Data[i].FakePlayer->IsInGame && this->Data[i].Owner->IsInGame) {
			if (!this->Data[i].FakePlayer->Owner.Reference) {
				Stewie_cPlayer *fakePlayer = this->Data[i].FakePlayer;
				for (unsigned int j = 0; j < this->Data.size(); j++) {
					if (this->Data[i].FakePlayer == fakePlayer) {
						this->CreateQueue[this->Data[i].Owner->PlayerId] = true;
						this->Data[i].CanDelete = true;
					}
				}
				return;
			}
			if (this->Data[i].Owner && this->Data[i].Owner->Owner.Reference) {
				if (this->Data[i].Owner->Owner.Reference->obj->As_SoldierGameObj()->ControlOwner == -1) {
					this->Data[i].CanDelete = true;
				} else {
					Stewie_BaseGameObj *Obj = vObjects::Get_Object(this->Data[i].ObjID);
					if (Obj && this->Data[i].Owner->Owner.Reference->obj->As_PhysicalGameObj()->Phys->As_MoveablePhysClass() &&
						this->Data[i].FakePlayer->Owner.Reference->obj->As_DamageableGameObj()->Defence.Health.Get() > 0.0f) {
							const double AimbotAngle = (float(Random(0,10)) / 10.0f) - 0.5f; // Commands->Get_Random(-0.5f,0.5f);
							const double AimbotMinDistance = 10;
							const double AimbotMaxDistance = 30;
							double AimbotDistance = AimbotMinDistance + (double)rand() / RAND_MAX * (AimbotMaxDistance - AimbotMinDistance);
							double SinAimbotAngle = sin(AimbotAngle);
							double CosAimbotAngle = cos(AimbotAngle);
							Stewie_Matrix3D FakeObjectTransform = Stewie_Matrix3D(this->Data[i].Owner->Owner.Reference->obj->As_PhysicalGameObj()->Phys->Get_Transform());
							FakeObjectTransform.XPos += (float)(FakeObjectTransform.XAxis.X * AimbotDistance * CosAimbotAngle + FakeObjectTransform.YAxis.X * AimbotDistance * SinAimbotAngle);
							FakeObjectTransform.YPos -= (float)(FakeObjectTransform.XAxis.Y * AimbotDistance * CosAimbotAngle + FakeObjectTransform.YAxis.Y * AimbotDistance * SinAimbotAngle);
							FakeObjectTransform.ZPos += 5;
							Obj->As_PhysicalGameObj()->Set_Transform(FakeObjectTransform);
					}
				}
			}
		} else {
			this->Data[i].CanDelete = true;
		}
	}
}
void vAimbotManager::Played_Joined(int PlayerID) {
	this->CreateQueue[PlayerID] = true;
}
bool vAimbotManager::Create(int Target) {
	Stewie_cPlayer *pData = Stewie_cPlayerManager::Find_Player(Target);
	if (!pData || !pData->Owner.Reference || Stewie_cPlayerManager::Tally_Team_Size(pData->PlayerType.Get() ^ 1) == 0) { return false; }
	std::vector<Stewie_cPlayer *> availableData;
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (!Node->NodeData->IsActive || !Node->NodeData->IsInGame) { continue; }
		if (Node->NodeData->PlayerId != Target) { continue; }
		if (!Node->NodeData->Owner.Reference) { continue; }
		if (Node->NodeData->Owner.Reference->obj->As_DamageableGameObj()->Defence.Health.Get() > 0.0f) {
			availableData.push_back(Node->NodeData);
		}
	}
	if (availableData.size() <= 0) { return false; }

	Stewie_cPlayer *newFake = availableData[Random(0,(int)availableData.size(),true)];
	Stewie_BaseGameObj *Obj = Create_Object(newFake->Owner.Reference->obj->Definition->Get_Name(),World_Position((Stewie_BaseGameObj *)Player_GameObj(Target)),false);
	if (!Obj) { return false; }

	AimbotObj data;
	data.ObjID = Obj->NetworkID;
	data.FakePlayer = newFake;
	data.Owner = pData;
	data.CanDelete = false;
	this->Data.push_back(data);

	Set_Model(Obj,"NULL");
	Set_Is_Visible(Obj->As_ScriptableGameObj(),false);

	Stewie_SoldierGameObj *bot = Obj->As_ScriptableGameObj()->As_SoldierGameObj();
	bot->Set_Control_Owner(newFake->PlayerId);
	bot->Set_Player_Type(pData->PlayerType.Get() ^ 1);
	bot->Set_Object_Dirty_Bit(vDBCREATION,false);
	bot->Set_Object_Dirty_Bit(vDBOCCASIONAL,false);
	bot->Set_Object_Dirty_Bit(vDBRARE,false);
	bot->Set_Object_Dirty_Bit(vDBFREQUENT,false);
	bot->Set_Object_Dirty_Bit(Target,vDBCREATION,true);
	bot->Defence.Set_Health(float(Random(0,2000)));
	bot->Defence.Set_Shield_Strength(float(Random(0,2000)));
	Stewie_cNetwork::Send_Object_Update(bot,Target);
	bot->Toggle_Fly_Mode();
	bot->Set_Object_Dirty_Bit(vDBCREATION,false);
	bot->Set_Control_Owner(-1);
	bot->Defence.CanObjectDie = false;
	bot->Defence.Set_Shield_Type(24);
	return true;
}

Stewie_BuildingGameObj *Team_Building(int Team, vBUILDINGTYPE Type) {
	Stewie_BaseControllerClass *Base = Stewie_BaseControllerClass::Find_Base(Team);
	if (!Base) { return NULL; }
	return Base->Find_Building(Type);
}
Stewie_BuildingGameObj *Team_Base_Defense(int Team) {
	return Team_Building(Team,vBaseDefense);
}
Stewie_SoldierFactoryGameObj *Team_Barracks(int Team) {
	Stewie_BuildingGameObj *obj = Team_Building(Team,vSoldierFactory);
	if (!obj) { return NULL; }
	return obj->As_SoldierFactoryGameObj();
}
Stewie_VehicleFactoryGameObj *Team_War_Factory(int Team) {
	Stewie_BuildingGameObj *obj = Team_Building(Team,vVehicleFactory);
	if (!obj) { return NULL; }
	return obj->As_VehicleFactoryGameObj();
}
Stewie_RefineryGameObj *Team_Refinery(int Team) {
	Stewie_BuildingGameObj *obj = Team_Building(Team,vRefinery);
	if (!obj) { return NULL; }
	return obj->As_RefineryGameObj();
}
Stewie_PowerPlantGameObj *Team_Power_Plant(int Team) {
	Stewie_BuildingGameObj *obj = Team_Building(Team,vRefinery);
	if (!obj) { return NULL; }
	return obj->As_PowerPlantGameObj();
}
Stewie_VehicleGameObj *Team_Harvester(int team) {
	if (!Team_Refinery(team)) { return NULL; }
	Stewie_HarvesterClass *Harvester = Team_Refinery(team)->Harvester;
	if (!Harvester || !Harvester->Harvester) { return NULL; }
	return Harvester->Harvester->As_VehicleGameObj();
}

const char *Get_Preset_Name(GameObject *obj) {
	return Commands->Get_Preset_Name(obj);
}
Stewie_DefinitionClass *Find_Random(unsigned int ClassID, int Team) {
	std::vector<Stewie_DefinitionClass *> Possibilities;
	for (Stewie_DefinitionClass *Def = Stewie_DefinitionMgrClass::Get_First(ClassID,1); Def; Def = Stewie_DefinitionMgrClass::Get_Next(Def,ClassID,1)) {
		if (!Def) { continue; }
		if (ClassID == SoldierCID) {
			Stewie_SoldierGameObjDef *SDef = (Stewie_SoldierGameObjDef *)Def;
			if (Team != 2 && SDef->DefaultPlayerType != Team) { continue; }
			if (SDef->DefaultPlayerType != 0 && SDef->DefaultPlayerType != 1) { continue; }
			if (!isin(SDef->Get_Name(),"CnC_") || isin(SDef->Get_Name(),"Chicken") || isin(SDef->Get_Name(),"Skirmish")) { continue; }
		} else if (ClassID == VehicleCID) {
			Stewie_VehicleGameObjDef *SDef = (Stewie_VehicleGameObjDef *)Def;
			if (Team != 2 && SDef->DefaultPlayerType != Team) { continue; }
			if (SDef->VehicleType != 0 && SDef->VehicleType != 1) { continue; }
			if (!isin(SDef->Get_Name(),"CnC_") || isin(SDef->Get_Name(),"Harv") || isin(SDef->Get_Name(),"Skirmish")) { continue; }
		} else if (ClassID == WeaponCID) {
			Stewie_WeaponDefinitionClass *SDef = (Stewie_WeaponDefinitionClass *)Def;
			if (!SDef->AGiveWeaponsWeapon) { continue; }
			if (isin(SDef->Get_Name(),"pistol")) { continue; }
			if (isin(SDef->Get_Name(),"Ultimate")) { continue; }
		}
		Possibilities.push_back(Def);
	}
	if (Possibilities.size() <= 0) { return NULL; }
	int r = Random(0,Possibilities.size(),true);
	return Possibilities[r];
}
Stewie_DefinitionClass *Get_Powerup_For_Weapon(const char *weapon) {
	Stewie_WeaponDefinitionClass* Definition = Stewie_WeaponManager::Find_Weapon_Definition(weapon);
	if (!Definition) { return NULL; }
	for (Stewie_DefinitionClass *Def = Stewie_DefinitionMgrClass::Get_First(PowerUpCID,1); Def; Def = Stewie_DefinitionMgrClass::Get_Next(Def,PowerUpCID,1)) {
		if (!Def) { continue; }
		Stewie_PowerupGameObjDef *PowerupDef = (Stewie_PowerupGameObjDef *)Def;
		if (PowerupDef->GrantWeaponDefID == Definition->Get_ID()) {
			return Def;
		}
	}
	return NULL;
}
Stewie_BaseGameObjDef *Get_Object_Definition(GameObject *obj) {
	if (!obj) { return NULL; }
	if (Commands->Get_ID(obj) <= 0) { return NULL; }
	Stewie_BaseGameObj *o = (Stewie_BaseGameObj *)obj;
	if (!o) { return NULL; }
	if (!o->Definition) { return NULL; }
	return (Stewie_BaseGameObjDef *)o->Definition;
}
Stewie_SoldierGameObjDef *Get_Soldier_Definition(GameObject *obj) {
	if (!obj) { return NULL; }
	if (Commands->Get_ID(obj) <= 0) { return NULL; }
	if (!Is_Soldier(obj)) { return NULL; }
	Stewie_SoldierGameObj *o = (Stewie_SoldierGameObj *)obj;
	if (!o) { return NULL; }
	if (!o->As_SoldierGameObj()) { return NULL; }
	if (!o->Definition) { return NULL; }
	Stewie_SoldierGameObjDef *def = (Stewie_SoldierGameObjDef *)o->Definition;
	return def;
}
Stewie_VehicleGameObjDef *Get_Vehicle_Definition(GameObject *obj) {
	if (!obj) { return NULL; }
	if (Commands->Get_ID(obj) <= 0) { return NULL; }
	if (!Is_Vehicle(obj)) { return NULL; }
	Stewie_VehicleGameObj *o = (Stewie_VehicleGameObj *)obj;
	if (!o) { return NULL; }
	if (!o->As_VehicleGameObj()) { return NULL; }
	if (!o->Definition) { return NULL; }
	Stewie_VehicleGameObjDef *def = (Stewie_VehicleGameObjDef *)o->Definition;
	return def;
}

int Get_Vehicle_Parent_Type(GameObject *obj) {
	Stewie_VehicleGameObjDef *def = Get_Vehicle_Definition(obj);
	return (def ? def->DefaultPlayerType : -1);
}
int Get_Soldier_State(GameObject *obj) {
	// 0 = Upright, 1 = Crouched, 2 = Sniping, 3 = Wounded, 5 = Airborne, 7 = Dead, 9 = In Vehicle, 10 = In Transition, 12 = Destroyed
	if (!obj || !Commands->Get_ID(obj) || !Is_Soldier(obj)) { return 0; }
	obj = As_SoldierGameObj(obj);
	Stewie_SoldierGameObj *s = (Stewie_SoldierGameObj *)obj;
	Stewie_HumanStateClass::HumanStateType State = s->HumanState.Type;
	return (int)State;
}
bool Is_Soldier_Airborne(GameObject *obj) {
	if (!obj) { return false; }
	return (Get_Soldier_State(obj) == 5);
}
void Set_Model(Stewie_BaseGameObj *obj, const char *preset) {
	Stewie_PhysicalGameObj *AsPhys = obj->As_PhysicalGameObj();
	if (!AsPhys) { return; }
	Stewie_SoldierGameObj *AsSoldier = AsPhys->As_SoldierGameObj();
	if (AsSoldier) {
		AsSoldier->Set_Model(preset);
	} else if (AsPhys->Phys) {
		AsPhys->Phys->Set_Model_By_Name(preset);
		AsPhys->Get_Anim_Control()->Set_Model(AsPhys->Phys->Model);
	}
	AsPhys->Hide_Muzzle_Flashes(true);
	obj->Set_Object_Dirty_Bit(vDBRARE,true);
}
void Set_Character(Stewie_SoldierGameObj *obj, const char *preset) {
	Stewie_SoldierGameObjDef *SoldierDef = (Stewie_SoldierGameObjDef *)Stewie_DefinitionMgrClass::Find_Definition(preset);
	if (SoldierDef) { obj->Re_Init(*SoldierDef); }
}
float Get_Total_Health(Stewie_BaseGameObj *obj) {
	if (!obj || !obj->As_ScriptableGameObj()) { return 0.0f; }
	Stewie_DamageableGameObj *o = obj->As_ScriptableGameObj()->As_DamageableGameObj();
	return o->Defence.Get_Health() + o->Defence.Get_Shield_Strength();
}
float Get_Total_Max_Health(Stewie_BaseGameObj *obj) {
	if (!obj || !obj->As_ScriptableGameObj()) { return 0.0f; }
	Stewie_DamageableGameObj *o = obj->As_ScriptableGameObj()->As_DamageableGameObj();
	return o->Defence.Get_Health_Max() + o->Defence.Get_Shield_Strength_Max();
}

Stewie_Vector3 World_Position(Stewie_BaseGameObj *o) {
	if (!o || !o->NetworkID) { return Stewie_Vector3(); }
	if (!o->As_ScriptableGameObj()) { return Stewie_Vector3(); }
	Stewie_Vector3 Value;
	o->As_ScriptableGameObj()->Get_Position(&Value);
	return Value;
}
Stewie_BaseGameObj *Create_Object(const char *Preset, Stewie_Vector3 Position, bool Collisions) {
	Stewie_ScriptableGameObj *Obj = Stewie_ObjectLibraryManager::Create_Object(Preset);
	if (!Obj) { return NULL; }
	Obj->As_PhysicalGameObj()->Set_Position(Position);
	if (!Collisions) { Obj->As_PhysicalGameObj()->Set_Collision_Group(1); }
	Obj->Start_Observers();
	return Obj->As_BaseGameObj();
}
void Create_Explosion(const char *Preset, Stewie_Vector3 Position, Stewie_ScriptableGameObj *Damager) {
	if (!Preset) { return; }
	Stewie_DefinitionClass *Definition = Stewie_DefinitionMgrClass::Find_Typed_Definition(Preset,ExplosionCID,true);
	if (!Definition) { return; }
	if (!Damager || !Damager->As_SoldierGameObj()) { Damager = NULL; }
	DebugMessage("EXPLODE %s %f %f %f %d %d %u",Preset,Position.X,Position.Y,Position.Z,Damager,Definition,Definition->Get_ID());
	Stewie_ExplosionManager::Create_Explosion_At(Definition->Get_ID(),Position,(Stewie_ArmedGameObj *)Damager,Stewie_Vector3(0.0f,0.0f,-1.0f),NULL);
}
Stewie_ScriptableGameObj *Set_Up_SAM_Site(int team, Stewie_Vector3 Position) {
	Stewie_ScriptableGameObj *SAM = Create_Object("SAM_Site_Quick_Turn",Position)->As_ScriptableGameObj();
	if (!SAM) { return NULL; }
	SAM->As_DamageableGameObj()->Set_Player_Type(team);
	SAM->As_SmartGameObj()->IsEnemySeenEnabled = true;
	Script_Attach(SAM,"vSAMSite","",true);
	return SAM;
}
void Set_Is_Visible(Stewie_ScriptableGameObj *obj, bool visible) {
	if (obj->As_SoldierGameObj()) {
		obj->As_SoldierGameObj()->IsVisible = visible;
	} else if (obj->As_VehicleGameObj()) {
		bool *b = (bool *)obj;
		b += 0x995;
		*b = visible;
	} else {
		return;
	}
	int x = 0;
	if (!visible) { x = Script_Attach(obj,"Invisible","",true); }
	else { Script_Remove(obj,"Invisible"); }
}

float Find_Distance_To_Closest_Object_By_Preset(Vector3 &pos, const char *preset) {
	float d = FLT_MAX;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_ScriptableGameObj((GameObject *)Node->NodeData);
		if (o) {
			float dist = Commands->Get_Distance(Commands->Get_Position(o),pos);
			if (!stricmp(preset,Get_Preset_Name(o)) && dist < d) {
				d = dist;
			}
		}
	}
	return d;
}
void Kill_C4(GameObject *obj) {
	Kill_C4((Stewie_C4GameObj *)obj);
}
void Kill_C4(Stewie_C4GameObj *obj) {
	Create_Explosion("Explosion_Mine_Disarm",Stewie_Vector3(World_Position(obj)),NULL);
	//Commands->Create_Explosion("Explosion_Mine_Disarm",Vector3(p.X,p.Y,p.Z),0);
	obj->Set_Delete_Pending();
}
Stewie_SoldierGameObj *Get_C4_Owner(Stewie_C4GameObj *o) {
	if (!o || !o->Owner.Reference || !o->Owner.Reference->obj) { return NULL; }
	return o->Owner.Reference->obj->As_SoldierGameObj();
}
int Get_C4_Type(Stewie_C4GameObj *o) {
	return (o->Ammo ? o->Ammo->AmmoType.Get() : 0);
}
Stewie_C4GameObj *Find_First_C4_By_Player(int Owner, int Type) {
	Stewie_C4GameObj *First = NULL;
	for (Stewie_SLNode<Stewie_BaseGameObj> *Node = Stewie_BaseGameObj::BaseObjList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		Stewie_PhysicalGameObj *o = Node->NodeData->As_PhysicalGameObj();
		if (o && o->As_C4GameObj()) {
			Stewie_C4GameObj *C4Obj = o->As_C4GameObj();
			Stewie_SoldierGameObj *s = Get_C4_Owner(C4Obj);
			if (!s || !s->Player) { continue; }
			if (Get_C4_Type(C4Obj) == Type && s->Player->PlayerId == Owner) {
				First = C4Obj;
			}
		}
	}
	return First;
}
int Get_Player_C4_Count(GameObject *Owner, int Type) {
	int count = 0;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_ScriptableGameObj((GameObject *)Node->NodeData);
		if (o && Is_C4(o)) {
			if (Get_C4_Mode(o) == Type && Get_C4_Planter(o) == Owner) {
				count++;
			}
		}
	}
	return count;
}
int Get_C4_Count_Type(int Team, int Type) {
	int count = 0;
	for (Stewie_SLNode<Stewie_BaseGameObj> *Node = Stewie_BaseGameObj::BaseObjList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		Stewie_PhysicalGameObj *o = Node->NodeData->As_PhysicalGameObj();
		if (o && o->As_C4GameObj()) {
			if ((Team == 2 || o->PlayerType == Team) && Get_C4_Type(o->As_C4GameObj()) == Type) {
				count++;
			}
		}
	}
	return count;
}
int Get_C4_Count_Timed(int Team) {
	int count = 0;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_ScriptableGameObj((GameObject *)Node->NodeData);
		if (o && Is_C4(o)) {
			if ((Get_Object_Type(o) == Team || Team == 2) && Get_C4_Mode(o) == 2) {
				count++;
			}
		}
	}
	return count;
}
int Get_Proxy_C4s_Attached_By_Building(GameObject *building, int Team) {
	int total = 0;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = (GameObject *)Node->NodeData;
		if (o && Is_C4(o)) {
			if (Team == 2 || Get_Object_Type(o) == Team && Get_C4_Mode(o) == 3) {
				if (Get_C4_Attached(o) == building) {
					total++;
				}
			}
		}
	}
	return total;
}
std::string Find_Proxy_C4_On_Buildings(int Team) {
	bool first = true;
	char ret[1024];
	const char *temp;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_BuildingGameObj((GameObject *)Node->NodeData);
		if (o) {
			if (Team == 2 || Get_Object_Type(o) == Team) {
				if (first) {
					sprintf(ret,"%s(%d)",Get_Pretty_Name(o).c_str(),Get_Proxy_C4s_Attached_By_Building(o,Team));
					first = false;
				} else {
					temp = (const char *)ret;
					sprintf(ret,"%s, %s(%d)",temp,Get_Pretty_Name(o).c_str(),Get_Proxy_C4s_Attached_By_Building(o,Team));
				}
			}
		}
	}
	std::string Retn = ret;
	return Retn;
}
int Get_C4_Limit_Type(int team, int C4Type) {
	int LimitRemote, LimitTimed, LimitProx;
	if (team == 1) {
		LimitRemote = Config->GDIRemoteC4Limit;
		LimitTimed = Config->GDITimedC4Limit;
		LimitProx = Config->GDIProximityC4Limit;
	} else if (team == 0) {
		LimitRemote = Config->NODRemoteC4Limit;
		LimitTimed = Config->NODTimedC4Limit;
		LimitProx = Config->NODProximityC4Limit;
	} else {
		LimitRemote = 20;
		LimitTimed = -1;
		LimitProx = 35;
	}
	if (C4Type == 1) { return LimitRemote; }
	if (C4Type == 2) { return LimitTimed; }
	if (C4Type == 3) { return LimitProx; }
	return 0;
}
int Get_Player_Beacon_Count(int PlayerID) {
	int count = 0;
	for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		GameObject *o = As_ScriptableGameObj((GameObject *)Node->NodeData);
		if (o && Is_Beacon(o)) {
			if (Get_Player_ID(Get_Beacon_Planter(o)) == PlayerID) {
				count++;
			}
		}
	}
	return count;
}
void Destroy_All_Objects_By_Preset(int Team, const char *Preset) {
	GenericSLNode *x = BaseGameObjList->HeadNode;
	while (x) {
		GameObject *o = As_ScriptableGameObj((GameObject *)x->NodeData);
		if (o) {
			if ((Get_Object_Type(o) == Team || Team == 2) && !stricmp(Get_Preset_Name(o),Preset)) {
				Commands->Destroy_Object(o);
			}
		}
		x = x->NodeNext;
	}
}
GameObject *Find_Random_Player_By_Team(int Team) {
	SimpleDynVecClass<GameObject*> ObjList(0);
	GenericSLNode *x = BaseGameObjList->HeadNode;
	while (x) {
		GameObject *o = As_ScriptableGameObj((GameObject *)x->NodeData);
		if (o) {
			if ((Get_Object_Type(o) == Team || Team == 2) && Get_Player_ID(o) > 0) {
				ObjList.Add(o);
			}
		}
		x = x->NodeNext;
	}
	int len = ObjList.Count();
	if (len > 1) {
		return ObjList[Random(0,len,true)];
	} else if (!len) {
		return 0;
	}
	return ObjList[0];
}
GameObject *Find_Random_Building_By_Team(int Team, bool alive) {
	SimpleDynVecClass<GameObject*> ObjList(0);
	for (GenericSLNode *Node = BuildingGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		GameObject *o = As_BuildingGameObj((GameObject *)Node->NodeData);
		if (o && Commands->Get_ID(o) > 0) {
			if (Get_Object_Type(o) == Team || Team == 2) {
				if (alive == false || (alive && Commands->Get_Health(o) > 0.0f)) {
					ObjList.Add(o);
				}
			}
		}
	}
	int len = ObjList.Count();
	if (len > 1) { return ObjList[Random(0,len,true)]; }
	else if (!len) { return 0; }
	return ObjList[0];
}
GameObject *Find_Closest_Building_By_Team(int Team, Vector3 position) {
	float closestdist = FLT_MAX;
	GameObject *closest = 0;
	for (GenericSLNode *Node = BuildingGameObjList->HeadNode; Node; Node = Node->NodeNext) {
		GameObject *o = As_BuildingGameObj((GameObject *)Node->NodeData);
		if (o && Commands->Get_ID(o) > 0) {
			Vector3 pos = Commands->Get_Position(o);
			float dist = Commands->Get_Distance(pos,position);
			if (dist < closestdist && (Get_Object_Type(o) == Team || Team == 2)) {
				closestdist = dist;
				closest = o;
			}
		}
	}
	return closest;
}

void Update_Dirty_Bits(Stewie_NetworkObjectClass* obj) {
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		Stewie_cNetwork::Send_Object_Update(obj,Node->NodeData->PlayerId);
	}
}

bool Has_Observer(Stewie_ScriptableGameObj& obj, const char* scn) {
	for (int i = obj.Observers.Count() - 1; i >= 0; i--) {
		if (!stricmp(obj.Observers[i]->Get_Name(),scn)) {
			return true;
		}
	}
	return false;
}
void Remove_Observer(Stewie_ScriptableGameObj& obj, const char* scn) {
	for (int i = obj.Observers.Count() - 1; i >= 0; i--) {
		if (!stricmp(obj.Observers[i]->Get_Name(),scn)) {
			obj.Remove_Observer(obj.Observers[i]);
		}
	}
}

void __stdcall ObjectCreationHook(Stewie_ScriptableGameObj *obj) {
	if (!vObjects::Valid(obj)) { return; }
	Script_Remove(obj,"M00_Vehicle_Log");
	const char *preset = obj->Definition->Get_Name();
	if (Config->Crates && isin(preset,"_Crate")) {
		if (GameDataObj()->Is_Gameplay_Permitted()) {
			Stewie_Vector3 Position = World_Position(obj);
			Stewie_ScriptableGameObj *nObj = Create_Object("POW_Tissue_Nanites",Position)->As_ScriptableGameObj();
			Script_Attach(nObj,"vCrateHandler","",true);
		}
		obj->Set_Delete_Pending();
		return;
	}
	vPManager->Add_Seen_Object(obj->NetworkID);
	if (obj->As_VehicleGameObj()) {
		if (vObjects::Is_Drivable_Vehicle(obj) || vObjects::Is_Harvester(obj)) { Script_Attach(obj,"M00_Vehicle_Log","",true); }
	}
}
bool __stdcall DoorFixHook(Stewie_SmartGameObj *Object, Stewie_DoorPhysClass *Door) {
	if (!Object || !Door) { return false; }
	GameObject *obj = (GameObject *)Object;
	if (Is_Soldier(obj) && Get_Vehicle(obj) && ((Stewie_DoorPhysDefClass *)Door->Definition)->DoorOpensForVehicles == false) { return false; }
	return Object->Is_Human_Controlled();
}
bool __stdcall DamageHook(Stewie_cCsDamageEvent& dmgevent) {
	if (!GameDataObj()->Is_Gameplay_Permitted()) { return false; }

	if (!vBigheadObserver::CheckDamageEvent(dmgevent)) { return false; }
	if (!Config->CheckDamageEvents) { return true; }

	if (dmgevent.damage == 0.0f) { return false; }
	if (dmgevent.damagerID == dmgevent.targetID) { return true; }

	DamageEvent DamageObject;

	DamageObject.Damager = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(dmgevent.ID);
	if (!DamageObject.Damager) { return true; }

	DamageObject.DamagerAsPhys = Stewie_GameObjManager::Find_PhysicalGameObj(dmgevent.damagerID);
	if (!DamageObject.DamagerAsPhys || !DamageObject.DamagerAsPhys->Phys) { return true; }

	DamageObject.TargetAsPhys = Stewie_GameObjManager::Find_PhysicalGameObj(dmgevent.targetID);
	if (!DamageObject.TargetAsPhys || !DamageObject.TargetAsPhys->Phys) { return true; }

	DamageObject.PlayerId = dmgevent.ID;
	DamageObject.EvtDamage = dmgevent.damage;
	DamageObject.EvtWarhead = dmgevent.warhead;
	DamageObject.IsDamagerVehicle = (DamageObject.DamagerAsPhys->As_VehicleGameObj() || DamageObject.Damager->VehicleOccupied);

	bool Allow = CheckDamageEvent(DamageObject);
#ifdef DEBUG
	int Time = (int)clock();
	bool x = false;
	unsigned int Num = 1000;
	for (unsigned int i = 0; i < Num; i++) { x = CheckDamageEvent(DamageObject); }
	DebugMessage("DID %u EVENTS IN %.3f SECS, RESULT %d",Num,float((int)clock() - Time) / float(CLOCKS_PER_SEC),x);
#endif
	return Allow;
}
bool CheckDamageEvent(DamageEvent& Event) {
	int ID = Event.PlayerId;
	vPlayer *p = vPManager->Get_Player(ID);
	if (!p) { return false; }
	
	Stewie_cPlayer *cP = Event.Damager->Player;
	if (!cP || !cP->IsActive) { return false; }

	if (Config->BlockNoSerialActions && !p->serial) { return false; }
	if (p->CheatPending > vManager.DurationAsInt()) { return true; }

	if (Event.Damager->Defence.Get_Health() <= 0.0f) { return false; }

	if (Event.TargetAsPhys->As_SoldierGameObj()) {
		if (Is_Spectating(Event.TargetAsPhys)) { return false; }
	} else {
		if (!stricmp(Event.TargetAsPhys->Definition->Get_Name(),"pct_zone_nod") || 
			!stricmp(Event.TargetAsPhys->Definition->Get_Name(),"pct_zone_gdi")) { return false; }
	}

	bool DamageAllowed = true;
	bool Kick = false;

	if (Event.IsDamagerVehicle == false) {
		Stewie_WeaponClass *Weapon = Get_Held_Weapon(Event.Damager);
		if (!Weapon || !Weapon->Definition) { return false; }
		if (Weapon->State == (int)WeaponState::wsSwitching || Weapon->State == (int)WeaponState::wsWaiting) { return true; }

		Stewie_AmmoDefinitionClass *Ammo = NULL;
		int WeaponID = 0;
		if (Weapon->SecondaryTriggered) {
			Ammo = Weapon->SecondaryAmmoDef;
			WeaponID = Weapon->Definition->SecondaryAmmoDefId;
			p->LastFiringMode = 2;
		} else {
			Ammo = Weapon->PrimaryAmmoDef;
			WeaponID = Weapon->Definition->PrimaryAmmoDefId;
			p->LastFiringMode = 1;
		}
		if (!Ammo || !WeaponID) { return false; }
		float DefDamage = Ammo->Damage.Get();
		if (DefDamage == 0.0f) { return false; }

		int ping = cP->Get_Ping();
		float PingFactor = float(ping) / 1000.0f;
		if (PingFactor < 1.0f) { PingFactor = 1.0f; }

		if (Event.Damager->As_SoldierGameObj() || Event.Damager->As_VehicleGameObj()) { p->Add_Hit((int)Event.TargetAsPhys->NetworkID,WeaponID); }

		Stewie_Vector3 ShooterPos = World_Position(Event.DamagerAsPhys);
		Stewie_Vector3 TargetPos = World_Position(Event.TargetAsPhys);
		
		if (Config->DetectDmgHack) {
			float ExpectedBodyshotDamage = DefDamage;
			float ExpectedNeckshotDamage = ExpectedBodyshotDamage*3;
			float ExpectedHeadshotDamage = ExpectedBodyshotDamage*5;
			bool ShowCheatMsg = false;
			if (Event.EvtDamage > 0) {
				if (Event.EvtDamage != ExpectedBodyshotDamage && Event.EvtDamage != ExpectedNeckshotDamage && Event.EvtDamage != ExpectedHeadshotDamage) {
					if (Event.EvtDamage > 1000.0f) {
						Kick = true;
						if (Config->BlockDamageEvents) { Kill_Player(ID); }
					}
					ShowCheatMsg = true;
				}
			} else {
				ExpectedNeckshotDamage = ExpectedBodyshotDamage;
				ExpectedHeadshotDamage = ExpectedBodyshotDamage;
				if (Event.EvtDamage != ExpectedBodyshotDamage) {
					if (Event.EvtDamage < -4.0f) {
						Kick = true;
						if (Config->BlockDamageEvents) { Kill_Player(ID); }
					}
					ShowCheatMsg = true;
				}
			}
			if (ShowCheatMsg) {
				vLogger->Log(vLoggerType::vCHEAT,"",
					"Player %s did %.4f damage to %s:",
					p->name.c_str(),
					Event.EvtDamage,
					Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str()
				);
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tShooter Data: \
					\n\t\t\tPreset: %s\
					\n\t\t\tDamage: %.2f (%.2f,%.2f,%.2f expected) \
					\n\t\t\tWarhead: %u (%s) \
					\n\t\t\tWeapon: %s \
					\n\t\t\tAmmunition: %s \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
					Event.DamagerAsPhys->Definition->Get_Name(),
					Event.EvtDamage,
					ExpectedBodyshotDamage,
					ExpectedNeckshotDamage,
					ExpectedHeadshotDamage,
					Event.EvtWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
					Weapon->Definition->Get_Name(),
					Ammo->Get_Name(),
					ShooterPos.X,
					ShooterPos.Y,
					ShooterPos.Z,
					Event.DamagerAsPhys->Get_Facing()
				);
				vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tVictim Data: \
					\n\t\t\tPreset: %s (%s) \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
					\n",
					Event.TargetAsPhys->Definition->Get_Name(),
					Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
					TargetPos.X,
					TargetPos.Y,
					TargetPos.Z,
					Event.TargetAsPhys->Get_Facing()
				);

				vCheatMessage Msg;
				Msg.ID = ID;
				Msg.Type = vCheatMsgType::vDAMAGE;
				Msg.WeaponID = WeaponID;
				Msg.EvictAfterMessage = Kick;
				Msg.Damage = Event.EvtDamage;
				Msg.ExpectedDamage = ExpectedBodyshotDamage;
				vPManager->Add_Cheat_Message(Msg);

				DamageAllowed = false;
			}
		}

		if (Config->DetectWHHack) {
			unsigned int ExpectedWarhead = (unsigned int)Ammo->WarheadId.Get();
			if (ExpectedWarhead > 0 && Event.EvtWarhead != ExpectedWarhead) {
				if (Event.EvtWarhead == 24) {
					Kick = true;
					if (Config->BlockDamageEvents) { Kill_Player(ID); }
				}

				vLogger->Log(vLoggerType::vCHEAT,"",
					"Player %s used warhead %u when damaging %s:",
					p->name.c_str(),
					Event.EvtWarhead,
					Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str()
				);
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tShooter Data: \
					\n\t\t\tPreset: %s\
					\n\t\t\tDamage: %.2f \
					\n\t\t\tWarhead: %u (%s), Expected: %u (%s) \
					\n\t\t\tWeapon: %s \
					\n\t\t\tAmmunition: %s \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
					Event.DamagerAsPhys->Definition->Get_Name(),
					Event.EvtDamage,
					Event.EvtWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
					ExpectedWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(ExpectedWarhead),
					Weapon->Definition->Get_Name(),
					Ammo->Get_Name(),
					ShooterPos.X,
					ShooterPos.Y,
					ShooterPos.Z,
					Event.DamagerAsPhys->Get_Facing()
				);
				vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tVictim Data: \
					\n\t\t\tPreset: %s (%s) \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
					\n",
					Event.TargetAsPhys->Definition->Get_Name(),
					Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
					TargetPos.X,
					TargetPos.Y,
					TargetPos.Z,
					Event.TargetAsPhys->Get_Facing()
				);

				vCheatMessage Msg;
				Msg.ID = ID;
				Msg.Type = vCheatMsgType::vWARHEAD;
				Msg.EvictAfterMessage = Kick;
				Msg.WeaponID = WeaponID;
				Msg.Warhead = Event.EvtWarhead;
				Msg.ExpectedWarhead = ExpectedWarhead;
				vPManager->Add_Cheat_Message(Msg);

				DamageAllowed = false;
			}
		}

		if (Config->DetectScopeHack) {
			bool CanScope = Weapon->Definition->CanSnipe;
			int State = (int)(Event.Damager->HumanState.Type);
			if (CanScope == false && State == 2) {
				vLogger->Log(vLoggerType::vCHEAT,"",
					"Player %s used scope on nonscopable weapon when damaging %s:",
					p->name.c_str(),
					Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str()
				);
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tShooter Data: \
					\n\t\t\tPreset: %s\
					\n\t\t\tDamage: %.2f \
					\n\t\t\tWarhead: %u (%s) \
					\n\t\t\tWeapon: %s \
					\n\t\t\tAmmunition: %s \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
					Event.DamagerAsPhys->Definition->Get_Name(),
					Event.EvtDamage,
					Event.EvtWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
					Weapon->Definition->Get_Name(),
					Ammo->Get_Name(),
					ShooterPos.X,
					ShooterPos.Y,
					ShooterPos.Z,
					Event.DamagerAsPhys->Get_Facing()
				);
				vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tVictim Data: \
					\n\t\t\tPreset: %s (%s) \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
					\n",
					Event.TargetAsPhys->Definition->Get_Name(),
					Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
					TargetPos.X,
					TargetPos.Y,
					TargetPos.Z,
					Event.TargetAsPhys->Get_Facing()
				);

				vCheatMessage Msg;
				Msg.ID = ID;
				Msg.Type = vCheatMsgType::vSCOPE;
				Msg.WeaponID = WeaponID;
				vPManager->Add_Cheat_Message(Msg);

				DamageAllowed = false;
			}
		}

		if (Config->DetectROFHack) {
			int ValidHits = 1;
			float MeasureTime = 1.0f;
			if (Weapon->Definition->DSClipSize.Get() == 1) {
				float ReloadTime = Weapon->Definition->DSReloadTime.Get();
				MeasureTime = ((ReloadTime * (1.0f - Ammo->RateOfFire)) > 1.0f) ? 1.0f : floor((1.0f / Ammo->RateOfFire) + ReloadTime);
			} else {
				MeasureTime = (Ammo->RateOfFire > 1.0f) ? 1.0f : floor(1.0f / Ammo->RateOfFire);
				ValidHits = (int)((Ammo->RateOfFire + 1) * Ammo->SprayCount.Get() + 1.5f);
			}
			ValidHits = (int)(float(ValidHits) * (PingFactor * MeasureTime) + 0.5f); // Account for possible lag?
			if (ValidHits <= 0) { ValidHits = 1; }

			int Hits = p->Get_Hits(Event.TargetAsPhys->NetworkID,MeasureTime,WeaponID);
			int ExcessHits = (Hits - ValidHits);
			if (ExcessHits > 0) {
				if (ExcessHits >= 5) {
					vLogger->Log(vLoggerType::vCHEAT,"",
						"Player %s exceeded maximum allowed hits when damaging %s:",
						p->name.c_str(),
						Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str()
					);
					vLogger->Log(vLoggerType::vCHEAT,"",
						"\tShooter Data: \
						\n\t\t\tPreset: %s\
						\n\t\t\tHits: %d (expected %d, measure time %.2f) \
						\n\t\t\tDamage: %.2f \
						\n\t\t\tWarhead: %u (%s) \
						\n\t\t\tWeapon: %s \
						\n\t\t\tAmmunition: %s \
						\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
						Event.DamagerAsPhys->Definition->Get_Name(),
						Hits,
						ValidHits,
						MeasureTime,
						Event.EvtDamage,
						Event.EvtWarhead,
						Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
						Weapon->Definition->Get_Name(),
						Ammo->Get_Name(),
						ShooterPos.X,
						ShooterPos.Y,
						ShooterPos.Z,
						Event.DamagerAsPhys->Get_Facing()
					);
					vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
					vLogger->Log(vLoggerType::vCHEAT,"",
						"\tVictim Data: \
						\n\t\t\tPreset: %s (%s) \
						\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
						\n",
						Event.TargetAsPhys->Definition->Get_Name(),
						Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
						TargetPos.X,
						TargetPos.Y,
						TargetPos.Z,
						Event.TargetAsPhys->Get_Facing()
					);

					if (ExcessHits >= 15) {
						DamageAllowed = false;
						if (ExcessHits >= 30) {
							Kick = true;
							if (Config->BlockDamageEvents) { Kill_Player(ID); }
						}
					}

					vCheatMessage Msg;
					Msg.ID = ID;
					Msg.Type = vCheatMsgType::vROF;
					Msg.WeaponID = WeaponID;
					Msg.EvictAfterMessage = Kick;
					Msg.Hits = Hits;
					Msg.ExpectedHits = ValidHits;
					Msg.HitMeasureTime = MeasureTime;
					vPManager->Add_Cheat_Message(Msg);
				}
			}
		}

		if (Config->DetectDistHack) {
			float Distance = ShooterPos.Distance(TargetPos);
			float ValidDistance = (Ammo->Range.Get() + Ammo->EffectiveRange.Get() + (5.0f * PingFactor));
			if (Distance > ValidDistance && Distance < GameDataObj()->MaxWorldDistance) {
				vLogger->Log(vLoggerType::vCHEAT,"",
					"Player %s shot %s from %.4fft away:",
					p->name.c_str(),
					Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str(),
					Distance
				);
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tShooter Data: \
					\n\t\t\tPreset: %s\
					\n\t\t\tDamage: %.2f \
					\n\t\t\tDistance: %.2f (expected %.2f) \
					\n\t\t\tWarhead: %u (%s) \
					\n\t\t\tWeapon: %s \
					\n\t\t\tAmmunition: %s \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
					Event.DamagerAsPhys->Definition->Get_Name(),
					Event.EvtDamage,
					Distance,
					ValidDistance,
					Event.EvtWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
					Weapon->Definition->Get_Name(),
					Ammo->Get_Name(),
					ShooterPos.X,
					ShooterPos.Y,
					ShooterPos.Z,
					Event.DamagerAsPhys->Get_Facing()
				);
				vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tVictim Data: \
					\n\t\t\tPreset: %s (%s) \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
					\n",
					Event.TargetAsPhys->Definition->Get_Name(),
					Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
					TargetPos.X,
					TargetPos.Y,
					TargetPos.Z,
					Event.TargetAsPhys->Get_Facing()
				);

				vCheatMessage Msg;
				Msg.ID = ID;
				Msg.Type = vCheatMsgType::vRANGE;
				Msg.WeaponID = WeaponID;
				Msg.Time = vManager.DurationAsInt() + 2;
				Msg.Distance = Distance;
				Msg.ExpectedDistance = ValidDistance;
				vPManager->Add_Cheat_Message(Msg);

				DamageAllowed = false;
			}
		}
		
		if (Config->LogSpectator) {
			int SeenTime = p->Get_Seen_Time(Event.TargetAsPhys->NetworkID);
			int Time = (int)clock();
			if (SeenTime == -1 || (int)abs(Time - SeenTime) > 5 * CLOCKS_PER_SEC) {
				if (SeenTime == -1) { 
					vLogger->Log(vLoggerType::vCHEAT,"",
						"Player %s damaged %s that was never seen",
						p->name.c_str(),
						Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str()
					);
				} else {
					vLogger->Log(vLoggerType::vCHEAT,"",
						"Player %s damaged %s visible %.2fs ago",
						p->name.c_str(),
						Get_Pretty_Name((GameObject *)Event.TargetAsPhys).c_str(),
						(float((int)abs(Time - SeenTime)) / float(CLOCKS_PER_SEC))
					);
				}
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tShooter Data: \
					\n\t\t\tPreset: %s\
					\n\t\t\tDamage: %.2f \
					\n\t\t\tWarhead: %u (%s) \
					\n\t\t\tWeapon: %s \
					\n\t\t\tAmmunition: %s \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f",
					Event.DamagerAsPhys->Definition->Get_Name(),
					Event.EvtDamage,
					Event.EvtWarhead,
					Stewie_ArmorWarheadManager::Get_Warhead_Name(Event.EvtWarhead),
					Weapon->Definition->Get_Name(),
					Ammo->Get_Name(),
					ShooterPos.X,
					ShooterPos.Y,
					ShooterPos.Z,
					Event.DamagerAsPhys->Get_Facing()
				);
				vLogger->Log(vLoggerType::vCHEAT,"","\tWeapon List: %s",Get_Weapon_List(Event.DamagerAsPhys->As_ArmedGameObj(),", ").c_str());
				vLogger->Log(vLoggerType::vCHEAT,"",
					"\tVictim Data: \
					\n\t\t\tPreset: %s (%s) \
					\n\t\t\tPosition: %.4f %.4f %.4f (X,Y,Z); facing %.2f \
					\n",
					Event.TargetAsPhys->Definition->Get_Name(),
					Player_Name_From_GameObj(Event.TargetAsPhys).c_str(),
					TargetPos.X,
					TargetPos.Y,
					TargetPos.Z,
					Event.TargetAsPhys->Get_Facing()
				);
			}
		}
	} else {
		Stewie_VehicleGameObj *Veh = Event.Damager->VehicleOccupied;
		if (!Veh) { Veh = Event.DamagerAsPhys->As_VehicleGameObj(); }
		if (Veh && Veh->Definition) {
			Stewie_DamageableGameObjDef *Def = (Stewie_DamageableGameObjDef *)Stewie_DefinitionMgrClass::Find_Definition(Veh->Definition->Get_Name());
			p->Add_Hit(Event.TargetAsPhys->NetworkID,Def->NameID);
		}
	}

	if (Config->BlockDamageEvents == false) { DamageAllowed = true; }
	return DamageAllowed;
}
bool SoldierRDHook(Stewie_SoldierGameObj *Victim, Stewie_OffenseObjectClass &object) {
	if (!GameDataObj()->Is_Gameplay_Permitted()) { return false; }
	if (!Victim || !Victim->As_SoldierGameObj() || Victim->NetworkID <= 0 || object.damage == 0.0f) { return false; }
	if (Victim->VehicleOccupied) { return false; }
	if (Victim->Defence.Get_Health() <= 0.0f) { return false; }
	
	if (Is_Script_Attached((GameObject *)Victim,"Invincibility")) { return false; }

	Stewie_PhysicalGameObj *VictimAsPhys = Victim->As_PhysicalGameObj();
	if (!VictimAsPhys || !VictimAsPhys->Phys) { return false; }
	if (!VictimAsPhys->Phys->Model || !stricmp(VictimAsPhys->Phys->Model->Get_Name(),"NULL")) { return false; }

	float damage = object.damage;
	if (damage < 0.0f) { return true; }
	int warhead = object.warhead;
	if (warhead == 22) { return true; }

	bool IsShooter = true;
	if (!object.dmgr.Reference || !object.dmgr.Reference->obj || (!object.dmgr.Reference->obj->As_SoldierGameObj() && !object.dmgr.Reference->obj->As_VehicleGameObj())) { IsShooter = false; }
	if (IsShooter == false && Config->GameMode == Config->vSNIPER) { return false; }

	if (IsShooter) {
		vPlayer *player = vPManager->Get_Player(Get_Player_ID((GameObject *)Victim));
		if (!player) { return false; }
		player->LastDamager = (GameObject *)object.dmgr.Reference->obj;
		player->LastDamage = damage;
	}

	return true;
}
bool VehicleRDHook(Stewie_VehicleGameObj *Victim, Stewie_OffenseObjectClass &object) {
	if (!GameDataObj()->Is_Gameplay_Permitted()) { return false; }
	if (!Victim || !Victim->As_VehicleGameObj() || Victim->NetworkID <= 0 || object.damage == 0.0f) { return false; }
	if (Victim->Defence.Get_Health() <= 0.0f) { return false; }

	vVehicle *veh = vVManager->Get_Vehicle(Victim->NetworkID);
	if (veh) { veh->Damaged(object.damage); }

	return true;
}
void __stdcall BuildingRDHook(Stewie_BuildingGameObj *Victim, Stewie_OffenseObjectClass &object) {
	if (!GameDataObj()->Is_Gameplay_Permitted()) {
		object.damage = 0.0f;
		return;
	}
	if (object.damage > 0.0f && !CanPlayersFriendlyFire(true) && object.dmgr.Reference && object.dmgr.Reference->obj && object.dmgr.Reference->obj->As_SmartGameObj()->PlayerType == Victim->PlayerType) {
		object.damage = 0.0f;
		return;
	}
	if (Config->GameMode == Config->vINFONLY) {
		if (Victim->As_SoldierFactoryGameObj()) {
			if (object.damage == 2500.0f) {
				object.damage /= 6.0f;
			} else {
				object.damage /= 3.0f;
			}
		}
	}
}
void __stdcall FatalDamageHook(Stewie_DefenseObjectClass& Defense, Stewie_OffenseObjectClass& Offense) {
	Stewie_ScriptableGameObj *objScriptable = NULL, *shooterScriptable = NULL;
	if (Defense.Owner.Reference && Defense.Owner.Reference->obj) { objScriptable = Defense.Owner.Reference->obj; }
	if (Offense.dmgr.Reference && Offense.dmgr.Reference->obj) { shooterScriptable = Offense.dmgr.Reference->obj; }
	if (!objScriptable) { return; }
	if (objScriptable->As_BuildingGameObj()) { return; } // Any building damage registers as fatal...
	if (Defense.Get_Health() > 0.0f) { return; } // Damage is done already; we don't want anything still alive.
	if (Offense.damage == 0.0f) { return; } // Okay, so this isn't actually what killed it - JUST IN CASE.

	float damage = Offense.damage;
	int warhead = Offense.warhead;

	GameObject *obj = (GameObject *)objScriptable;
	GameObject *shooter = (GameObject *)shooterScriptable;

	if (objScriptable->As_SoldierGameObj()) {
		std::string VicName = Player_Name_From_GameObj(objScriptable);
		vPlayer *player = vPManager->Get_Player(VicName.c_str());
		if (!player) { return; }
		int ID = player->ID;
		vPlayer *killer = NULL;

		unsigned int R = 0, G = 0, B = 0;
		if (shooter && shooterScriptable && (shooterScriptable->As_SoldierGameObj() || (shooterScriptable->As_VehicleGameObj() && Get_Vehicle_Occupant_Count(shooter) > 0))) {
			std::string KillerName = Player_Name_From_GameObj(shooterScriptable);
			killer = vPManager->Get_Player(KillerName.c_str());
			Get_Team_Color(Get_Object_Type(shooter),&R,&G,&B);
			bool KillerInVeh = false;
			if (Is_Vehicle(shooter) || shooterScriptable->As_VehicleGameObj()) {
				if (Get_Vehicle_Gunner(shooter)) { shooter = Get_Vehicle_Gunner(shooter); }
				KillerInVeh = true;
			}
			if (Get_Vehicle(shooter)) { KillerInVeh = true; }
			if (KillerInVeh) {
				if (damage == 10000.0f) {
					vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s squished %s (%s VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Preset_Info(obj).c_str());
					PrivColoredVA(0,2,R,G,B,"%s squished %s",KillerName.c_str(),VicName.c_str());
					// MEDALS
					if (killer) {
						killer->SquishesWithoutDying++;
						if (killer->SquishesWithoutDying >= 5) {
							vMManager->Got_Medal(killer->ID,vMManager->vSPLATTERSPREE,true);
							killer->SquishesWithoutDying = 0;
						}
					}
				} else {
					vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Preset_Info(obj).c_str());
					PrivColoredVA(0,2,R,G,B,"%s killed %s",KillerName.c_str(),VicName.c_str());
				}
			} else {
				if (shooter == obj) {
					vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed themself (%s)",VicName.c_str(),Get_Preset_Info(obj).c_str());
					PrivColoredVA(0,2,R,G,B,"%s killed themself (%s)",VicName.c_str(),Get_Pretty_Name(obj).c_str());
				} else if (warhead == 4 && damage == 100) {
					vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s/%s C4 VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Translated_C4_Mode(3).c_str(),Get_Preset_Info(obj).c_str());
					PrivColoredVA(0,2,R,G,B,"%s killed %s (%s C4)",KillerName.c_str(),VicName.c_str(),Get_Translated_C4_Mode(3).c_str());
				} else if (warhead == 12) {
					if (damage == 200.0f) {
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s/%s C4 VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Translated_C4_Mode(2).c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s killed %s (%s C4)",KillerName.c_str(),VicName.c_str(),Get_Translated_C4_Mode(2).c_str());
					} else if (damage == 100.0f) {
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s/%s C4 VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Translated_C4_Mode(1).c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s killed %s (%s C4)",KillerName.c_str(),VicName.c_str(),Get_Translated_C4_Mode(1).c_str());
					} else {
						Get_Team_Color(Get_Object_Type(obj),&R,&G,&B);
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s was killed (%s)",VicName.c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s was killed",VicName.c_str());
					}
				} else if (damage == 2500.0f) {
					if (warhead == 20) {
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s/%s VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Pretty_Name("CnC_POW_IonCannonBeacon_Player").c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s killed %s (%s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name("CnC_POW_IonCannonBeacon_Player").c_str());
					} else if (warhead == 18) {
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s killed %s (%s/%s VS. %s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name(shooter).c_str(),Get_Pretty_Name("CnC_POW_Nuclear_Missle_Beacon").c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s killed %s (%s)",KillerName.c_str(),VicName.c_str(),Get_Pretty_Name("CnC_POW_Nuclear_Missle_Beacon").c_str());
					} else {
						Get_Team_Color(Get_Object_Type(obj),&R,&G,&B);
						vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s was killed (%s)",VicName.c_str(),Get_Preset_Info(obj).c_str());
						PrivColoredVA(0,2,R,G,B,"%s was killed",VicName.c_str());
					}
				} else {
					if (killer) {
						int Hits = 0;
						if (Get_Vehicle(shooter)) {
							Stewie_DamageableGameObjDef *Def = (Stewie_DamageableGameObjDef *)(Stewie_DefinitionMgrClass::Find_Definition(Get_Preset_Name(Get_Vehicle(shooter))));
							Hits = killer->Get_Hits(Commands->Get_ID(obj),1,Def->NameID);
						} else {
							Stewie_WeaponClass *Weapon = Get_Held_Weapon((Stewie_SoldierGameObj *)shooter);
							if (Weapon) {
								Hits = killer->Get_Hits(Commands->Get_ID(obj),1,(Weapon->SecondaryTriggered ? Weapon->Definition->SecondaryAmmoDefId : Weapon->Definition->PrimaryAmmoDefId));
							}
						}
						if (Hits > 0) {
							vLogger->Log(vLoggerType::vVGM,"_PLAYERKILL","%s killed %s (%s VS. %s) - LF: %s; LD: %.2f; Hits: %d",KillerName.c_str(),VicName.c_str(),Get_Preset_Info(shooter).c_str(),Get_Preset_Info(obj).c_str(),(killer->LastFiringMode == 1 ? "P" : "S"),player->LastDamage,Hits);
						} else {
							vLogger->Log(vLoggerType::vVGM,"_PLAYERKILL","%s killed %s (%s VS. %s) - LF: %s; LD: %.2f",KillerName.c_str(),VicName.c_str(),Get_Preset_Info(shooter).c_str(),Get_Preset_Info(obj).c_str(),(killer->LastFiringMode == 1 ? "P" : "S"),player->LastDamage);
						}
					} else {
						vLogger->Log(vLoggerType::vVGM,"_PLAYERKILL","%s killed %s (%s VS. %s) - LD: %.2f",KillerName.c_str(),VicName.c_str(),Get_Preset_Info(shooter).c_str(),Get_Preset_Info(obj).c_str(),player->LastDamage);
					}
					PrivColoredVA(0,2,R,G,B,"%s killed %s",KillerName.c_str(),VicName.c_str());
				}
			}
		} else if (shooter) {
			Get_Team_Color(Get_Object_Type(shooter),&R,&G,&B);
			vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","The %s killed %s (%s)",Get_Pretty_Name(shooter).c_str(),VicName.c_str(),Get_Preset_Info(obj).c_str());
			PrivColoredVA(0,2,R,G,B,"The %s killed %s (%s)",Get_Pretty_Name(shooter).c_str(),VicName.c_str(),Get_Pretty_Name(obj).c_str());
			if (Is_Harvester(shooter)) {
				vMManager->Got_Medal(ID,vMManager->vHARVYSUICIDE,true); // MEDALS
			}
		} else if (warhead == 22 && player->LastFallDistance >= 5.0f) {
			Get_Team_Color(Get_Object_Type(obj),&R,&G,&B);
			vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s fell to their death (%s). Fall distance: %.2f",VicName.c_str(),Get_Preset_Info(obj).c_str(),player->LastFallDistance);
			PrivColoredVA(0,2,R,G,B,"%s fell to their death. Fall distance: %.2f",VicName.c_str(),player->LastFallDistance);
			vMManager->Got_Medal(ID,vMManager->vAIRSUICIDE,true); // MEDALS
		} else if (warhead == 9) {
			Get_Team_Color(Get_Object_Type(obj),&R,&G,&B);
			vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s was killed by Tiberium Poison (%s)",VicName.c_str(),Get_Preset_Info(obj).c_str());
			PrivColoredVA(0,2,R,G,B,"%s was killed by Tiberium Poison",VicName.c_str());
			vMManager->Got_Medal(ID,vMManager->vTIBSUICIDE,true); // MEDALS
		} else {
			Get_Team_Color(Get_Object_Type(obj),&R,&G,&B);
			vLogger->Log(vLoggerType::vVGM,"_PLAYERDEATH","%s was killed (%s)",VicName.c_str(),Get_Preset_Info(obj).c_str());
			PrivColoredVA(0,2,R,G,B,"%s was killed",VicName.c_str());
		}
	} else if (objScriptable->As_VehicleGameObj()) {
		DebugMessage("VEHKILLED; %s",Get_Preset_Name(obj));
	}
}
void __stdcall JumpHook(GameObject *o) {
	Stewie_SoldierGameObj *s = (Stewie_SoldierGameObj *)o;
	s->HumanState.Type = Stewie_HumanStateClass::AIRBORNE;
	vPlayer *player = vPManager->Get_Player(Get_Player_ID(o));
	if (!player) { return; }
	player->LastAirborne = (int)clock();
}
bool __stdcall FallHook(GameObject *o, float height) {
	Stewie_SoldierGameObj *s = (Stewie_SoldierGameObj *)o;
	s->HumanState.Type = Stewie_HumanStateClass::LAND;
	if (Commands->Get_Health(o) <= 0.0f) { return false; }
	if (Is_Script_Attached(o,"Parachute")) {
		Commands->Send_Custom_Event(o,o,1600003,1,0.0f);
		return false;
	}
	if (height > 5) {
		vPlayer *player = vPManager->Get_Player(Player_ID_From_GameObj(s));
		if (!player) { return true; }
		player->LastAirborne = (int)clock();
		float damage = (Commands->Get_Max_Health(o) / 15) * (height - 5);
		if ((Commands->Get_Health(o) - damage) <= 0 && Commands->Get_Health(o) > 0) {
			player->LastDamager = NULL;
			player->LastDamage = damage;
		}
		player->LastFallDistance = height;
	}
	return true;
}
bool __stdcall PowerupCollectedHook(GameObject *soldier, GameObject *powerup) {
	if (!soldier || !powerup) { return false; }
	int ID = Get_Player_ID(soldier);

	if (!stricmp(Get_Model(soldier),"NULL")) { return false; }
	if (Commands->Get_Health(soldier) <= 0.0f) { return false; }

	const char *POWpreset = Get_Preset_Name(powerup);
	if (Config->Crates) {
		if (isin(POWpreset,"_Crate") || isin(POWpreset,"_Uplink") || !stricmp(POWpreset,"POW_Tissue_Nanites")) {
			if (ID == vCManager->Carrier) { return false; }
			Commands->Send_Custom_Event(soldier,powerup,1000000026,0,0.0f);
			goto HandledPickup;
		}
	}

	if (!Is_Vehicle(soldier) && !Get_Vehicle(soldier)) { 
		Stewie_PowerupGameObjDef *PowDef = (Stewie_PowerupGameObjDef *)Stewie_DefinitionMgrClass::Find_Definition(POWpreset);
		if (PowDef && PowDef->GrantWeaponDefID > 0) {
			if (ID == vCManager->Carrier) { return false; }
			return vWManager->Collect(Get_Player_ID(soldier),Commands->Get_ID(powerup));
		} else if (!stricmp(POWpreset,"POW_Backpack")) {
			if (ID == vCManager->Carrier) { return false; }
			bool Collected = vWManager->Collect(Get_Player_ID(soldier),Commands->Get_ID(powerup));
			if (Collected) {
				if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00psbk_aqob0004i1evag_snd.wav"); }
				return true;
			}
			return false;
		} else if (!stricmp(POWpreset,"POW_Stealth_Suit")) {
			if (ID == vCManager->Carrier) { return false; }
			// Full Backpack
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00psbk_aqob0004i1evag_snd.wav"); }
			vWeaponManager::Give_Weapon(ID,"POW_Chaingun_Player");
			vWeaponManager::Give_Weapon(ID,"POW_ChemSprayer_Player");
			vWeaponManager::Give_Weapon(ID,"POW_LaserChaingun_Player");
			vWeaponManager::Give_Weapon(ID,"CnC_MineProximity_05"); // PROBLEM HERE; THIS IS A POWERUP I THINK, BUT IT'S THINKING IT'S A WEAPON...?
			vWeaponManager::Give_Weapon(ID,"CnC_POW_MineRemote_02");
			vWeaponManager::Give_Weapon(ID,"CnC_POW_MineRemote_01");
			vWeaponManager::Give_Weapon(ID,"POW_PersonalIonCannon_Player");
			vWeaponManager::Give_Weapon(ID,"POW_Railgun_Player");
			vWeaponManager::Give_Weapon(ID,"POW_RamjetRifle_Player");
			vWeaponManager::Give_Weapon(ID,"CnC_POW_RepairGun_Player");
			vWeaponManager::Give_Weapon(ID,"CnC_POW_RocketLauncher_Player");
			vWeaponManager::Give_Weapon(ID,"POW_Shotgun_Player");
			vWeaponManager::Give_Weapon(ID,"POW_SniperRifle_Player");
			vWeaponManager::Give_Weapon(ID,"POW_TiberiumFlechetteGun_Player");
			vWeaponManager::Give_Weapon(ID,"POW_VoltAutoRifle_Player");
			vWeaponManager::Give_Weapon(ID,"CnC_POW_Ammo_ClipMax");
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Neuro_Link")) {
			return vCManager->Collected(Get_Player_ID(soldier));
		} else if (!stricmp(POWpreset,"POW_Head_Band")) {
			if (ID == vCManager->Carrier) { return false; }
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00prkv_aqob0001i1gbmg_snd.wav"); }
			Change_Character(soldier,"GDI_Logan_Sheppard");
			Commands->Clear_Weapons(soldier); // Weapon_SniperRifle_Ai_GDI
			Commands->Give_Powerup(soldier,"POW_Pistol_Player",true);
			Commands->Give_Powerup(soldier,"POW_RamjetRifle_Player",true);
			Commands->Give_Powerup(soldier,"POW_RepairGun_Player",true);
			Commands->Give_Powerup(soldier,"CnC_POW_MineTimed_Player_01",true);
			Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)soldier);
			if (bag && bag->Vector.Count() >= 1) { bag->Select_Index(1); }
			Set_Death_Points(soldier,25);
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Ammo_Regeneration")) {
			if (ID == vCManager->Carrier) { return false; }
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00puar_aqob0002i1evag_snd.wav"); }
			Grant_Refill(soldier);
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Tiberium_Shield")) {
			if (ID == vCManager->Carrier) { return false; }
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00puts_aqob0002i1evag_snd.wav"); }
			Commands->Set_Shield_Type(soldier,"SkinChemWarrior");
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Armor_Max")) {
			if (ID == vCManager->Carrier) { return false; }
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00prba_aqob0004i1evag_snd.wav"); }
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Health_Max")) {
			if (ID == vCManager->Carrier) { return false; }
			if (Config->Sounds) { Create_2D_WAV_Sound_Player(soldier,"m00puar_aqob0002i1evag_snd.wav"); }
			goto HandledPickup;
		} else if (!stricmp(POWpreset,"POW_Data_Disc")) {
			return vCManager->CollectedRefineryDisk(Get_Player_ID(soldier),powerup);
		}
	} else {
		return false;
	}
	if ((Is_Vehicle(soldier) && !Get_Vehicle_Driver(soldier)) || Get_Vehicle(soldier)) { return false; }
	if (ID == vCManager->Carrier) { return false; }
	return true;

HandledPickup:
	Commands->Create_Object("Spawner Created Special Effect",Commands->Get_Position(soldier));
	Commands->Destroy_Object(powerup);
	return false;
}
bool __stdcall WantsPowerupsHook(GameObject *soldier) {
	if (!soldier) { return false; }
	Stewie_SoldierGameObj *s = (Stewie_SoldierGameObj *)soldier;
	if (!s->As_SoldierGameObj()) { return false; }
	if (Commands->Get_ID(soldier) <= 0) { return false; }
	if (!stricmp(Get_Model(soldier),"NULL")) { return false; }
	if (Commands->Get_Health(soldier) <= 0.0f) { return false; }

	return s->Is_Human_Controlled();
}

bool CanPlayersFriendlyFire(bool isBuilding) {
	if (isBuilding) {
		return GameDataObj()->IsFriendlyFirePermitted;
	} else {
		bool &FriendlyFire = *(bool *)0x000000; // For security and licensing purposes, an address has been hidden from this line
		return (FriendlyFire ? true : false);
	}
}

bool obb_separation_test(Stewie_ObbCollisionStruct& CollisionCheck, float Extent1, float Extent2, float BeginBoxOffset, float EndBoxOffset) {
	float BeginDistance = abs(BeginBoxOffset) - Extent1 - Extent2;
	float EndDistance = abs(EndBoxOffset) - Extent1 - Extent2;

	if (BeginDistance >= 0) {
		CollisionCheck.IsColliding = false;

		if (EndDistance >= 0) {
			CollisionCheck.MoveDistance = 1;
			CollisionCheck.CollisionDirection = BeginBoxOffset >= EndBoxOffset ? 1 : -1;
			return 1;
		}

		if (EndDistance == BeginDistance) { return false; }
		if (BeginDistance / (BeginDistance - EndDistance) >= CollisionCheck.MoveDistance) {
			CollisionCheck.AxisId = CollisionCheck.TestAxisId;
			CollisionCheck.CollisionDirection = BeginBoxOffset >= EndBoxOffset ? 1 : -1;
			CollisionCheck.MoveDistance = BeginDistance / (BeginDistance - EndDistance);
			return 0;
		}
		if (((CollisionCheck.CollisionDirection + 1) & ~2) != 0) { return false; }
	}
	return 0;
}
bool obb_check_box0_basis(Stewie_ObbCollisionStruct& CollisionCheck, int AxisIndex) {
	float A = CollisionCheck.Box1.Extent[AxisIndex];
	float B = abs(CollisionCheck.Box2.Extent.X * CollisionCheck.SeparationAxes[AxisIndex].X) + abs(CollisionCheck.Box2.Extent.Y * CollisionCheck.SeparationAxes[AxisIndex].Y) + abs(CollisionCheck.Box2.Extent.Z * CollisionCheck.SeparationAxes[AxisIndex].Z);
	float C = CollisionCheck.BeginOffset.DotProduct(CollisionCheck.Box1Axes[AxisIndex]);
	float D = CollisionCheck.EndOffset.DotProduct(CollisionCheck.Box1Axes[AxisIndex]);
	return obb_separation_test(CollisionCheck, A, B, C, C + D);
}
bool obb_check_box1_basis(Stewie_ObbCollisionStruct& CollisionCheck, int AxisIndex) {
	float A = abs(CollisionCheck.Box1.Extent.X * CollisionCheck.SeparationAxes[0][AxisIndex]) + abs(CollisionCheck.Box1.Extent.Y * CollisionCheck.SeparationAxes[1][AxisIndex]) + abs(CollisionCheck.Box1.Extent.Z * CollisionCheck.SeparationAxes[2][AxisIndex]);
	float B = CollisionCheck.Box2.Extent[AxisIndex];
	float C = CollisionCheck.BeginOffset.DotProduct(CollisionCheck.Box2Axes[AxisIndex]);
	float D = CollisionCheck.EndOffset.DotProduct(CollisionCheck.Box2Axes[AxisIndex]);
	return obb_separation_test(CollisionCheck, A, B, C, C + D);
}
