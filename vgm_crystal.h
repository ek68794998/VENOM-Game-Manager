#pragma once

#include "vgm_engine.h"
#include "vgm_actions.h"
#include "vgm_game.h"
#include "vgm_obj.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vCrystalHandler;
class vCrystalManager;

extern vCrystalManager *vCManager;

class vCrystalHandler : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
	bool Executed;
	bool PickedUp;
};
class Crystal_Creator : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};
class Crystal_Damager : public ScriptImpClass {
	void Created(GameObject *obj);
	void Timer_Expired(GameObject *obj, int number);
};

class vCrystalManager {
public:
	int ObjID;
	int Stage;
	float Value;
	float DistribValue;
	unsigned int WillExpire;

	int Carrier;
	std::vector<Stewie_WeaponDefinitionClass*> CarrierWeapons;

	float FullValue;
	float FullDistribValue;
	int GrowTimeMin;
	int GrowTimeMax;
	int DecrementWhenDropped;
	int ExpiryTime;
	float HealthLossTime;
	bool SBHCanCarry;
	bool CanEnterVehicle;
	bool CanRefill;
	int MaxStage;

	Stewie_PhysicalGameObj *GDIDisk;
	Stewie_PhysicalGameObj *NodDisk;

	Stewie_ScriptableGameObj *Controller;
	Stewie_PhysicalGameObj *Bone;
	std::vector<std::string> ModelList;
	std::vector<Stewie_Vector3> SpawnPositions;

	// dsp_tibsingle = Mini, 1 crystal
	// dsp_tibsingl2 = Mini, 1 crystal
	// dsp_stibduble = Mini, 2 crystals
	// dsp_mtibtriple = Kinda Mini, 3 crystals
	// dsp_ltibquad = Medium, 4 crystals
	// dsp_tibgrande = Giant

public:
	void Init();
	void Think(bool FullSecond);
	void Load();
	void Spawn();
	void Grow() { SetStage(GetStage() < 0 ? 0 : GetStage() + 1); };
	void Reset() {
		Carrier = -1;
		SetStage(-1);
	};
	void Evaluate();
	void Expire();
	void Drop();
	bool Collected(int ID);
	bool CollectedRefineryDisk(int ID, GameObject *disk);

	Stewie_PhysicalGameObj* GetDisk(int Team) {
		if (Team == 1) { return GDIDisk; }
		if (Team == 0) { return NodDisk; }
		return NULL;
	}
	void SetObj(GameObject *c) {
		if (!c) { return; }
		ObjID = Commands->Get_ID(c);
	}
	int GetStage() { return Stage; }
	void SetStage(int NStage) {
		Stage = NStage;
		Evaluate();
	}
};
