#include "vgm_obj.h"
#include "vgm_presets.h"
#include "vgm_vehicle.h"

vVehicleManager *vVManager;

// ---------------------------
//
//  Vehicle Manager
//
// ---------------------------
vVehicle::vVehicle(int ID, int o) {
	ObjID = ID;
	Lock(o,0);
	Damaged(0.0f);
	WasFlipKilled = false;
}
vVehicle::~vVehicle() {
	DebugMessage("VEHICLE OBJ KILLED %d",this->ObjID);
}
void vVehicle::Set_Owner(int ID) {
	if (ID > 0 && Player_GameObj(ID)) { Owner = ID; }
	else { Owner = 0; }
}
void vVehicle::Destroy() {
	delete this;
}

void vVehicleManager::Init() {
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		this->Data[i]->Destroy();
	}
	this->Data.clear();
}
int vVehicleManager::Count_Player_Bound_Vehicles(int ID, bool enemy) {
	if (Stewie_cPlayerManager::Is_Player_Present(ID) == false || this->Data.size() <= 0) { return 0; }
	int count = 0;
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (this->Data[i]->Owner == ID && this->Data[i]->Locked > 0) {
			GameObject *o = Commands->Find_Object(this->Data[i]->ObjID);
			if (!o) { continue; }
			GameObject *s = Player_GameObj(ID);
			if (enemy == false && Get_Vehicle_Parent_Type(o) == Commands->Get_Player_Type(s)) { count++; }
			else if (enemy && Get_Vehicle_Parent_Type(o) != Commands->Get_Player_Type(s)) { count++; }
		}
	}
	return count;
}
std::string vVehicleManager::Get_Bound_Vehicles(int ID) {
	if (Stewie_cPlayerManager::Is_Player_Present(ID) == false || this->Data.size() <= 0) { return NULL; }
	char q[512];
	sprintf(q,"");
	const char *mt;
	bool f = true;
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (this->Data[i]->Owner == ID && this->Data[i]->Locked > 0) {
			GameObject *o = Commands->Find_Object(this->Data[i]->ObjID);
			if (!o) { continue; }
			if (f) {
				sprintf(q,"%s",Get_Pretty_Name(o).c_str());
				f = false;
			} else {
				mt = (const char *)q;
				sprintf(q,"%s, %s",mt,Get_Pretty_Name(o).c_str());
			}
		}
	}
	if (strlen(q) < 2) { sprintf(q,"None"); }
	std::string Retn = q;
	return Retn;
}
void vVehicleManager::Unbind_All_Player_Vehicles(int ID) {
	if (this->Data.size() <= 0) { return; }
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (this->Data[i]->Owner == ID) {
			this->Data[i]->Lock(0,0);
		}
	}
}
void vVehicleManager::Delete_Vehicle(Stewie_ScriptableGameObj *obj) {
	Delete_Vehicle(obj->NetworkID);
}
void vVehicleManager::Delete_Vehicle(int ID) {
	if (!ID || this->Data.size() <= 0) { return; }
	for (unsigned int i = 0; i < this->Data.size(); i++) {
		if (!this->Data[i]) { continue; }
		if (this->Data[i]->ObjID == ID) {
			this->Data[i]->Destroy();
			this->Data.erase(this->Data.begin() + i);
			break;
		}
	}
}

// ---------------------------
//
//  Vehicle Hooks
//
// ---------------------------
void __stdcall VehicleCreateHook(GameObject *VehObj, GameObject *Owner) {
	if (!Owner || !VehObj) { return; }
	vLogger->Log(vLoggerType::vVGM,"_VEHPURCHASE","%s %s purchased a %s",Get_Translated_Team_Name(Commands->Get_Player_Type(Owner)).c_str(),Player_Name_From_GameObj((Stewie_BaseGameObj *)Owner).c_str(),Get_Pretty_Name(VehObj).c_str());
	Commands->Set_Player_Type(VehObj,Commands->Get_Player_Type(Owner));

	int ID = Get_Player_ID(Owner);
	vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(VehObj));
	if (veh) {
		int c = vVManager->Count_Player_Bound_Vehicles(ID,false);
		if (c < Config->MaxFriendlyVeh) {
			veh->Lock(ID,1);
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The %s you purchased has been automatically bound to you.",Get_Pretty_Name(VehObj).c_str());
		} else {
			veh->Lock(ID,0);
		}
	}

	if (Config->VetEnabled) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (p) {
			vVetManager->Upgrade(VehObj,p->VetRank - 1,2);
			float bonus = vVetManager->Promo_ArmorBonusGained[p->VetRank - 1];
			if (bonus > 0.0f) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The %s you purchased has been upgraded %.0f%% for your veteran status.",Get_Pretty_Name(VehObj).c_str(),bonus);
			}
			
			float Refund = float(p->VetRank) * (100.0f / float(Config->VetLevels)) - 20.0f;
			if (Refund > 0.0f && Get_Cost(Get_Preset_Name(VehObj)) > 0) {
				Commands->Give_Money(Owner,(Refund / 100.0f) * Get_Cost(Get_Preset_Name(VehObj)),false);
				PrivMsgColoredVA(Get_Player_ID(Owner),2,0,200,0,"[VGM] You have been refunded %.0f%% (%.0f credits) for your %s.",Refund,(Refund / 100.0f) * Get_Cost(Get_Preset_Name(VehObj)),Get_Pretty_Name(VehObj).c_str());
			}
		}
	}
}
void __stdcall VehicleFlipHook(Stewie_VehicleGameObj *Veh) {
	if (!Veh || !Veh->Definition) { return; }
	vVehicle *v = vVManager->Get_Vehicle(Veh->NetworkID);
	if (v) { v->WasFlipKilled = true; }
}

// ---------------------------
//
//  Vehicle Functions
//
// ---------------------------
Stewie_SoldierGameObj *Vehicle_Driver(Stewie_VehicleGameObj *obj) {
	if (!obj->Is_Human_Controlled()) { return NULL; }
	if (!Vehicle_Occupants(obj)[0]) { return NULL; }
	return Vehicle_Occupants(obj)[0]->As_SoldierGameObj();
}
Stewie_VectorClass<Stewie_SoldierGameObj *>& Vehicle_Occupants(Stewie_VehicleGameObj *obj) {
	Stewie_VectorClass<Stewie_SoldierGameObj *>& occupants = (Stewie_VectorClass<Stewie_SoldierGameObj *>&)obj->Occupants;
	return occupants;
}
void Set_Vehicle_Is_Visible2(GameObject *obj, bool visible) {
	bool *b = (bool *)obj;
	b += 0x995;
	*b = visible;
	if (visible) { Attach_Script_Once(obj,"Invisible",""); }
	else { Remove_Script(obj,"Invisible"); }
}
int Find_First_Available_Seat(GameObject *vehicle, bool driverOK) {
	if (!vehicle || !Commands->Get_ID(vehicle)) { return 0; }
	int t = 1;
	if (driverOK) { t = 0; }
	for (int i = t; i < Get_Vehicle_Seat_Count(vehicle); i++) {
		if (!Get_Vehicle_Occupant(vehicle,i)) { return i; }
	}
	return -1;
}
void Add_Occupant(GameObject *o, GameObject *v) {
	if (Get_Vehicle(o) || !Is_Vehicle(v)) { return; }
	//Stewie_SoldierGameObj *Obj = (Stewie_VehicleGameObj *)o;
	//Stewie_VehicleGameObj *Veh = (Stewie_VehicleGameObj *)v;
	//Veh->Add_Occupant(Obj);
	__asm {
		mov ebx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov ecx,v
		push o
		call ebx
	}
}
void Add_Occupant_Seat(GameObject *o, GameObject *v, int seat) {
	if (!o || !v || Get_Vehicle(o) || !Is_Vehicle(v) || seat < 0) { return; }
	//Stewie_SoldierGameObj *Obj = (Stewie_VehicleGameObj *)o;
	//Stewie_VehicleGameObj *Veh = (Stewie_VehicleGameObj *)v;
	//Veh->Add_Occupant_Seat(Obj,seat);
	__asm {
		mov ebx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov ecx,v
		push seat
		push o
		call ebx
	}
}
void Force_Occupant_ID_Exit(GameObject *obj, int Player) {
	if (!Commands->Get_ID(obj) || !obj || !As_VehicleGameObj(obj)) {
		return;
	}
	VectorClass<GameObject *> *ptr = (VectorClass<GameObject *>*)(obj+0x9AC);
	VectorClass<GameObject *> occupants = (VectorClass<GameObject *>)*ptr;
	int x = occupants.Length();
	for (int i = 0; i < x; i++) {
		if (occupants[i] && Get_Player_ID(occupants[i]) == Player) {
			Check_Transitions(occupants[i],true);
		}
	}
}
void Force_Occupants_Exit_Except(GameObject *obj, int Except) {
	if (!Commands->Get_ID(obj) || !obj || !As_VehicleGameObj(obj)) {
		return;
	}
	VectorClass<GameObject *> *ptr = (VectorClass<GameObject *>*)(obj+0x9AC);
	VectorClass<GameObject *> occupants = (VectorClass<GameObject *>)*ptr;
	int x = occupants.Length();
	for (int i = 0; i < x; i++) {
		if (occupants[i] && Get_Player_ID(occupants[i]) != Except) {
			Check_Transitions(occupants[i],true);
		}
	}
}
