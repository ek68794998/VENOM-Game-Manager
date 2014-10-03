#include "vgm_hooks.h"

// ---------------------------
//  This File (Hooks.cpp) generally need not be modified, with the exception of adding a new hook.
// ---------------------------

bool InitHook() {
	vManager.Init();
	return true;
}
ASMHackJump InitHack(InitHookAddr, &InitHook);
void __stdcall XWISInitHook() {
	__asm {
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
	}
	static ASMHackJump* XWISJoinHook1 = new ASMHackJump(XWISPlayerLimitBypassHookAddr1, &XWISPlayerLimitBypassNaked1, 1);
	static ASMHackCall* XWISJoinHook2 = new ASMHackCall(XWISPlayerLimitBypassHookAddr2, &XWISPlayerLimitBypassNaked2);
	ConsoleOut("[VGM] WOLAPI Hooks Enabled.");
}
ASMHackCall XWISInitHack(XWISInitHookAddr, &XWISInitHook);
void __stdcall OnThink() {
	vManager.Think();
	Stewie_GameModeManager::Think();
}
ASMHackCall ThinkHack(ThinkHookAddr, &OnThink);
void __stdcall OnShutdown() { vManager.Shutdown(); ConsoleOut("[VGM] The Dedicated Server shutdown procedure is complete."); }
ASMHackJump ShutdownHack(ShutdownHookAddr, &OnShutdown);
void __stdcall OnLevelLoaded() { vManager.Level_Loaded(); }
ASMHackJump LevelLoadHack(LevelLoadHookAddr, &OnLevelLoaded);
void __stdcall OnLevelEnd() { vManager.Level_Ended(); }
__declspec(naked) void LevelEndNaked() {
	__asm {
		sub esp,0x10
		push ebx
		push ebp
		push ecx
		call OnLevelEnd
		pop ecx
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump LevelEndHack(LevelEndHookAddr, &LevelEndNaked);
ASMHackJump TeamScoreSwitchFixHack(TeamScoreSwitchFixHookAddr, TeamScoreSwitchFixHookAddr + 0x109);
void __stdcall OnGameReset() { vManager.Game_Reset(); }
__declspec(naked) void ResetGameNaked() {
	__asm {
		call OnGameReset
		jmp GameDataObj // Original function
	}
}
ASMHackNops ChangeTeamNops(ChangeTeamHookAddr, 0x6B); // Old remix code
ASMHackCall ResetGameHack(ResetGameHookAddr, &ResetGameNaked); // Just before resetting the player data (scores)

ASMHackCall ChatMsgHack(ChatMsgHookAddr, &ChatMsgHook);
__declspec(naked) void RadioNaked() {
	__asm {
		push ecx
		push ecx
		call RadioHook
		pop ecx

		test al,al
		je skip

		push edi
		mov edi,ecx
		push 0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump RadioHack(RadioHookAddr, &RadioNaked, 3);

__declspec(naked) void ReserveAllSlotsNaked() {
	__asm {
		mov eax,127
		sub esp,0x10
		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump ReserveAllSlotsHack(ReserveAllSlotsHookAddr, &ReserveAllSlotsNaked, 2);
__declspec(naked) void XWISPlayerLimitBypassNaked1() {
	__asm {
		push 0x80
		push [esi+4]
		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
unsigned int __stdcall XWISPlayerLimitBypassHook2() {
	return GameDataObj()->MaxPlayers;
}
__declspec(naked) void XWISPlayerLimitBypassNaked2() {
	__asm {
		call XWISPlayerLimitBypassHook2
		add eax,1
		retn
	}
}

__declspec(naked) void ClientConnectionNaked() {
	__asm {
		push [esp+4]
		push ecx
		call ClientConnectionHook
		retn 4
	}
}
ASMHackJump ClientConnectionHack(ClientConnectionHookAddr, &ClientConnectionNaked);
__declspec(naked) void PlayerJoinNaked() {
	__asm {
		push ecx
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		call PlayerJoinHook
		retn
	}
}
ASMHackPtr PlayerJoinHack(PlayerJoinHookAddr, &PlayerJoinNaked);
__declspec(naked) void ClientDisconnectNaked() {
	__asm {
		push esi
		call ClientDisconnectHook
		retn 12
	}
}
ASMHackCall ClientDisconnectHack(ClientDisconnectHookAddr, &ClientDisconnectNaked);
__declspec(naked) void PlayerLeaveNaked() {
	__asm {
		push edi
		call PlayerLeaveHook
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackCall PlayerLeaveHack(PlayerLeaveHookAddr, &PlayerLeaveNaked);

__declspec(naked) void PlayerTeamNaked() {
	__asm {
		push ecx
		push [esp+4]
		push ecx
		call PlayerTeamHook
		pop ecx

		mov edi,ecx
		mov [esp+0xC],edi

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump PlayerTeamHack(PlayerTeamHookAddr, &PlayerTeamNaked, 1);

__declspec(naked) void SerialRetrievedNaked() {
	__asm {
		push esi
		call SerialRetrievedHook
		pop edi
		pop esi
		pop ebx
		retn 4
	}
}
ASMHackJump SerialRetrievedHack(SerialRetrievedHookAddr, &SerialRetrievedNaked);

__declspec(naked) void VehicleTransitionNaked() {
	__asm {
		push ecx
		push ecx
		push [esp+0xC]
		call VehicleTransitionHook
		pop ecx
		test al,al
		je go

		push [esp+4]
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax

		go:
			retn
	}
}
ASMHackCall VehicleTransitionHack(VehicleTransitionHookAddr, &VehicleTransitionNaked);
__declspec(naked) void ExitDeadVehicleNaked() {
	__asm {
		push ecx
		push ecx
		call ExitDeadVehicleHook
		pop ecx

		sub esp,0x7C
		push esi
		mov esi,ecx

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump ExitDeadVehicleHack(ExitDeadVehicleHookAddr, &ExitDeadVehicleNaked, 1);

ASMHackCall OnPurchaseHack(OnPurchaseHookAddr, &OnPurchaseHook);
ASMHackCall OnPurchaseCharacterHack(OnPurchaseCharacterHookAddr, &OnPurchaseCharacterHook);

__declspec(naked) void PowerupCollectedNaked() {
	__asm {
		push ecx
		push ecx
		mov eax,[esp+0xC]
		push eax
		call PowerupCollectedHook
		pop ecx

		test al,al
		je skip

		push ebx
		mov ebx,[esp+8]
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump PowerupCollectedHack(PowerupCollectedHookAddr, &PowerupCollectedNaked);
__declspec(naked) void WantsPowerupsNaked() {
	__asm {
		push ecx
		call WantsPowerupsHook
		retn
	}
}
ASMHackJump WantsPowerupsHack(WantsPowerupsHookAddr, &WantsPowerupsNaked);

__declspec(naked) void WeaponDefSearchNaked() {
	__asm {
		test esi,esi
		je skip

		mov eax,[esi]
		test eax,eax
		je skip

		mov ecx,esi
		call [eax+0x20]
		mov ecx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp ecx

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump WeaponDefSearchHack(WeaponDefSearchHookAddr, &WeaponDefSearchNaked);

__declspec(naked) void PlayerKillMsgNaked() {
	__asm {
		pop eax
		pop eax
		push [esp+0x14]
		push [esp+0x10]
		push ecx
		call PlayerKillMsgHook
		mov ecx,PlayerKillMsgHookAddr + 5
		jmp ecx
	}
}
ASMHackJump PlayerKillMsgHack(PlayerKillMsgHookAddr, &PlayerKillMsgNaked);

__declspec(naked) bool DamageNaked() {
	__asm {
		push ecx
		push ecx
		call DamageHook
		pop ecx
		retn
	}
}
ASMHackCall DamageHack(DamageHookAddr, &DamageNaked);
void __stdcall SoldierRDNaked(Stewie_OffenseObjectClass &object, float unk1, int unk2) {
	bool damage = false;
	__asm {
		push ecx
		push object
		push ecx
		call SoldierRDHook
		mov damage,al
		pop ecx
	}
	if (damage) {
		__asm {
			push unk2
			push unk1
			push object
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			call eax
		}
	}
}
ASMHackCall SoldierRDHack(SoldierRDHookAddr, &SoldierRDNaked);
void __stdcall VehicleRDNaked(Stewie_OffenseObjectClass &object, float unk1, int unk2) {
	bool damage = false;
	__asm {
		push ecx
		push object
		push ecx
		call VehicleRDHook
		mov damage,al
		pop ecx
	}
	if (damage) {
		__asm {
			push unk2
			push unk1
			push object
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			call eax
		}
	}
}
ASMHackCall VehicleRDHack(VehicleRDHookAddr, &VehicleRDNaked);
__declspec(naked) void BuildingRDNaked() {
	__asm {
		push ebx
		mov ebx,ecx
		push esi
		push edi

		push ecx
		push [esp+0x14]
		push ecx
		call BuildingRDHook
		pop ecx

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump BuildingRDHack(BuildingRDHookAddr, &BuildingRDNaked);
__declspec(naked) void FatalDamageNaked() {
	__asm {
		push [esp]
		push ebx
		call FatalDamageHook

		cmp esi,ebp
		je skip

		fld [esi]

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump FatalDamageHack(FatalDamageHookAddr, &FatalDamageNaked, 1);
__declspec(naked) void JumpNaked() {
	__asm {
		mov eax,[ecx]
		call [eax+0x18]

		push eax
		mov eax,esi
		mov eax,[eax+0x2C]
		mov eax,[eax+0x4C]
		add eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		push eax
		call JumpHook
		pop eax

		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump JumpHack(JumpHookAddr, &JumpNaked);
__declspec(naked) void FallNaked() {
	__asm {
		push [esp+0x28]
		mov eax,edi
		mov eax,[eax+0x2C]
		mov eax,[eax+0x4C]
		add eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		push eax
		call FallHook

		test al,al
		je nodmg

		mov eax,FallHookAddr + 6
		jmp eax

		nodmg:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump FallHack(FallHookAddr, &FallNaked);
__declspec(naked) void __stdcall VehicleFlipNaked() {
	__asm {
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax
		push eax
		push ebx
		call VehicleFlipHook
		pop eax

		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump VehicleFlipHack(VehicleFlipHookAddr, &VehicleFlipNaked);

__declspec(naked) void __stdcall ObjectCreationNaked() {
	__asm {
		push esi
		mov esi,ecx
		push edi
		xor edi,edi

		push ecx
		push esi
		call ObjectCreationHook
		pop ecx

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump ObjectCreationHack(ObjectCreationHookAddr, &ObjectCreationNaked, 1);
__declspec(naked) void __stdcall VehicleCreateNaked() {
	__asm {
		push ecx
		
		push [esp+0x8]
		push edi
		call VehicleCreateHook
		
		pop ecx
		
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackCall VehicleCreateHack1(VehicleCreateHookAddr1, &VehicleCreateNaked);
ASMHackCall VehicleCreateHack2(VehicleCreateHookAddr2, &VehicleCreateNaked);

__declspec(naked) void HarvesterDockedNaked() {
	__asm {
		push esi
		mov esi,ecx

		push [esi+0x82C]
		push esi
		call HarvesterDockedHook

		mov ecx,esi

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax

		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump HarvesterDockedHack(HarvesterDockedHookAddr, &HarvesterDockedNaked, 3);

__declspec(naked) void C4InitNaked() {
	__asm {
		push ecx
		push ebx
		call C4InitHook
		pop ecx

		pop esi
		pop ebx
		add esp,0x20

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump C4InitHack(C4InitHookAddr, &C4InitNaked);
ASMHackCall C4LimitHack(C4LimitHookAddr, &C4LimitHook);
ASMHackCall C4ShouldExplodeHack(C4ShouldExplodeHookAddr, &C4ShouldExplodeHook);
__declspec(naked) void C4DefuseNaked() {
	__asm {
		push ecx
		push [esp+0x8] // ECX + Return Address = 0x8
		push ecx
		call C4DefuseHook
		pop ecx
		test al,al
		je skip

		mov al,0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump C4DefuseHack(C4DefuseHookAddr, &C4DefuseNaked);
/*__declspec(naked) void C4DetonationNaked() {
	__asm {
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax

		push eax
		push esi
		call C4DetonationHook
		pop eax

		mov ebx,C4DetonationHookAddr + 5
		jmp ebx
	}
}
ASMHackJump C4DetonationHack(C4DetonationHookAddr, &C4DetonationNaked);*/
__declspec(naked) void C4DetonationNaked() {
	__asm {
		push ecx
		push ecx
		call C4DetonationHook
		pop ecx
		test al,al
		je skip

		mov al,0x000000 // For security and licensing purposes, an address has been hidden from this line
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump C4DetonationHack(C4DetonationHookAddr, &C4DetonationNaked);
__declspec(naked) void C4PlantedNaked() {
	__asm {
		push ecx
		lea edi,[esi-0x76C]
		push edi
		call C4PlantedHook
		pop ecx
		
		mov al,0x000000 // For security and licensing purposes, an address has been hidden from this line

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump C4PlantedHack(C4PlantedHookAddr, &C4PlantedNaked);
__declspec(naked) void C4ExportAttachedNaked() {
	__asm {
		push 0x20

		test ecx,ecx
		jne skip

		mov ecx,0x000000 // For security and licensing purposes, an address has been hidden from this line

		skip:
		push ecx
		mov ecx,esi
		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax

		mov ecx,C4ExportAttachedHookAddr + 0xA
		jmp ecx
	}
}
ASMHackJump C4ExportAttachedHack(C4ExportAttachedHookAddr, &C4ExportAttachedNaked, 5);

ASMHackCall EnemySeenHack(EnemySeenHookAddr, &EnemySeenHook);

__declspec(naked) void BeaconStateNaked() {
	__asm {
		push ebp
		mov ebp,[esp+0x14]

		push ecx
		push ebp
		push ecx
		call BeaconStateHook
		pop ecx

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump BeaconStateHack(BeaconStateHookAddr, &BeaconStateNaked);
__declspec(naked) void BeaconWarningNaked() {
	__asm {
		push ebx
		push esi
		mov esi,[0x000000] // For security and licensing purposes, an address has been hidden from this line

		push ecx
		push 5
		push [esp+0x14]
		call BeaconStateHook
		pop ecx

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax
	}
}
ASMHackJump BeaconWarningHack(BeaconWarningHookAddr, &BeaconWarningNaked, 3);
__declspec(naked) void BeaconInitiatedNaked() {
	__asm {
		push 6
		push esi
		call BeaconStateHook
		
		mov al,[0x000000] // For security and licensing purposes, an address has been hidden from this line

		mov ecx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp ecx
	}
}
ASMHackJump BeaconInitiatedHack(BeaconInitiatedHookAddr, &BeaconInitiatedNaked);
__declspec(naked) void BeaconDisarmedNaked() {
	__asm {
		push ecx
		push [esp+0x8]
		push ecx
		call BeaconDisarmedHook
		pop ecx
		
		push esi
		mov esi,ecx
		mov al,[esi+0x4A4]

		mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp edx
	}
}
ASMHackJump BeaconDisarmedHack(BeaconDisarmedHookAddr, &BeaconDisarmedNaked, 4);
__declspec(naked) void BeaconRequestedNaked() {
	__asm {
		push [esp+4]
		push ecx
		call BeaconRequestedHook
		retn 4
	}
}
ASMHackJump BeaconRequestedHack(BeaconRequestedHookAddr, &BeaconRequestedNaked);
__declspec(naked) bool BeaconOwnerInterruptedNaked() {
	__asm {
		push ecx
		push ecx
		call BeaconOwnerInterruptedHook
		pop ecx
		retn
	}
}
ASMHackCall BeaconOwnerInterruptedHack(BeaconOwnerInterruptedHookAddr, &BeaconOwnerInterruptedNaked);
__declspec(naked) void PedestalTimerNaked() {
	__asm {
		push ecx
		push esi
		call PedestalTimerHook
		pop ecx

		test al,al
		je NotDetonated

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		NotDetonated:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump PedestalTimerHack(PedestalTimerHookAddr, &PedestalTimerNaked);
__declspec(naked) void PedestalNaked() {
	__asm {
		push ecx
		push esi
		call PedestalHook
		pop ecx
		test al,al
		je skip

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		call eax

		mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
		jmp eax

		skip:
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp eax
	}
}
ASMHackJump PedestalHack(PedestalHookAddr, &PedestalNaked);

__declspec(naked) void SelectWeaponNaked() {
	__asm {
		push esi
		call SelectWeaponHook
		
		pop esi
		retn
	}
}
ASMHackJump SelectWeaponHack(SelectWeaponHookAddr, &SelectWeaponNaked);

int __cdecl UnlimitedAmmoHook() {
	Stewie_WeaponClass *w;
	__asm { mov w, ecx }
	int r = 0;
	if (w) {
		if ((int)(w->ClipBullets) == -1) {
			if ((int)(w->Bullets) == 0) { r = -1; }
			else { r = (int)(w->Bullets); }
		} else {
			r = (int)(w->Bullets) + (int)(w->ClipBullets);
		}
	}
	return r;
}
ASMHackCall UnlimitedAmmoHack(UnlimitedAmmoHookAddr, &UnlimitedAmmoHook);

/*
#ifdef DEBUG
#ifdef _DEV_
void __stdcall InterpretStateHook(Stewie_HumanStateClass& HumanState, Stewie_HumanStateClass::HumanStateType StateType, int SubState) {
	if (HumanState.Type != Stewie_HumanStateClass::IN_VEHICLE) {
		HumanState.Set_State(StateType,SubState);
	}
}
__declspec(naked) void InterpretStateNaked() {
	__asm {
		push [esp+8]
		push [esp+8]
		push ecx
		call InterpretStateHook
		retn 8
	}
}
ASMHackJump obb_separation_testHook(obbSeparationTestFixHookAddr, &obb_separation_test);
ASMHackJump CheckBox0BasisHack(CheckBox0BasisHookAddr, &obb_check_box0_basis);
ASMHackJump CheckBox1BasisHack(CheckBox1BasisHookAddr, &obb_check_box1_basis);
ASMHackCall InterpretStateHack(0x000000, &InterpretStateNaked); // For security and licensing purposes, an address has been hidden from this line
#else
__declspec(naked) void BlueHellHook() {
	__asm {
		mov esi,[ecx+0xAC]
		mov dword ptr [ecx+0xC],-1
		retn
	}
}
ASMHackCall BlueHellHack(BlueHellHookAddr, &BlueHellHook, 1);
#endif
#endif
*/

__declspec(naked) void MenuBgCrashFixNaked() {
	__asm {
		test ecx,ecx
		je skip

		mov eax,[ecx+0x10]
		test eax,eax
		jnz skip

		mov edx,MenuBgCrashFixHookAddr + 0x7
		jmp edx

		skip:
			mov edx,0x000000 // For security and licensing purposes, an address has been hidden from this line
			jmp edx
	}
}
ASMHackJump MenuBgCrashFixHack(MenuBgCrashFixHookAddr, &MenuBgCrashFixNaked);

__declspec(naked) void DoorFixNaked() {
	__asm {
		push [esp+0x10]
		push ecx
		call DoorFixHook
		retn
	}
}
ASMHackCall DoorFixHack(DoorFixHookAddr, &DoorFixNaked);

__declspec(naked) void PointFixHook() {
	__asm {
		push [esp+4+0x4C] // OffenseObject
		push ebx // DefenseObject
		call GetDmgMultiplier

		fmul [esp+4+0x14] // ShieldDamage
		fadd [esp+4+0x10] // HealthDamage

		retn
	}
}
ASMHackCall PointFixHack(PointFixHookAddr, &PointFixHook, 1);

void __stdcall PTFixHook(Stewie_cPlayer& Player) {
	Player.Set_Is_Active(true);
	Stewie_cNetwork::Send_Object_Update(&Player,Player.PlayerId);
}
__declspec(naked) void PTFixNaked() {
	__asm {
		mov [esp+4],ecx
		jmp PTFixHook
	}
}
ASMHackCall PTFixHack(PTFixHookAddr, &PTFixNaked);

ASMHackNops NodHarvesterGlitchFixNops(NodHarvesterGlitchFixHookAddr, 0xC);

ASMHackNops ResultScreenGlitchFixNops(ResultScreenGlitchFixHookAddr, 0x9);
