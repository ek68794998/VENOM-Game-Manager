#include "vgm_obj.h"
#include "vgm_obeliskfix.h"

void Nod_Obelisk_CnC::Created(GameObject* ObeliskObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Nod_Obelisk_CnC::Killed(GameObject* ObeliskObj, GameObject* Killer) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Nod_Obelisk_CnC::Custom(GameObject* ObeliskObj, int Message, int Param, GameObject* Sender) {
	/* Due to licensing, portions of this file have been omitted. */
}


void Obelisk_Weapon_CnC::Created(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::Destroyed(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

bool Obelisk_Weapon_CnC::IsValidEnemy(GameObject* WeaponObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::StartFiring(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::StopFiring(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::StartEffect(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}
void Obelisk_Weapon_CnC::StopEffect(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::FireAt(GameObject* WeaponObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::StopFireAt(GameObject* WeaponObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::Timer_Expired(GameObject* WeaponObj, int Number) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::Enemy_Seen(GameObject* WeaponObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void Obelisk_Weapon_CnC::Register_Auto_Save_Variables() {
	Auto_Save_Variable(1, 4, &EnemyID);
	Auto_Save_Variable(2, 4, &EffectID);
	Auto_Save_Variable(3, 1, &Firing);
	Auto_Save_Variable(4, 1, &Charged);
}

ScriptRegistrant<Nod_Obelisk_CnC> M00_Nod_Obelisk_CnC_Registrant("M00_Nod_Obelisk_CnC", "");
ScriptRegistrant<Nod_Obelisk_CnC> Nod_Obelisk_CnC_Registrant("Nod_Obelisk_CnC", "");
ScriptRegistrant<Obelisk_Weapon_CnC> Obelisk_Weapon_CnC_Registrant("Obelisk_Weapon_CnC", "");