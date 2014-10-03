#include "vgm_keys.h"

void KeyHookCall(void *Data) {
	vKeyHook *Base = (vKeyHook *)Data;
	Base->KeyHookFunc();
}
int vKeyHook::AddNewKeyHook(const char *Key, GameObject *obj) {
	k = 0;
	k = new KeyHookStruct;
	k->data = this;
	k->hook = KeyHookCall;
	k->PlayerID = Get_Player_ID(obj);
	k->key = strdup(Key);
	if (is_keyhook_set == 1337) { RemoveHook(); }
	hookid = AddKeyHook(k);
	return hookid;
}
void vKeyHook::RemoveHook() {
	if (hookid != 0 && RemoveKeyHook != 0) {
		RemoveKeyHook(hookid);
		hookid = 0;
		if (k != 0) {
			delete k;
			k = 0;
		}
	}
}
void vKeyHook::Detach(GameObject *obj) {
	Destroyed(obj);
}
void vKeyHook::Destroyed(GameObject *obj) {
	if (is_keyhook_set == 1337) { RemoveHook(); }
}

void vTauntKey::Created(GameObject *obj) {
	if (is_keyhook_set != 1337) {
		hookid = AddNewKeyHook(Get_Parameter("Key"),obj);
		is_keyhook_set = 1337;
	}
}
void vTauntKey::KeyHookFunc() {
	vPlayer *p = vPManager->Get_Player(Get_Player_ID(Owner()));
	if (!p) { return; }
	if ((int)clock() - p->LastAirborne <= 1 * CLOCKS_PER_SEC) { return; }
	if (Get_Vehicle(Owner())) { return; }
	if (Is_Soldier_Airborne(Owner())) { p->LastAirborne = (int)clock(); return; }
	if (Commands->Get_Health(Owner()) <= 0.0f) { return; }
	Commands->Set_Animation(Owner(),Get_Parameter("Anim"),false,0,0.0f,-1.0f,false);
}

void vRadioKey::Created(GameObject *obj) {
	i = 0;
	if (is_keyhook_set != 1337) {
		hookid = AddNewKeyHook(Get_Parameter("Key"),obj);
		is_keyhook_set = 1337;
	}
}

void vRadioKey::KeyHookFunc() {
	int ID = Get_Player_ID(Owner());

	if (Config->CustomRadio) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (!p) { return; }
		if (p->muted) { return; }
		if (p->serial == false && Config->BlockNoSerialActions) { return; }

		int GameTime = vManager.DurationAsInt();
		if (p->RSpamExpire > GameTime) { return; }
		if (p->FirstRSpam <= GameTime - 5) { p->RadioMsgs = 0; }
		if (p->RadioMsgs == 0) { p->FirstRSpam = GameTime; }
		p->RadioMsgs++;
		if (p->RadioMsgs >= 5) {
			p->RadioMsgs = 0;
			p->RSpamExpire = GameTime + 5;
		}

		int type = Get_Int_Parameter("Type");
		if (type == 1) { // F4
			New_Chat_Message(Get_Player_ID(Owner()),-1,"Why am I the only one attacking!? Get over here!",false,1);
			ConsoleIn("sndt %d m00gnod_secx0003i3nemg_snd.wav",Commands->Get_Player_Type(Owner()));
		} else if (type == 2) { // F5
			New_Chat_Message(Get_Player_ID(Owner()),-1,"We need backup!",false,1);
			ConsoleIn("sndt %d m00ggdi_secx0005a2geen_snd.wav",Commands->Get_Player_Type(Owner()));
		} else if (type == 3) { // F6
			New_Chat_Message(Get_Player_ID(Owner()),-1,"Incoming! Everyone in position!",false,1);
			ConsoleIn("sndt %d m00ggdi_hesx0018i3gomg_snd.wav",Commands->Get_Player_Type(Owner()));
		} else if (type == 4) { // F7
			New_Chat_Message(Get_Player_ID(Owner()),-1,"Look out!!!",false,1);
			ConsoleIn("sndt %d m00itoc_002in_gers_snd.wav",Commands->Get_Player_Type(Owner()));
		}
	}
}

void vChatKey::Created(GameObject *obj) {
	if (is_keyhook_set != 1337) {
		hookid = AddNewKeyHook(Get_Parameter("Key"),obj);
		is_keyhook_set = 1337;
	}
}
void vChatKey::KeyHookFunc() {
	ChatMsgProcessing(this->Get_Parameter("Command"),this->Get_Int_Parameter("Type"),false,Get_Player_ID(Owner()),-1);
}

ScriptRegistrant<vRadioKey> vRadioKey_Registrant("vRadioKey","Key:string,Type:int");
ScriptRegistrant<vTauntKey> vTauntKey_Registrant("vTauntKey","Key:string,Anim:string");
ScriptRegistrant<vChatKey> vChatKey_Registrant("vChatKey","Key:string,Command:string,Type:int");
