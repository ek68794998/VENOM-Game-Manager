#pragma once

#include "vgm_engine.h"
#include "vgm_obj.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class GDI_AGT : public ScriptImpClass {
	int MissileID;
	int GunID[4];

	void Created(GameObject* AGTObj);
	void Killed(GameObject* AGTObj, GameObject* KillerObj);
	void Custom(GameObject* AGTObj, int Message, int Param, GameObject* Sender);
};

class GDI_AGT_Gun : public ScriptImpClass {
	int MissileID;
	int EnemyID;
	
	void Created(GameObject* GunObj);
	void Destroyed(GameObject* GunObj);
	void Enemy_Seen(GameObject* GunObj, GameObject* EnemyObj);
	void Timer_Expired(GameObject* GunObj, int Number);
	void Custom(GameObject* MissileObj, int Message, int Param, GameObject* SenderObj);

	bool IsValidEnemy(GameObject* GunObj, GameObject* EnemyObj);
};

class GDI_AGT_Missile : public ScriptImpClass {
	int EnemyID;
	
	void Created(GameObject* MissileObj);
	void Destroyed(GameObject* MissileObj);
	void Timer_Expired(GameObject* MissileObj, int Number);
	void Custom(GameObject* MissileObj, int Message, int Param, GameObject* SenderObj);

	bool IsValidEnemy(GameObject* WeaponObj, GameObject* EnemyObj);
};