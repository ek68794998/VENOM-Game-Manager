#include "vgm_agtfix.h"
	
void GDI_AGT::Created(GameObject* AGTObj) {
	/* Due to licensing, portions of this file have been omitted. */
}


void GDI_AGT::Killed(GameObject* AGTObj, GameObject* KillerObj) {
	for (int I = 0; I < 4; I++) {
		Commands->Destroy_Object(Commands->Find_Object(GunID[I]));
	}
	Commands->Destroy_Object(Commands->Find_Object(MissileID));
}

void GDI_AGT::Custom(GameObject* AGTObj, int Message, int Param, GameObject* Sender) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Gun::Created(GameObject* GunObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Gun::Destroyed(GameObject* GunObj) {
	Commands->Action_Reset(GunObj, 100);
}

void GDI_AGT_Gun::Enemy_Seen(GameObject* GunObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Gun::Timer_Expired(GameObject* GunObj, int Number) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Gun::Custom(GameObject* MissileObj, int Message, int Param, GameObject* SenderObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

bool GDI_AGT_Gun::IsValidEnemy(GameObject* GunObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Missile::Created(GameObject* MissileObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Missile::Destroyed(GameObject* MissileObj) {
	Commands->Action_Reset(MissileObj, 100);
}

void GDI_AGT_Missile::Timer_Expired(GameObject* MissileObj, int Number) {
	/* Due to licensing, portions of this file have been omitted. */
}

void GDI_AGT_Missile::Custom(GameObject* MissileObj, int Message, int Param, GameObject* SenderObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

bool GDI_AGT_Missile::IsValidEnemy(GameObject* MissileObj, GameObject* EnemyObj) {
	/* Due to licensing, portions of this file have been omitted. */
}

ScriptRegistrant<GDI_AGT> M00_Advanced_Guard_Tower_Registrant("M00_Advanced_Guard_Tower","");
ScriptRegistrant<GDI_AGT> GDI_AGT_Registrant("GDI_AGT","");
ScriptRegistrant<GDI_AGT_Gun> GDI_AGT_Gun_Registrant("GDI_AGT_Gun","");
ScriptRegistrant<GDI_AGT_Missile> GDI_AGT_Missile_Registrant("GDI_AGT_Missile","");