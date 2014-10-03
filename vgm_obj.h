#pragma once

#include "vgm_engine.h"
#include "vgm_actions.h"
#include "vgm_bighead.h"
#include "vgm_medals.h"
#include "vgm_weapon.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vObjects;
class vAimbotManager;
struct DamageEvent;
struct AimbotObj;

extern vAimbotManager *vAManager;

class vObjects {
public:
	static bool Valid(Stewie_BaseGameObj *o);
	static Stewie_BaseGameObj* Get_Object(int ID);
	static bool Is_Harvester(Stewie_BaseGameObj *o);
	static bool Is_Drivable_Vehicle(Stewie_BaseGameObj *obj);
};

class vAimbotManager {
public:
	void Init();
	void Think(bool FullSecond);
	void Played_Joined(int PlayerID);
	bool Create(int Target);
	std::vector<AimbotObj> Data;
	bool CreateQueue[128];
};

const char *Get_Preset_Name(GameObject *obj);
Stewie_DefinitionClass *Find_Random(unsigned int ClassID, int Team = 2);
Stewie_DefinitionClass *Get_Powerup_For_Weapon(const char *weapon);
Stewie_BaseGameObjDef *Get_Object_Definition(GameObject *obj);
Stewie_SoldierGameObjDef *Get_Soldier_Definition(GameObject *obj);
Stewie_VehicleGameObjDef *Get_Vehicle_Definition(GameObject *obj);

Stewie_BuildingGameObj *Team_Building(int Team, vBUILDINGTYPE Type);
Stewie_BuildingGameObj *Team_Base_Defense(int Team);
Stewie_SoldierFactoryGameObj *Team_Barracks(int Team);
Stewie_VehicleFactoryGameObj *Team_War_Factory(int Team);
Stewie_RefineryGameObj *Team_Refinery(int Team);
Stewie_PowerPlantGameObj *Team_Power_Plant(int Team);
Stewie_VehicleGameObj *Team_Harvester(int team);

int Get_Vehicle_Parent_Type(GameObject *obj);
int Get_Soldier_State(GameObject *obj);
bool Is_Soldier_Airborne(GameObject *obj);
void Set_Model(Stewie_BaseGameObj *obj, const char *preset);
void Set_Character(Stewie_SoldierGameObj *obj, const char *preset);
float Get_Total_Health(Stewie_BaseGameObj *obj);
float Get_Total_Max_Health(Stewie_BaseGameObj *obj);

Stewie_Vector3 World_Position(Stewie_BaseGameObj *o);
Stewie_BaseGameObj *Create_Object(const char *Preset, Stewie_Vector3 Position = Stewie_Vector3(), bool Collisions = true);
void Create_Explosion(const char *Preset, Stewie_Vector3 Position, Stewie_ScriptableGameObj *Damager);
Stewie_ScriptableGameObj *Set_Up_SAM_Site(int team, Stewie_Vector3 Position);
void Set_Is_Visible(Stewie_ScriptableGameObj *obj, bool visible);

float Find_Distance_To_Closest_Object_By_Preset(Vector3 &pos, const char *preset);
void Kill_C4(GameObject *obj);
void Kill_C4(Stewie_C4GameObj *obj);
Stewie_SoldierGameObj *Get_C4_Owner(Stewie_C4GameObj *o);
int Get_C4_Type(Stewie_C4GameObj *o);
Stewie_C4GameObj *Find_First_C4_By_Player(int Owner, int Type);
int Get_Player_C4_Count(GameObject *Owner, int Type);
int Get_C4_Count_Type(int Team, int Type);
int Get_C4_Count_Timed(int Team);
int Get_Proxy_C4s_Attached_By_Building(GameObject *building, int Team);
std::string Find_Proxy_C4_On_Buildings(int Team);
int Get_C4_Limit_Type(int team, int C4Type);
int Get_Player_Beacon_Count(int PlayerID);
void Destroy_All_Objects_By_Preset(int Team, const char *Preset);
GameObject *Find_Random_Player_By_Team(int Team);
GameObject *Find_Random_Building_By_Team(int Team, bool alive);
GameObject *Find_Closest_Building_By_Team(int Team, Vector3 position);

void Update_Dirty_Bits(Stewie_NetworkObjectClass* obj);

bool Has_Observer(Stewie_ScriptableGameObj& obj, const char* scn);
void Remove_Observer(Stewie_ScriptableGameObj& obj, const char* scn);

void __stdcall ObjectCreationHook(Stewie_ScriptableGameObj *obj);
bool __stdcall DoorFixHook(Stewie_SmartGameObj *Object, Stewie_DoorPhysClass *Door);
bool CheckDamageEvent(DamageEvent& Event);
bool __stdcall DamageHook(Stewie_cCsDamageEvent& dmgevent);
bool SoldierRDHook(Stewie_SoldierGameObj *Victim, Stewie_OffenseObjectClass &object);
bool VehicleRDHook(Stewie_VehicleGameObj *Victim, Stewie_OffenseObjectClass &object);
void __stdcall BuildingRDHook(Stewie_BuildingGameObj *Victim, Stewie_OffenseObjectClass &object);
void __stdcall FatalDamageHook(Stewie_DefenseObjectClass& Defense, Stewie_OffenseObjectClass& Offense);
void __stdcall JumpHook(GameObject *o);
bool __stdcall FallHook(GameObject *o, float height);
bool __stdcall PowerupCollectedHook(GameObject *soldier, GameObject *powerup);
bool __stdcall WantsPowerupsHook(GameObject *soldier);

bool CanPlayersFriendlyFire(bool isBuilding);

bool obb_separation_test(Stewie_ObbCollisionStruct& CollisionCheck, float Extent1, float Extent2, float BeginBoxOffset, float EndBoxOffset);
bool obb_check_box0_basis(Stewie_ObbCollisionStruct& CollisionCheck, int AxisIndex);
bool obb_check_box1_basis(Stewie_ObbCollisionStruct& CollisionCheck, int AxisIndex);

struct DamageEvent {
	int PlayerId;
	Stewie_SoldierGameObj *Damager;
	Stewie_PhysicalGameObj *DamagerAsPhys;
	Stewie_PhysicalGameObj *TargetAsPhys;
	float EvtDamage;
	unsigned int EvtWarhead;
	bool IsDamagerVehicle;
};
struct AimbotObj {
	Stewie_cPlayer *Owner;
	Stewie_cPlayer *FakePlayer;
	int ObjID;
	bool CanDelete;
};
