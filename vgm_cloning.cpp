#include "vgm_cloning.h"

void Stewie_RefineryGameObj::Think() {
	Stewie_cGameData *GameObj = GameDataObj();
	if (GameObj && Config && GameObj->Is_Gameplay_Permitted() && !this->IsDestroyed && this->Defence.Get_Health() > 0.0f) {
		this->Manage_Money_Trickle_Sound();
		bool Docked = this->IsHarvesterDocked;
		float FrameSeconds = Stewie_TimeManager::FrameSeconds;
		if (!Docked && (Config->GameMode != Config->vSNIPER && Config->GameMode != Config->vMONEY)) {
			this->DistribTimer -= FrameSeconds;
			if (this->DistribTimer <= 0.0f) {
				this->DistribTimer = 1.0f;
				float DistribFunds = this->FundsPerSec;
				if (Config->IncomePerSec > 0) {
					DistribFunds = float(Config->IncomePerSec);
					if (!this->Base->IsPowered) { DistribFunds /= 2; }
				}
				Give_Money_To_All_Players(DistribFunds,this->Base->Team);
			}
		}
		if (this->Harvester) {
			if (Docked) {
				this->UnloadTimer -= (FrameSeconds / this->Base->OperationTimeFactor);
				Stewie_VehicleGameObj *HarvesterAsVeh = this->Get_Harvester_Vehicle();
				int team = ((Stewie_VehicleGameObjDef *)HarvesterAsVeh->Definition)->DefaultPlayerType;
				if (this->UnloadTimer <= 0.0f) {
					this->Harvester->Go_Harvest();
					this->IsHarvesterDocked = false;
					this->Set_Object_Dirty_Bit(vDIRTYBIT::vDBRARE,true);
					this->Play_Unloading_Animation(false);
					if (this->TotalFunds > 0.0f) { Give_Money_To_All_Players(this->TotalFunds,team); }
					this->TotalFunds = float(team == 1 ? Config->GDIHarvesterDump : Config->NodHarvesterDump);
				} else {
					Stewie_BaseControllerClass *base = Stewie_BaseControllerClass::Find_Base(team);
					float TotalCash = float(team == 1 ? Config->GDIHarvesterDump : Config->NodHarvesterDump);
					float Creds = LesserOfTwo(this->TotalFunds,TotalCash / (10.0f * Stewie_cNetwork::Fps));
					if (Creds > 0.0f) {
						Give_Money_To_All_Players(Creds,team);
						this->TotalFunds -= Creds;
					}
				}
			}
			this->Harvester->Think();
		} else {
			this->Set_Object_Dirty_Bit(vDIRTYBIT::vDBRARE,true);
			Stewie_RefineryGameObjDef *Definition = (Stewie_RefineryGameObjDef *)this->Definition;
			this->Base->Request_Harvester(Definition->HarvesterDefID);
		}
	}
	this->Stewie_ScriptableGameObj::Think();
}
void __stdcall RefineryGameObj__Think() {
	Stewie_RefineryGameObj *o = NULL;
	__asm { mov o,ecx }
	if (!o) { return; }
	o->Stewie_RefineryGameObj::Think();
}
ASMHackJump RefineryThinkCloneHack(RefineryThinkCloneAddr, &RefineryGameObj__Think, 1);
