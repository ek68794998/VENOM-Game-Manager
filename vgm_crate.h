#pragma once

#include "vgm_engine.h"
#include "vgm_actions.h"
#include "vgm_game.h"
#include "vgm_obj.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

extern bool CrateExists;

void Crate_Init();
void Place_Crate_Randomly(GameObject *obj);
std::string Pick_Random_Crate(int type);

class M00_CNC_Crate : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
};
class vCrateHandler : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Custom(Stewie_ScriptableGameObj *obj, int message, int param, Stewie_ScriptableGameObj *sender);
	bool Executed;
	bool PickedUp;
};
class Crate_Creator : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Power_Down : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Crazy_Defense : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class GG : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
	void Killed(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *shooter);
	int AttackTeam;
	int VictimTeam;
	const char *AttackType;
};

class God : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Hover : public Stewie_ScriptImpClass {
};

class Invincibility : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Kamikaze : public Stewie_ScriptImpClass {
};

class AntiMine : public Stewie_ScriptImpClass {
};

class AntiEnemy : public Stewie_ScriptImpClass {
};

class EnemyThief : public Stewie_ScriptImpClass {
};

class Locked : public Stewie_ScriptImpClass {
};

class NoC4Allowed : public Stewie_ScriptImpClass {
};

class EMP : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Fly : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class Radar : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};

class RandNuke : public Stewie_ScriptImpClass {
	void Created(Stewie_ScriptableGameObj *obj);
	void Timer_Expired(Stewie_ScriptableGameObj *obj, int number);
};
