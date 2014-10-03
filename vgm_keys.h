#pragma once

#include "vgm_engine.h"
#include "vgm_obj.h"
#include "vgm_game.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

void KeyHookCall(void *Data);

class vKeyHook : public ScriptImpClass {
public:
	KeyHookStruct *k;
	int hookid,is_keyhook_set;
	int AddNewKeyHook(const char *Key, GameObject *obj);
	void RemoveHook();
	void Detach(GameObject *obj);
	void Destroyed(GameObject *obj);
	virtual void KeyHookFunc() = 0;
};
class vTauntKey : public vKeyHook {
	void Created(GameObject *obj);
	void KeyHookFunc();
};
class vRadioKey : public vKeyHook {
	void Created(GameObject *obj);
	void KeyHookFunc();
	int i;
	int LastPress;
};
class vChatKey : public vKeyHook {
	void Created(GameObject *obj);
	void KeyHookFunc();
};
