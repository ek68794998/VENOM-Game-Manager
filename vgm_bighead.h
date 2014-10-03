#pragma once

#include "vgm_engine.h"
#include "vgm_obj.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vBigheadObserver : public Stewie_ScriptImpClass {
public:
	/* Due to licensing, portions of this file have been omitted. */

public:
	vBigheadObserver();
	const char* Get_Name();
	void Created(Stewie_ScriptableGameObj* Observee);
	void Killed(Stewie_ScriptableGameObj *Observee, Stewie_ScriptableGameObj *Killer);
	void Detach(Stewie_ScriptableGameObj* Observee);
	void Destroyed(Stewie_ScriptableGameObj* Observee);
	/* Due to licensing, portions of this file have been omitted. */
};
