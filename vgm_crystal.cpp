#include "vgm_crystal.h"

vCrystalManager *vCManager;

void vCrystalHandler::Created(GameObject *obj) {
	if (!Config->Crystal) { Commands->Destroy_Object(obj); }
	Remove_Duplicate_Script(obj,"vCrystalHandler");
	if (Config->GameMode == Config->vSNIPER) {
		Commands->Destroy_Object(obj);
	} else if (vCManager->ObjID != Commands->Get_ID(obj)) {
		if (vCManager->GetStage() <= -1) {
			vCManager->SetObj(obj);
			vCManager->Grow();
			vCManager->Value = vCManager->FullValue;
			vCManager->DistribValue = vCManager->FullDistribValue;
			Vector3 pos = Commands->Get_Position(obj);
			Commands->Start_Timer(obj,this,(float)Random(vCManager->GrowTimeMin,vCManager->GrowTimeMax),700001);
			vLogger->Log(vLoggerType::vVGM,"_CRYSTAL","Created: %.2f %.2f %.2f",pos.X,pos.Y,pos.Z);
			PowerupGameObjDef *def = (PowerupGameObjDef *)((PowerupGameObj *)obj)->definition;
			def->Persistant = true;
		}
	}
}
void vCrystalHandler::Timer_Expired(GameObject *obj, int number) {
	if (number == 700001) {
		Commands->Start_Timer(obj,this,(float)Random(vCManager->GrowTimeMin,vCManager->GrowTimeMax),700001);
		vCManager->Grow();
	}
}
void Crystal_Creator::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,(float)Random(vCManager->GrowTimeMin,vCManager->GrowTimeMax),900001);
}
void Crystal_Creator::Timer_Expired(GameObject *obj, int number) {
	if (number == 900001) {
		if (GameDataObj()->Is_Gameplay_Permitted()) {
			int rand = Random(0,vCManager->SpawnPositions.size(),true);

			if (vCManager->Bone) { vCManager->Bone->Set_Delete_Pending(); }
			vCManager->Bone = Stewie_ObjectLibraryManager::Create_Object("Invisible_Object")->As_PhysicalGameObj();
			vCManager->Bone->Set_Position(vCManager->SpawnPositions[rand]);

			Stewie_PhysicalGameObj *x = Stewie_ObjectLibraryManager::Create_Object("POW_Neuro_Link")->As_PhysicalGameObj();
			x->Attach_To_Object_Bone(vCManager->Bone,"neck");

			Script_Attach(x,"vCrystalHandler","",true);
		} else {
			Commands->Start_Timer(obj,this,5.0f,900001);
		}
	}
}

void Crystal_Damager::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,vCManager->HealthLossTime,805001);
}
void Crystal_Damager::Timer_Expired(GameObject *obj, int number) {
	if (number == 805001) {
		if (Get_Player_ID(obj) != vCManager->Carrier) { Remove_Script(obj,"Crystal_Damager"); return; }
		Commands->Start_Timer(obj,this,vCManager->HealthLossTime,805001);
		Do_Damage((Stewie_BaseGameObj *)obj,1.0f,"Tiberium");
	}
}

void vCrystalManager::Init() {
	if (!Config->Crystal) { return; }
	Stewie_RefineryGameObj* GRef = Team_Refinery(1);
	Stewie_RefineryGameObj* NRef = Team_Refinery(0);
	if (GRef && NRef && ((Stewie_RefineryGameObjDef *)GRef->Definition)->HarvesterDefID && ((Stewie_RefineryGameObjDef *)NRef->Definition)->HarvesterDefID) {
		GDIDisk = Stewie_ObjectLibraryManager::Create_Object("POW_Data_Disc")->As_PhysicalGameObj();
		GDIDisk->Set_Transform(GRef->DockTransform);
		GDIDisk->Set_Player_Type(1);
		GDIDisk->Set_Object_Dirty_Bit(vDBCREATION,false);

		NodDisk = Stewie_ObjectLibraryManager::Create_Object("POW_Data_Disc")->As_PhysicalGameObj();
		NodDisk->Set_Transform(NRef->DockTransform);
		NodDisk->Set_Player_Type(0);
		NodDisk->Set_Object_Dirty_Bit(vDBCREATION,false);
	} else {
		Config->Crystal = false;
		return;
	}
	
	ObjID = 0;
	Bone = NULL;
	Controller = Create_Object("Invisible_Object")->As_ScriptableGameObj();
	Load();
	Spawn();
}
void vCrystalManager::Think(bool FullSecond) {
	if (FullSecond) {
		if (WillExpire <= vManager.DurationAsUint() && WillExpire > 0) {
			WillExpire = 0;
			Expire();
		}
	}
}
void vCrystalManager::Spawn() {
	if (!Config->Crystal) { return; }
	if (Controller && Controller->NetworkID) { Controller->Set_Delete_Pending(); }
	WillExpire = 0;
	Controller = Create_Object("Invisible_Object")->As_ScriptableGameObj();
	Script_Attach(Controller,"Crystal_Creator","",true);
}
void vCrystalManager::Load() {
	CarrierWeapons.clear();
	ModelList.clear();
	SpawnPositions.clear();
	if (Config->Crystal) {
		const char *ConfigFile = "vgm_tibcrystal.ini";

		FullValue = getProfileFloat("General","FullValue",1500.0f,ConfigFile);
		FullDistribValue = getProfileFloat("General","FullDistributeValue",250.0f,ConfigFile);
		DecrementWhenDropped = getProfileInt("General","DecrementPerDrop",25,ConfigFile);
		HealthLossTime = getProfileFloat("General","LoseHealthEvery",1.0f,ConfigFile);
		GrowTimeMin = getProfileInt("General","GrowTimeMin",60,ConfigFile);
		GrowTimeMax = getProfileInt("General","GrowTimeMax",120,ConfigFile);
		ExpiryTime = getProfileInt("General","ExpireAfterDropping",30,ConfigFile);
		SBHCanCarry = (getProfileInt("General","SBHCanCarry",0,ConfigFile) == 1 ? true : false);
		CanEnterVehicle = (getProfileInt("General","CanEnterVehicle",0,ConfigFile) == 1 ? true : false);
		CanRefill = (getProfileInt("General","CanRefill",0,ConfigFile) == 1 ? true : false);
		MaxStage = getProfileInt("Growth","Steps",0,ConfigFile) - 1;

		int NumSteps = MaxStage + 1;
		if (NumSteps <= 0) {
			Config->Crystal = false;
			return;
		}
		for (int i = 1; i <= NumSteps; i++) {
			char Model[512];
			char Entry[512];
			sprintf(Entry,"Model%d",i);
			getProfileString("Growth",Entry,"dsp_tibgrande",Model,512,ConfigFile);
			ModelList.push_back(Model);
		}
		char *CurrMap = GameDataObj()->MapName;
		int SpawnTot = getProfileInt(CurrMap,"LocNum",0,ConfigFile);
		for (int i = 1; i <= SpawnTot; i++) {
			char RandLoc[512];
			char SpawnLoc[512];
			sprintf(RandLoc,"Loc%d",i);
			getProfileString(CurrMap,RandLoc,"0 0 0",SpawnLoc,512,ConfigFile);
			const char *NewSpawn = (char *)SpawnLoc;
			char* mess = new char[strlen(NewSpawn) + 1];
			strcpy(mess,NewSpawn);
			int tokens = numtok(mess,32);
			if (tokens != 3) { NewSpawn = "0 0 0"; }
			float SpawnX = (float)atof(strtok(mess," "));
			float SpawnY = (float)atof(strtok(NULL," "));
			float SpawnZ = (float)atof(strtok(NULL," "));
			Stewie_Vector3 NewLocation(SpawnX,SpawnY,SpawnZ);
			SpawnPositions.push_back(NewLocation);
			delete[] mess;
		}
	}
	Reset();
}
void vCrystalManager::Evaluate() {
	if (!Config->Crystal) { return; }
	GameObject *Crystal = Commands->Find_Object(ObjID);
	if (!Crystal) {
		Stage = -1;
	}
	if (Stage >= 0 && Stage < (int)ModelList.size()) {
		Commands->Set_Model(Crystal,ModelList[Stage].c_str());
	} else if (Stage < 0) {
		Spawn();
	}
}
void vCrystalManager::Expire() {
	if (!Config->Crystal) { return; }
	Stewie_PhysicalGameObj *DiskObj = GetDisk(1);
	if (DiskObj) {
		DiskObj->DeletePending = true;
		Update_Dirty_Bits(DiskObj);
		DiskObj->DeletePending = false;
	}
	DiskObj = GetDisk(0);
	if (DiskObj) {
		DiskObj->DeletePending = true;
		Update_Dirty_Bits(DiskObj);
		DiskObj->DeletePending = false;
	}
	GameObject *Crystal = Commands->Find_Object(ObjID);
	if (Crystal) { Commands->Destroy_Object(Crystal); }
	Reset();
	Carrier = -1;
	CarrierWeapons.clear();
}
void vCrystalManager::Drop() {
	if (!Config->Crystal) { return; }
	Value -= Value * (float(DecrementWhenDropped) / 100.0f);
	DistribValue -= DistribValue * (float(DecrementWhenDropped) / 100.0f);

	WillExpire = vManager.DurationAsInt() + (unsigned int)ExpiryTime;

	GameObject *obj = Player_GameObj(Carrier);
	if (Carrier > 0 && obj && Stewie_cNetwork::PServerConnection && Stewie_cNetwork::PServerConnection->RemoteHosts[Carrier]) {
		Stewie_PhysicalGameObj *DiskObj = GetDisk(Commands->Get_Player_Type(obj));
		DiskObj->DeletePending = true;
		Stewie_cNetwork::Send_Object_Update(DiskObj,Carrier);
		DiskObj->DeletePending = false;

		if (Commands->Get_Health(obj) > 0.0f) {
			Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)obj);
			for (unsigned int i = 0; i < CarrierWeapons.size(); i++) {
				int bullets = CarrierWeapons[i]->DSClipSize.Get() + CarrierWeapons[i]->DSMaxClipBullets.Get();
				if (CarrierWeapons[i]->DSMaxClipBullets.Get() == -1) { bullets = -1; }
				bag->Add_Weapon(CarrierWeapons[i],bullets,true);
			}
			if (bag->Vector.Count() >= 1) { bag->Select_Index(1); }
		}

		Remove_Script(obj,"Crystal_Damager");
		PrivMsgColoredVA(Carrier,2,0,255,64,"[VGM] You dropped the Tiberium Crystal and its value has decreased by %d%%.",DecrementWhenDropped);
	}

	Stewie_PhysicalGameObj *Crystal = Stewie_GameObjManager::Find_PhysicalGameObj(ObjID);
	if (obj) {
		if (Crystal) {
			if (Bone) { Bone->Set_Delete_Pending(); }
			Bone = Stewie_ObjectLibraryManager::Create_Object("Invisible_Object")->As_PhysicalGameObj();
			Crystal->Attach_To_Object_Bone(Bone,"neck");
		}
	} else {
		Expire();
		return;
	}

	Carrier = -1;
	CarrierWeapons.clear();
}
bool vCrystalManager::Collected(int ID) {
	if (!Config->Crystal) { return false; }
	if (GetStage() < MaxStage) { return false; }
	if (Carrier == ID || Stewie_cPlayerManager::Is_Player_Present(Carrier)) { return false; }
	GameObject *Crystal = Commands->Find_Object(ObjID);
	if (!Crystal) { return false; }

	Carrier = ID;
	GameObject *obj = Player_GameObj(Carrier);
	
	if (SBHCanCarry == false && !stricmp(Get_Preset_Name(obj),"CnC_Nod_FlameThrower_2SF")) { return false; }

	CarrierWeapons.clear();
	Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)obj);
	if (bag) {
		int x = bag->Vector.Count();
		for (int i = 0; i < x; i++) {
			if (bag->Vector[i]) {
				Stewie_WeaponClass *Weapon = bag->Vector[i];
				CarrierWeapons.push_back(Weapon->Definition);
			}
		}
		bag->Clear_Weapons();
	} else {
		return false;
	}

	Stewie_PhysicalGameObj *DiskObj = GetDisk(Commands->Get_Player_Type(obj));
	if (DiskObj) {
		DiskObj->Set_Object_Dirty_Bit(Carrier,vDBCREATION,true);
		Stewie_cNetwork::Send_Object_Update(DiskObj,Carrier);
	}
	
	std::string pname = Player_Name_From_ID(Carrier);

	vLogger->Log(vLoggerType::vVGM,"_CRYSTAL","%s collected the Tiberium Crystal.",Player_Name_From_ID(ID).c_str());
	PrivMsgColoredVA(Carrier,2,0,255,64,"[VGM] You have picked up the Tiberium Crystal. Return it to your Refinery for a cash bonus!");
	Remove_Script(obj,"Crystal_Damager");
	Attach_Script_Once(obj,"Crystal_Damager","");

	if (Crystal) { Commands->Attach_To_Object_Bone(Crystal,obj,"bone for bag"); }
	WillExpire = 0;

	return true;
}
bool vCrystalManager::CollectedRefineryDisk(int ID, GameObject *disk) {
	if (!Config->Crystal) { return false; }
	if (ID != Carrier || Carrier <= 0) { return false; }
	GameObject *obj = Player_GameObj(ID);
	if (Commands->Get_Player_Type(obj) != Commands->Get_Player_Type(disk)) { return false; }

	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		if (Node->NodeData->PlayerType.Get() == Commands->Get_Player_Type(obj)) {
			if (Node->NodeData->PlayerId == ID) { Node->NodeData->Increment_Money(Value); }
			else { Node->NodeData->Increment_Money(DistribValue); }
		}
	}

	Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)obj);
	for (unsigned int i = 0; i < CarrierWeapons.size(); i++) {
		int bullets = CarrierWeapons[i]->DSClipSize.Get() + CarrierWeapons[i]->DSMaxClipBullets.Get();
		if (CarrierWeapons[i]->DSMaxClipBullets.Get() == -1) { bullets = -1; }
		bag->Add_Weapon(CarrierWeapons[i],bullets,true);
	}
	if (bag->Vector.Count() >= 1) { bag->Select_Index(1); }
	CarrierWeapons.clear();

	vLogger->Log(vLoggerType::vVGM,"_CRYSTAL","%s returned the Tiberium Crystal to the %s Refinery.",Player_Name_From_ID(ID).c_str(),Get_Translated_Team_Name(Commands->Get_Player_Type(obj)).c_str());
	PrivMsgColoredVA(Carrier,2,0,255,64,"[VGM] You have returned the Tiberium Crystal to your Refinery! Your teammates have received %.0f credits each and you have received %.0f credits.",DistribValue,Value);
	PrivMsgColoredVA(-1 * Carrier,Commands->Get_Player_Type(obj),0,255,64,"[VGM] %s has returned the Tiberium Crystal to your refinery! You have received %.0f credits.",Player_Name_From_ID(ID).c_str(),DistribValue);
	Remove_Script(obj,"Crystal_Damager");

	Carrier = -1;

	Stewie_PhysicalGameObj *DiskObj = GetDisk(Commands->Get_Player_Type(obj));
	DiskObj->DeletePending = true;
	Update_Dirty_Bits(DiskObj);
	DiskObj->DeletePending = false;

	GameObject *Crystal = Commands->Find_Object(ObjID);
	if (Crystal) { Commands->Destroy_Object(Crystal); }
	Reset();
	return false;
}

ScriptRegistrant<vCrystalHandler> vCrateHandler_Registrant("vCrystalHandler","");
ScriptRegistrant<Crystal_Creator> Crate_Creator_Registrant("Crystal_Creator","");
ScriptRegistrant<Crystal_Damager> Crate_Damager_Registrant("Crystal_Damager","");
