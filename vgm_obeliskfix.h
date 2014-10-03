#pragma once

#include "vgm_engine.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class Nod_Obelisk_CnC : public ScriptImpClass {
	int WeaponID;

	void Created(GameObject* ObeliskObj);
	void Killed(GameObject* ObeliskObj, GameObject* Killer);
	
	void Custom(GameObject* ObeliskObj, int Message, int Param, GameObject* Sender);
};

class Obelisk_Weapon_CnC : public ScriptImpClass {
	int EnemyID;
	int EffectID;
	bool Firing;
	bool Charged;
	
	int LastEnemyID;
	float LastEnemyHealth;

	void Created(GameObject* WeaponObj);
	void Destroyed(GameObject* WeaponObj);
	bool IsValidEnemy(GameObject* WeaponObj, GameObject* EnemyObj);

	void StartFiring(GameObject* WeaponObj);
	void StopFiring(GameObject* WeaponObj);
	void StartEffect(GameObject* WeaponObj);
	void StopEffect(GameObject* WeaponObj);
	void FireAt(GameObject* WeaponObj, GameObject* EnemyObj);
	void StopFireAt(GameObject* WeaponObj);
	void Timer_Expired(GameObject* WeaponObj, int Number);
	void Enemy_Seen(GameObject* WeaponObj, GameObject* EnemyObj);
	void Register_Auto_Save_Variables();
};