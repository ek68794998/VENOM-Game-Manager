#pragma once

#include "vgm_engine.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

#define Is_VTOL_Vehicle(X) Is_VTOLVehicle(X)

class vVehicle;
class vVehicleManager;

extern vVehicleManager *vVManager;

void __stdcall VehicleCreateHook(GameObject *VehObj, GameObject *Owner);
void __stdcall VehicleFlipHook(Stewie_VehicleGameObj *Veh);

Stewie_SoldierGameObj *Vehicle_Driver(Stewie_VehicleGameObj *obj);
Stewie_VectorClass<Stewie_SoldierGameObj *>& Vehicle_Occupants(Stewie_VehicleGameObj *obj);
void Set_Vehicle_Is_Visible2(GameObject *obj, bool visible);
GameObject *Find_Closest_Vehicle(Vector3 pos, int team);
int Find_First_Available_Seat(GameObject *vehicle, bool driverOK);
void Add_Occupant(GameObject *o, GameObject *v);
void Add_Occupant_Seat(GameObject *o, GameObject *v, int seat);
void Force_Occupant_ID_Exit(GameObject *obj, int Player);
void Force_Occupants_Exit_Except(GameObject *obj, int Except);

class vVehicle {
public:
	int ObjID;
	int Locked; // 0 = None, 1 = Bound, 2 = Locked
	int Owner; // Player ID
	float LastDamage;
	bool WasFlipKilled;

	vVehicle(int ID, int o);
	~vVehicle();

	void Damaged(float damage) { LastDamage = damage; }
	void Lock(int owner, int state) {
		Set_Owner(owner);
		Set_Lock_State(state);
	}
	void Set_Lock_State(int Lock) {
		if (Lock == 1 || Lock == 2) { Locked = Lock; }
		else { Locked = 0; }
		GameObject *o = Commands->Find_Object(ObjID);
		Commands->Send_Custom_Event(o,o,1000,Locked,0.0f);
	}
	void Set_Owner(int ID);
	void Destroy();
};

class vVehicleManager {
public:
	std::vector<vVehicle *> Data;

public:
	void Init();
	unsigned int Get_Vehicle_Count() { return this->Data.size(); }
	vVehicle *Add_Vehicle(Stewie_ScriptableGameObj *obj) {
		if (!obj) { return NULL; }
		if (this->Get_Vehicle(obj->NetworkID)) { return NULL; }
		vVehicle *veh = new vVehicle(obj->NetworkID,0);
		this->Data.push_back(veh);
		return veh;
	}
	vVehicle *Get_Vehicle(int ID) {
		if (this->Data.size() <= 0) { return NULL; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			if (this->Data[i]->ObjID == ID) { return this->Data[i]; }
		}
		return NULL;
	}
	int Count_Player_Bound_Vehicles(int ID, bool enemy);
	std::string Get_Bound_Vehicles(int ID);
	void Kick_All_Players_From_My_Vehicles(int ID) {
		if (this->Data.size() <= 0) { return; }
		for (unsigned int i = 0; i < this->Data.size(); i++) {
			if (!this->Data[i]) { continue; }
			if (this->Data[i]->Owner == ID && this->Data[i]->Locked > 0) {
				Force_Occupants_Exit_Except(Commands->Find_Object(this->Data[i]->ObjID),ID);
			}
		}
	}
	void Unbind_All_Player_Vehicles(int ID);
	void Delete_Vehicle(Stewie_ScriptableGameObj *obj);
	void Delete_Vehicle(int ID);
};
