#include "vgm_crystal.h"
#include "vgm_game.h"

bool Offensive;
bool EMPon;
bool GGstarted;

void InitiateOffensive() {
	Offensive = true;
	if (Team_Base_Defense(0)) { Team_Base_Defense(0)->Enable_Power(false); }
	if (Team_Base_Defense(1)) { Team_Base_Defense(1)->Enable_Power(false); }
	HostMessage("[VGM] --------------------------------");
	HostMessage("[VGM] --- Offensive Mode initiated!");
	HostMessage("[VGM] --- Base defenses are now offline!");
	HostMessage("[VGM] --------------------------------");
	Stewie_BackgroundMgrClass::Set_Clouds(50.0f,55.0f,1.0f);
	Stewie_BackgroundMgrClass::Set_Lightning(10.0f,0.0f,1.0f,0.0f,1.0f,1.0f);
	Stewie_BackgroundMgrClass::Override_Sky_Tint(8.0f,0.0f);
	if (Config->Sounds) {
		Create_2D_WAV_Sound("amb_airraid.wav");
		Create_2D_WAV_Sound("10-stomp.mp3");
		//Create_2D_WAV_Sound("m00evag_dsgn0086i1evag_snd.wav");
	}
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		GameObject *o = Player_GameObj(Node->NodeData->PlayerId);
		TeamPurchaseSettingsDefClass *PT = TeamPurchaseSettingsDefClass::Get_Definition(Node->NodeData->PlayerType.Get());
		Powerup((Stewie_SoldierGameObj *)o,Get_Definition_Name(PT->beaconpresetid),true);
		if (Config->Sounds) {
			if (Node->NodeData->PlayerType.Get() == 0) {
				Create_2D_WAV_Sound_Player(o,"m00evag_dsgn0070i1evag_snd.wav");
			} else {
				Create_2D_WAV_Sound_Player(o,"m00evan_dsgn0074i1evan_snd.wav");
			}
		}
	}
}

void __stdcall ChatMsgHook(Stewie_WideStringClass *string, int vtype, bool popup, int sender, int receiver) {
	ChatMsgProcessing(WCharToStr(string->m_Buffer),vtype,popup,sender,receiver);
}
void ChatMsgProcessing(std::string string, int vtype, bool popup, int sender, int receiver) {
	bool process = true;
	char strraw[1024];
	sprintf(strraw,"%s",string.c_str());
	const char *str = (const char *)strraw;
	if (sender <= 0) { goto doproc; }
	if (receiver <= -2) {
		vTokenParser *BHSStr = new vTokenParser((char *)str);
		BHSStr->Parse("\n");
		if (BHSStr->Numtok() >= 3 && !stricmp(BHSStr->Gettok(1).c_str(),"j") && atoi(BHSStr->Gettok(2).c_str()) == sender) {
			vBhsVersionTag VerTag;
			VerTag.PID = sender;
			VerTag.Version = (float)atof(BHSStr->Gettok(3).c_str());
			vPManager->Add_BHS_Version(VerTag);
			ConsoleOut("[VGM] Detected BHS version of player %d: %.1f",sender,VerTag.Version);
		}
		BHSStr->Delete();
		goto doproc;
	}
	vPlayer *p = vPManager->Get_Player(sender);
	if (!p) { process = false; goto doproc; }
	if (p->muted) { process = false; goto doproc; }
	if (p->serial == false && Config->BlockNoSerialActions) { process = false; goto doproc; }

	int GameTime = vManager.DurationAsInt();
	if (GameTime > 1) {
		if (p->CSpamExpire > GameTime) { process = false; goto doproc; }
		if (p->FirstCSpam <= GameTime - 5) { p->ChatMsgs = 0; }
		if (p->ChatMsgs == 0) { p->FirstCSpam = GameTime; }
		p->ChatMsgs++;
		if (p->ChatMsgs >= 5) {
			p->ChatMsgs = 0;
			p->CSpamExpire = GameTime + 5;
		}
	}

	vTokenParser *Str = new vTokenParser((char *)str);
	Str->Parse(" ");
	if (Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY || Config->GameMode == Config->vINFONLY) {
		if (Execute_AOWCommand(sender,Str,vtype) == false) { process = false; }
		else if (Execute_OtherCommand(sender,Str,vtype) == false) { process = false; }
		else if (Hide_Command(sender,Str,vtype)) { process = false; }
	} else if (Config->GameMode == Config->vSNIPER) {
		if (Execute_SniperCommand(sender,Str,vtype) == false) { process = false; }
		else if (Execute_OtherCommand(sender,Str,vtype) == false) { process = false; }
		else if (Hide_Command(sender,Str,vtype)) { process = false; }
	}
	Str->Delete();

	doproc:
	if (process) {
		if (vtype == 2) {
			if (sender > 0 && receiver > 0) {
				New_Chat_Message(sender,receiver > 0 ? receiver : -1,str,false,vtype);
				if (Config->ShowPrivateChat) {
					vLogger->Log(vLoggerType::vVGM,"_PRIVATE","%s (to %s): %s",Player_Name_From_ID(sender).c_str(),Player_Name_From_ID(receiver).c_str(),str);
				}
			}
		} else {
			char str2[512];
			sprintf(str2,"%s",Execute_Autocomplete(str).c_str());
			if (stricmp(str2,"NULL")) { New_Chat_Message(sender,receiver > 0 ? receiver : -1,str2,false,1); }
			else { New_Chat_Message(sender,receiver > 0 ? receiver : -1,str,false,vtype); }
		}
	}
}
bool Execute_AOWCommand(int ID, vTokenParser *Str, int type) {
	int tokens = Str->Numtok();
	std::string cmd = Str->Gettok(1);
	GameObject *o = Player_GameObj(ID);
#ifdef _DEV_
	if (!stricmp(cmd.c_str(),"!freeze")) {
		Stewie_BaseGameObj *b = (Stewie_BaseGameObj *)o;
		b->Freeze = true;
		DebugMessage("%s, Frozen.",b->Definition->Get_Name());
		return false;
	} else if (!stricmp(cmd.c_str(),"!ff")) {
		bool &FriendlyFire = *(bool *)0x000000; // For security and licensing purposes, an address has been hidden from this line
		FriendlyFire = (FriendlyFire ? false : true);
		DebugMessage("FF toggled.");
		return false;
	} else if (!stricmp(cmd.c_str(),"!showpos")) {
		Vector3 pos = Commands->Get_Position(o);
		DebugMessage("Position: %.4f %.4f %.4f",pos.X,pos.Y,pos.Z);
		return false;
	} else if (!stricmp(cmd.c_str(),"!rv")) {
		if (Stewie_cPlayerManager::Tally_Team_Size(1) >= 1) { Create_Vehicle(Find_Random(VehicleCID,1)->Get_Name(),0.0f,Find_First_Player(1),1); }
		if (Stewie_cPlayerManager::Tally_Team_Size(0) >= 1) { Create_Vehicle(Find_Random(VehicleCID,0)->Get_Name(),0.0f,Find_First_Player(0),0); }
		return false;
	} else if (!stricmp(cmd.c_str(),"!d1")) {
		DebugMessage("%f",vManager.DurationAsFloat());
		return false;
	} else if (!stricmp(cmd.c_str(),"!d2")) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (p) {
			int WhoCares = 0;
			DebugMessage("PLAYERHITS1 %s %u",p->name.c_str(),p->Hits.size());
			for (int i = 0; i < 500; i++) { p->Add_Hit(Commands->Get_ID(o),Get_Held_Weapon((Stewie_SoldierGameObj *)o)->Definition->PrimaryAmmoDefId); }
			DebugMessage("PLAYERHITS2 %s %u",p->name.c_str(),p->Hits.size());
			DebugMessage("PLAYERGETHITS1 %s %d",p->name.c_str(),clock());
			for (int i = 0; i < 500; i++) { WhoCares = p->Get_Hits(Commands->Get_ID(o),1.0f,Get_Held_Weapon((Stewie_SoldierGameObj *)o)->Definition->PrimaryAmmoDefId); }
			DebugMessage("PLAYERGETHITS2 %s %d %d",p->name.c_str(),clock(),WhoCares);
		} else {
			DebugMessage("NOPLAYER");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!d3")) {
#ifdef DEBUG
		m_dumpLog();
#else
		DebugMessage("Cannot commit memory dump in non-debug mode.");
#endif
		return false;
	}
#endif
	if (!stricmp(cmd.c_str(),"!debug8291")) {
		unsigned int OCount = 0, PCount = 0, HitsArr = 0, SeensArr = 0;
		for (GenericSLNode *Node = BaseGameObjList->HeadNode; Node; Node = Node->NodeNext) {
			if (!Node->NodeData) { continue; }
			OCount++;
		}
		for (unsigned int i = 0; i < vPManager->Get_Player_Count(); i++) {
			if (!vPManager->Data[i]) { continue; }
			PCount++;
			HitsArr += vPManager->Data[i]->Hits.size();
			SeensArr += vPManager->Data[i]->SeenList.size();
		}
		unsigned int VehArr = vVManager->Get_Vehicle_Count();
		unsigned int VetPtsArr = vVetManager->Pending_VetPoints.size();
		unsigned int CheatMsgsArr = vPManager->CheatMessages.size();
		unsigned int RecsArr = vPManager->Recommendations.size();
		unsigned int BountiesArr = vPManager->Bounties.size();
		Stewie_cNetwork::Update_Fps();
		HostMessage("[DEBUG1] SFPS: %d ;; Objects: %u ;; V: %u, H: %u, S: %u, P: %u, C: %u, R: %u, B: %u, Total: %u (%u)",Stewie_cNetwork::Fps,OCount,VehArr,HitsArr,SeensArr,VetPtsArr,CheatMsgsArr,RecsArr,BountiesArr,(VehArr + HitsArr + SeensArr + VetPtsArr + CheatMsgsArr + RecsArr + BountiesArr),PCount);
		return false;
	} else if (!stricmp(cmd.c_str(),"!rweapon") || !stricmp(cmd.c_str(),"!wdrop")) {
		int team = Commands->Get_Player_Type(o);
		float d = FLT_MAX;
		Vector3 Position = Commands->Get_Position(o);
		if (team == 1) { d = Find_Distance_To_Closest_Object_By_Preset(Position,"pct_zone_gdi"); }
		else if (team == 0) { d = Find_Distance_To_Closest_Object_By_Preset(Position,"pct_zone_nod"); }
		if (d > 30.0f) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must be closer to a friendly Purchase Terminal to drop weapons.");
		} else {
			const char *Wep = Get_Current_Weapon(o);
			if (Get_Weapon_Count(o) <= 1) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your weapon bag is too empty to drop any more weapons!");
			} else if (Get_Current_Total_Bullets(o) == 0) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You cannot drop a weapon with no ammo.");
			} else if (isin(Wep,"pistol") || isin(Wep,"timed")) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Default weapons cannot be dropped.");
			} else {
				vPlayer *p = vPManager->Get_Player(ID);
				bool CanDrop = true;
				if (p) {
					CanDrop = false;
					for (unsigned int i = 0; i < p->WeaponBag.size(); i++) {
						if (!p->WeaponBag[i]) { continue; }
						if (!stricmp(Wep,p->WeaponBag[i]->Get_Name())) {
							CanDrop = true;
							p->WeaponBag.erase(p->WeaponBag.begin() + i);
							break;
						}
					}
				}
				if (CanDrop == false) {
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Default weapons cannot be dropped.");
					return false;
				}
				Remove_Weapon(o,Wep);
				Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)o);
				if (bag->Vector.Count() >= 1) { bag->Select_Index(1); }
				vWManager->Create_Pack_Weapon(World_Position((Stewie_BaseGameObj *)o),Wep,true,false);
			}
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!buy")) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (p && tokens >= 2) {
			std::string type = Str->Gettok(2);
			int Recs = vPManager->Get_Current_Recs(ID);
			if (vManager.DurationAsInt() <= Config->VetCmdCooldown) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must be %s into the game to use the !buy feature.",Duration(Config->VetCmdCooldown).c_str());
			} else if (!stricmp(type.c_str(),"veh")) {
				int Cost = 15;
				if (Recs > Cost) { vPManager->Recommend(-1,ID,-1 * Cost,vRECNONE); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; you do not have enough recommendations for this. (Current: %d, Required: %d)",Recs,Cost); return false; }

				Vector3 pos = Commands->Get_Position(o);
				pos.Z += 5.0f;
				GameObject *v = Commands->Create_Object(Find_Random(VehicleCID,Commands->Get_Player_Type(o))->Get_Name(),pos);
				Commands->Set_Player_Type(v,-2);
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s.",Get_Pretty_Name(v).c_str());
				vVetManager->Upgrade(v,p->VetRank - 1,2);
			} else if (!stricmp(type.c_str(),"char")) {
				int Cost = 10;
				if (Recs > Cost) { vPManager->Recommend(-1,ID,-1 * Cost,vRECNONE); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; you do not have enough recommendations for this. (Current: %d, Required: %d)",Recs,Cost); return false; }

				const char *ch = "";
				while (Get_Cost(ch) <= 0) { ch = Find_Random(SoldierCID,Commands->Get_Player_Type(o))->Get_Name(); }
				Change_Character(o,ch);
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s character.",Get_Pretty_Name(ch).c_str());
			} else if (!stricmp(type.c_str(),"wep") || !stricmp(type.c_str(),"weapon")) {
				int Cost = 5;
				if (Recs > Cost) { vPManager->Recommend(-1,ID,-1 * Cost,vRECNONE); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; you do not have enough recommendations for this. (Current: %d, Required: %d)",Recs,Cost); return false; }

				Stewie_WeaponDefinitionClass *w = (Stewie_WeaponDefinitionClass *)Find_Random(WeaponCID);
				Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)o);
				int bullets = w->DSClipSize.Get() + w->DSMaxClipBullets.Get();
				if (w->DSMaxClipBullets.Get() == -1) { bullets = -1; }
				bag->Add_Weapon(w,bullets,true);

				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s.",Get_Pretty_Name(w->Get_Name()).c_str());
			} else if (!stricmp(type.c_str(),"spy")) {
				int Cost = 50;
				if (Recs > Cost) { vPManager->Recommend(-1,ID,-1 * Cost,vRECNONE); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; you do not have enough recommendations for this. (Current: %d, Required: %d)",Recs,Cost); return false; }

				Change_Character(o,"CnC_Nod_FlameThrower_2SF");
				Set_Is_Visible((Stewie_ScriptableGameObj *)o,false);
				HostMessage("[VGM] Warning! Player %s has purchased a Spy.",p->Get_Name());
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a Spy.");
			} else if (!stricmp(type.c_str(),"god")) {
				int Cost = 50;
				if (Recs > Cost) { vPManager->Recommend(-1,ID,-1 * Cost,vRECNONE); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; you do not have enough recommendations for this. (Current: %d, Required: %d)",Recs,Cost); return false; }

				if (Commands->Get_Player_Type(o) == 1) {
					Change_Character(o,"CnC_GDI_RocketSoldier_2SF_Secret");
					Commands->Clear_Weapons(o);
					Commands->Give_Powerup(o,"CnC_POW_VoltAutoRifle_Player",true);
				} else {
					Change_Character(o,"CnC_GDI_RocketSoldier_2SF_Secret");
					Commands->Clear_Weapons(o);
					Commands->Give_Powerup(o,"CnC_POW_VoltAutoRifle_Player_Nod",true);
				}
				float MH = Commands->Get_Max_Health(o), MSS = Commands->Get_Max_Shield_Strength(o);
				Stewie_SoldierGameObjDef *SDef = Get_Soldier_Definition(o);
				if (SDef) {
					MH = (MH / SDef->HealthMax.Get()) * 500.0f;
					MSS = (MSS / SDef->ShieldStrengthMax.Get()) * 500.0f;
				} else {
					MH = 500.0f;
					MSS = 500.0f;
				}
				Set_Max_Health(o,MH);
				Set_Max_Shield_Strength(o,MSS);
				Commands->Give_Powerup(o,"CnC_POW_MineRemote_02",true);
				Commands->Give_Powerup(o,"CnC_POW_MineTimed_Player_02",true);
				Commands->Give_Powerup(o,"CnC_POW_MineProximity_05",true);
				Commands->Give_Powerup(o,"POW_Pistol_Player",true);
				Commands->Set_Shield_Type(o,"SkinChemWarrior");
				Grant_Refill(o);
				Remove_Script(o,"SelfRepair");
				Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)o);
				if (bag && bag->Vector.Count() >= 1) { bag->Select_Index(1); }
				char params[64];
				sprintf(params,"%f,%f",1.0f,1.0f);
				Commands->Attach_Script(o,"SelfRepair",params);
				HostMessage("[VGM] Warning! Player %s has purchased a God.",p->Get_Name());
				if (Config->Sounds) { Create_2D_WAV_Sound("m00gemg_atoc0001i1gemg_snd.wav"); }
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a God.");
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; please make sure you have requested one of the following to buy: veh, char, wep, spy, god.");
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Error; please make sure you have requested one of the following to buy: veh, char, wep, spy, god.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!wep") || !stricmp(cmd.c_str(),"!weapon")) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (p && p->VetRank >= Config->VetLevels - 2) {
			if (p->LastVetCmds[0] < vManager.DurationAsInt() - Config->VetCmdCooldown) {
				FindWep:
					Stewie_WeaponDefinitionClass *w = (Stewie_WeaponDefinitionClass *)Find_Random(WeaponCID);
					if (isin(w->Get_Name(),"beacon")) { goto FindWep; }
				Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)o);
				int bullets = w->DSClipSize.Get() + w->DSMaxClipBullets.Get();
				if (w->DSMaxClipBullets.Get() == -1) { bullets = -1; }
				bag->Add_Weapon(w,bullets,true);

				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s.",Get_Pretty_Name(w->Get_Name()).c_str());
				p->LastVetCmds[0] = vManager.DurationAsInt();
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must wait another %d seconds to do this.",(Config->VetCmdCooldown - vManager.DurationAsInt() + p->LastVetCmds[0]));
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your veteran level is not high enough to do this.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!char")) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (p && p->VetRank >= Config->VetLevels - 1) {
			if (p->LastVetCmds[1] < vManager.DurationAsInt() - Config->VetCmdCooldown) {
				const char *ch = Find_Random(SoldierCID,Commands->Get_Player_Type(o))->Get_Name();
				Change_Character(o,ch);
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s character.",Get_Pretty_Name(ch).c_str());
				p->LastVetCmds[1] = vManager.DurationAsInt();
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must wait another %d seconds to do this.",(Config->VetCmdCooldown - vManager.DurationAsInt() + p->LastVetCmds[1]));
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your veteran level is not high enough to do this.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!veh")) {
		vPlayer *p = vPManager->Get_Player(ID);
		int team = Commands->Get_Player_Type(o);
		if (p && p->VetRank >= Config->VetLevels) {
			if (p->LastVetCmds[2] < vManager.DurationAsInt() - Config->VetCmdCooldown) {
				float d = FLT_MAX;
				Vector3 Pos = Commands->Get_Position(o);
				if (team == 1) { d = Find_Distance_To_Closest_Object_By_Preset(Pos,"pct_zone_gdi"); }
				else if (team == 0) { d = Find_Distance_To_Closest_Object_By_Preset(Pos,"pct_zone_nod"); }
				else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must be on %s or %s to do this.",Get_Translated_Team_Name(1).c_str(),Get_Translated_Team_Name(0).c_str()); return false; }
				if (d > 40) { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You are too far from a Purchase Terminal to do this."); return false; }
				Vector3 pos = Commands->Get_Position(o);
				pos.Z += 5.0f;
				GameObject *v = Commands->Create_Object(Find_Random(VehicleCID,team)->Get_Name(),pos);
				Commands->Set_Player_Type(v,-2);
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been given a(n) %s.",Get_Pretty_Name(v).c_str());
				vVetManager->Upgrade(v,p->VetRank - 1,2);
				p->LastVetCmds[2] = vManager.DurationAsInt();
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must wait another %d seconds to do this.",(Config->VetCmdCooldown - vManager.DurationAsInt() + p->LastVetCmds[2]));
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your veteran level is not high enough to do this.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!ach") || !stricmp(cmd.c_str(),"!achieves") || !stricmp(cmd.c_str(),"!achievements") || !stricmp(cmd.c_str(),"!vault")) {
		const char *nick = Get_Player_Name_By_ID(ID);
		bool allowed = false;
		if (Config->MedalsRestricted) {
			for (int totry = 1; totry <= 256; totry++) {
				char entry[64];
				sprintf(entry,"User%d",totry);
				char result[512];
				if (getProfileString("MedalUsers",(const char *)entry,"",result,512,"vgm_users.ini")) {
					if (!stricmp(result,nick)) {
						allowed = true;
						break;
					}
				}
			}
		} else {
			allowed = true;
			for (int totry = 1; totry <= 256; totry++) {
				char entry[64];
				sprintf(entry,"User%d",totry);
				char result[512];
				if (getProfileString("MedalUsers",(const char *)entry,"",result,512,"vgm_users.ini")) {
					if (!stricmp(result,nick)) {
						allowed = false;
						break;
					}
				}
			}
		}
		if (allowed) {
			int TotalMedals = 0;
			for (unsigned int j = 0; j < (int)vMManager->vLAST; j++) {
				char m[16],f[128];
				sprintf(m,"Medal%d",j);
				sprintf(f,"vgm\\medals_%s.ini",nick);
				int Medals = getProfileInt("Medals",(const char*)m,0,(const char*)f);
				TotalMedals += Medals;
				if (Medals > 0) { PrivMsgColoredVA(ID,2,0,200,200,"[VGM] %s times Achieved: %d",AwardNames[j],Medals); }
			}
			if (TotalMedals == 0) { PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You have no achievements."); }
			else { PrivMsgColoredVA(ID,2,0,200,200,"[VGM] Total %d achievements.",TotalMedals); }
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!com") || !stricmp(cmd.c_str(),"!command") || !stricmp(cmd.c_str(),"!commander")) {
		if (tokens == 1) {
			int team = Commands->Get_Player_Type(o);
			if (team == 1 && GDICommander > 0) { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your current Team Commander is: %s.",Player_Name_From_ID(GDICommander).c_str()); }
			else if (team == 0 && NodCommander > 0) { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your current Team Commander is: %s.",Player_Name_From_ID(NodCommander).c_str()); }
			else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your team currently has no Team Commander."); }
		} else if (tokens == 2) {
			if (Is_Commander(ID,Commands->Get_Player_Type(o),false) == false) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You are not the Commander for your team.");
				return false;
			}
			std::string order = Str->Gettok(2);
			if (!stricmp(order.c_str(),"o") || !stricmp(order.c_str(),"order") || !stricmp(order.c_str(),"w") || !stricmp(order.c_str(),"warn") || !stricmp(order.c_str(),"harv")) {
				PrivMsgColoredVA(ID,2,0,200,0,"Invalid parameters.");
			} else if (!stricmp(order.c_str(),"c4")) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] %s C4: %s",Get_Translated_Team_Name(Commands->Get_Player_Type(o)).c_str(),Find_Proxy_C4_On_Buildings(Commands->Get_Player_Type(o)).c_str());
			}
		} else if (tokens >= 3) {
			if (Is_Commander(ID,Commands->Get_Player_Type(o),false) == false) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You are not the Commander for your team.");
				return false;
			}
			std::string order = Str->Gettok(2);
			if (!stricmp(order.c_str(),"o") || !stricmp(order.c_str(),"order")) {
				std::string message = Str->Gettok(3,-1);
				char m[512];
				sprintf(m,"Order from Commander: %s",message.c_str());
				Send_Private_Message_Team(Commands->Get_Player_Type(o),m);
			} else if (!stricmp(order.c_str(),"w") || !stricmp(order.c_str(),"warn")) {
				std::string message = Str->Gettok(3,-1);
				char m[512];
				sprintf(m,"WARNING from Commander: %s",message.c_str());
				Send_Private_Message_Team(Commands->Get_Player_Type(o),m);
			} else if (!stricmp(order.c_str(),"harv") && Team_Refinery(Commands->Get_Player_Type(o))) {
				std::string message = Str->Gettok(3,-1);
				if (!stricmp(message.c_str(),"start")) {
					Stewie_HarvesterClass *HarvCheck = Team_Refinery(Commands->Get_Player_Type(o))->Harvester;
					if (!HarvCheck) {
						PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your team's Harvester is not available.");
						return false;
					}
					Stewie_HarvesterClass &Harvester = *HarvCheck;
					if (Harvester.State == 3) { Harvester.Go_Unload_Tiberium(); }
					else { Harvester.Go_Harvest(); }
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Harvester started.");
				} else if (!stricmp(message.c_str(),"stop")) {
					Stewie_BaseControllerClass *Base = Stewie_BaseControllerClass::Find_Base(Commands->Get_Player_Type(o));
					Stewie_HarvesterClass *HarvCheck = ((Stewie_RefineryGameObj *)(Stewie_BuildingGameObj *)(Base->Find_Building(vRefinery)))->Harvester;
					if (!HarvCheck) {
						PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your team's Harvester is not available.");
						return false;
					}
					Stewie_HarvesterClass &Harvester = *HarvCheck;
					// TO-DO: Doesn't work whilst harvesting.
					Harvester.Stop();
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Harvester stopped.");
				} else {
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Invalid Command. Please choose %s %s [start/stop].",cmd,order.c_str());
				}
				return false;
			}
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!d") || !stricmp(cmd.c_str(),"!donate")) {
		if (tokens >= 3) {
			std::string player = Str->Gettok(2);
			std::string amt = Str->Gettok(3);
			float amount = 0.0f;
			if ((unsigned int)Config->DonateTime > vManager.DurationAsUint()) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] The map must have elapsed %.0f minutes to donate.",float(Config->DonateTime) / 60.0f);
				return false;
			} else if (!stricmp(amt.c_str(),"all")) {
				amount = Commands->Get_Money(o);
			} else {
				amount = (float)atof(amt.c_str());
			}
			if (Commands->Get_Money(o) < amount) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You don't have %.0f credits.",amount);
				return false;
			} else if (amount <= 0.0f) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You can't donate negative credits!");
				return false;
			}
			GameObject *d = Player_GameObj_By_Name(player.c_str());
			if (!d) { d = Player_GameObj_By_Part_Name(player.c_str()); }
			if (d) {
				d = Player_GameObj_By_Part_Name(player.c_str());
				if (d == o) {
					PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You cannot donate to yourself.");
					return false;
				}
				if (Commands->Get_Player_Type(d) != Commands->Get_Player_Type(o)) {
					PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You cannot donate to the other team.");
					return false;
				}
				Commands->Give_Money(o,-1 * amount,false);
				Commands->Give_Money(d,amount,false);
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You donated %.0f credits to %s.",amount,Player_Name_From_GameObj(d).c_str());
				PrivMsgColoredVA(Get_Player_ID(d),2,0,200,200,"[VGM] You have received %.0f credits from %s.",amount,Player_Name_From_ID(ID).c_str());
				if (Config->Sounds) { Create_2D_WAV_Sound_Player(d,"m00pc$$_aqob0002i1evag_snd.wav"); }
				return false;
			} else {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] Player \"%s\" not found.",player.c_str());
				return false;
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You must specify an amount and a username! EG: %s Someone 500",cmd.c_str());
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!td") || !stricmp(cmd.c_str(),"!tdonate")) {
		if (tokens >= 2) {
			int team = Commands->Get_Player_Type(o);
			std::string amt = Str->Gettok(2);
			float amount = 0.0f;
			if ((unsigned int)Config->DonateTime > vManager.DurationAsUint()) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] The map must have elapsed %.0f minutes to donate.",float(Config->DonateTime) / 60.0f);
				return false;
			} else if (!stricmp(amt.c_str(),"all")) {
				amount = Commands->Get_Money(o);
			} else {
				amount = (float)atof(amt.c_str());
			}
			if (amount <= 0.0f) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You can't donate negative credits!");
				return false;
			}
			if ((unsigned int)Config->DonateTime > vManager.DurationAsUint()) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] The map must have elapsed %.0f minutes to donate.",float(Config->DonateTime) / 60.0f);
			} else if (Stewie_cPlayerManager::Tally_Team_Size(team) < 2) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You are the only one on your team.");
			} else if (amount > Commands->Get_Money(o)) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You do not have that much money.");
			} else if (amount > 0) {
				int divisors = (int)(Stewie_cPlayerManager::Tally_Team_Size(team) - 1);
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have donated %.0f credits to your team. (%.0f each)",amount,floor(float(amount) / float(divisors)));
				Commands->Give_Money(o,-1 * amount,false);
				amount = floor(float(amount) / float(divisors));
				Commands->Give_Money(o,-1 * amount,false);
				Give_Money_To_All_Players(amount,team);
				PrivMsgColoredVA(-1 * ID,team,0,200,0,"[VGM] You have received %.0f credits from %s.",amount,Player_Name_From_ID(ID).c_str());
				Create_2D_WAV_Sound_Team("m00pc$$_aqob0002i1evag_snd.wav",team);
			} else {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You must specify an amount greater than zero.");
			}
		} else {
			PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You must specify an amount! EG: %s 500",cmd.c_str());
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!tp")) {
		int team = Commands->Get_Player_Type(o);
		if (tokens >= 2) {
			int amount = atoi(Str->Gettok(2).c_str());
			if (amount > (int)Commands->Get_Money(o)) {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You do not have that much money.");
			} else if (amount > 0) {
				if (team == 1) { GDITeamPool += amount; }
				else if (team == 0) { NODTeamPool += amount; }
				else { return false; }
				Commands->Give_Money(o,(float)(-1 * amount),false);
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You have donated %d credits to the team pool.",amount);
				if (Has_Commander(team)) {
					PrivMsgColoredVA(team == 1 ? GDICommander : NodCommander,2,0,200,200,"[VGM] %s has donated %d credits to the team pool!",Player_Name_From_ID(ID).c_str(),amount);
				}
			} else {
				PrivMsgColoredVA(ID,2,0,200,200,"[VGM] You must specify an amount greater than zero.");
			}
		} else if (Is_Commander(ID,team,false)) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The %s Team Pool currently has %d credits.",Get_Translated_Team_Name(team).c_str(),team == 1 ? GDITeamPool : NODTeamPool);
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!c4")) {
		int Team = Commands->Get_Player_Type(o);
		if (Team == 1) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Remote: %d/%d - Timed: %d/%d - Proximity: %d/%d",Get_C4_Count_Remote(Team),Config->GDIRemoteC4Limit,Get_C4_Count_Timed(Team),Config->GDITimedC4Limit,Get_C4_Count_Proximity(Team),Config->GDIProximityC4Limit);
		} else if (Team == 0) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Remote: %d/%d - Timed: %d/%d - Proximity: %d/%d",Get_C4_Count_Remote(Team),Config->NODRemoteC4Limit,Get_C4_Count_Timed(Team),Config->NODTimedC4Limit,Get_C4_Count_Proximity(Team),Config->NODProximityC4Limit);
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] C4 limits not available.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!bind")) {
		GameObject *v = Get_Vehicle(o);
		if (v) {
			vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(v));
			if (veh) {
				if (veh->Locked == 0) {
					int VehType = Get_Vehicle_Parent_Type(v);
					if (VehType == Commands->Get_Player_Type(o)) {
						int c = vVManager->Count_Player_Bound_Vehicles(ID,false);
						if (c < Config->MaxFriendlyVeh) {
							veh->Lock(ID,1);
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle (%s) has been bound to you.",Get_Pretty_Name(v).c_str());
						} else {
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You cannot bind any more friendly vehicles.");
						}
					} else {
						int c = vVManager->Count_Player_Bound_Vehicles(ID,true);
						if (c < Config->MaxEnemyVeh) {
							veh->Lock(ID,1);
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle (%s) has been bound to you.",Get_Pretty_Name(v).c_str());
						} else {
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You cannot bind any more enemy vehicles.");
						}
					}
				} else {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle is already bound.");
				}
			}
		} else {
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You must be in a vehicle to use this command.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!lock") || !stricmp(cmd.c_str(),"!bl")) {
		GameObject *v = Get_Vehicle(o);
		if (v) {
			vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(v));
			if (veh) {
				if (veh->Locked == 0) {
					int VehType = Get_Vehicle_Parent_Type(v);
					if (VehType == Commands->Get_Player_Type(o)) {
						int c = vVManager->Count_Player_Bound_Vehicles(ID,false);
						if (c < Config->MaxFriendlyVeh) {
							veh->Lock(ID,2);
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle (%s) has been bound and locked to you.",Get_Pretty_Name(v).c_str());
						} else {
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You cannot bind any more friendly vehicles.");
						}
					} else {
						int c = vVManager->Count_Player_Bound_Vehicles(ID,true);
						if (c < Config->MaxEnemyVeh) {
							veh->Lock(ID,2);
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle (%s) has been bound and locked to you.",Get_Pretty_Name(v).c_str());
						} else {
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You cannot bind any more enemy vehicles.");
						}
					}
				} else if (veh->Locked == 1) {
					if (veh->Owner == ID) {
						veh->Lock(ID,2);
						PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle (%s) has been locked to you.",Get_Pretty_Name(v).c_str());
					} else {
						PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This is not your vehicle!");
					}
				} else {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle is already locked.");
				}
			}
		} else {
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You must be in a vehicle to use this command.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!unlock")) {
		if (Get_Vehicle(o)) {
			vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(Get_Vehicle(o)));
			if (veh) {
				if (veh->Owner != ID) {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This is not your vehicle!");
				} else if (veh->Locked < 2) {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle is not locked.");
				} else {
					veh->Lock(ID,1);
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] Your vehicle (%s) has been unlocked.",Get_Pretty_Name(Get_Vehicle(o)).c_str());
				}
			}
		} else {
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You must be in a vehicle to use this command.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!unbind") || !stricmp(cmd.c_str(),"!unbl") || !stricmp(cmd.c_str(),"!ub")) {
		if (!stricmp(Str->Gettok(2).c_str(),"all")) {
			vVManager->Unbind_All_Player_Vehicles(ID);
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] All of your vehicles have been unbound.");
		} else if (Get_Vehicle(o)) {
			vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(Get_Vehicle(o)));
			if (veh) {
				if (veh->Locked == 0) {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This vehicle is not bound.");
				} else if (veh->Owner != ID) {
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] This is not your vehicle!");
				} else if (veh->Locked > 0) {
					veh->Lock(ID,0);
					PrivMsgColoredVA(ID,2,200,200,0,"[VGM] Your vehicle (%s) has been unbound.",Get_Pretty_Name(Get_Vehicle(o)).c_str());
				}
			}
		} else {
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You must be in a vehicle to use this command.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!vlist")) {
		PrivMsgColoredVA(ID,2,200,200,0,"[VGM] Vehicles Bound: %s.",vVManager->Get_Bound_Vehicles(ID).c_str());
		return false;
	} else if (!stricmp(cmd.c_str(),"!vkick")) {
		if (tokens >= 2) {
			std::string nick = Str->Gettok(2);
			GameObject *d = Player_GameObj_By_Part_Name(nick.c_str());
			if (d) {
				nick = Player_Name_From_GameObj(d);
				if (d == o) { PrivMsgColoredVA(ID,2,200,200,0,"[VGM] You cannot kick yourself from your bound vehicles."); }
				else {
					GameObject *v = Get_Vehicle(d);
					if (!v) { PrivMsgColoredVA(ID,2,200,200,0,"[VGM] %s is not in a vehicle.",nick.c_str()); }
					else {
						vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(v));
						if (veh && veh->Owner == ID && veh->Locked > 0) {
							Force_Occupant_ID_Exit(v,Get_Player_ID(d));
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] %s has been kicked from your bound vehicle.",nick.c_str());
						} else {
							PrivMsgColoredVA(ID,2,200,200,0,"[VGM] %s is not in any of your bound vehicles.",nick.c_str());
						}
					}
				}
			} else {
				PrivMsgColoredVA(ID,2,200,200,0,"[VGM] \"%s\" not found.",nick.c_str());
			}
		} else {
			vVManager->Kick_All_Players_From_My_Vehicles(ID);
			PrivMsgColoredVA(ID,2,200,200,0,"[VGM] All players were kicked from your vehicles.");
		}
		return false;
	}
	return true;
}
bool Execute_SniperCommand(int ID, vTokenParser *Str, int type) {
	std::string cmd = Str->Gettok(1);
	int Tokens = Str->Numtok();
	GameObject *o = Player_GameObj(ID);
	int maxdist = 18;
	if (!stricmp(cmd.c_str(),"!killme")) {
		Kill_Player(ID);
		PrivMsgColoredVA(ID,2,200,0,0,"[VGM] You have been killed.");
		return false;
	}
	return true;
}
bool Execute_OtherCommand(int ID, vTokenParser *Str, int type) {
	std::string cmd = Str->Gettok(1);
	int tokens = Str->Numtok();
	GameObject *o = Player_GameObj(ID);
	if (!stricmp(cmd.c_str(),"!recs") || !stricmp(cmd.c_str(),"!myrecs")) {
		int pID = ID;
		if (tokens >= 2) {
			std::string player = Str->Gettok(2);
			GameObject *d = Player_GameObj_By_Name(player.c_str());
			if (!d) { d = Player_GameObj_By_Part_Name(player.c_str()); }
			if (d) { pID = Get_Player_ID(d); }
			else { PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Player \"%s\" not found.",player.c_str()); return false; }
		}
		vPlayer *p = vPManager->Get_Player(pID);
		if (!p) { return false; }
		char f[128];
		sprintf(f,"vgm\\medals_%s.ini",p->Get_Name());
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Current Recs for %s: %d - Kill Recs: %d - KD Recs: %d - MVP Recs: %d - Misc Recs: %d",
			p->Get_Name(),
			getProfileInt("Medals","Recs",0,(const char*)f),
			getProfileInt("Medals","KillRecs",0,(const char*)f),
			getProfileInt("Medals","KDRecs",0,(const char*)f),
			getProfileInt("Medals","MVPRecs",0,(const char*)f),
			getProfileInt("Medals","MiscRecs",0,(const char*)f)
		);
		return false;
	} else if (!stricmp(cmd.c_str(),"!rec")) {
		if (tokens >= 2) {
			vPlayer *p = vPManager->Get_Player(ID);
			if (!p) { return false; }
			std::string player = Str->Gettok(2);
			std::string reason("No reason");
			if (tokens >= 3) { reason = Str->Gettok(3,-1); }
			GameObject *d = Player_GameObj_By_Name(player.c_str());
			if (!d) { d = Player_GameObj_By_Part_Name(player.c_str()); }
			if (d == o) {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You cannot recommend yourself.");
				return false;
			} else if (d) {
				if (vPManager->Recommend(ID,Get_Player_ID(d),1)) {
					HostMessage("[VGM] %s has been recommended by %s for: %s",Player_Name_From_GameObj(d).c_str(),Player_Name_From_ID(ID).c_str(),reason.c_str());
					p->LastRecommend = (int)clock();
				} else {
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have rec'd/n00b'd someone too recently. Please wait to do it again.");
				}
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Player \"%s\" not found.",player.c_str());
				return false;
			}
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!n00b") || !stricmp(cmd.c_str(),"!noob")) {
		if (tokens >= 2) {
			vPlayer *p = vPManager->Get_Player(ID);
			if (!p) { return false; }
			std::string player = Str->Gettok(2);
			std::string reason("No reason");
			if (tokens >= 3) { reason = Str->Gettok(3,-1); }
			GameObject *d = Player_GameObj_By_Name(player.c_str());
			if (!d) { d = Player_GameObj_By_Part_Name(player.c_str()); }
			if (d) {
				if (vPManager->Recommend(ID,Get_Player_ID(d),-1)) {
					HostMessage("[VGM] %s has been n00bed by %s for: %s",Player_Name_From_GameObj(d).c_str(),Player_Name_From_ID(ID).c_str(),reason.c_str());
					p->LastRecommend = (int)clock();
				} else {
					PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have rec'd/n00b'd someone too recently. Please wait to do it again.");
				}
			} else {
				PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Player \"%s\" not found.",player.c_str());
				return false;
			}
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!tc") && tokens == 1) {
		int MyTeam = Commands->Get_Player_Type(o);
		if (MyTeam != 0 && MyTeam != 1) { return false; }
		int EnTeam = 1 - MyTeam;
		//Stewie_cTeam *MyCTeam = Stewie_cTeamManager::Find_Team(MyTeam);
		//Stewie_cTeam *EnCTeam = Stewie_cTeamManager::Find_Team(EnTeam);
		if (Stewie_cPlayerManager::Tally_Team_Size(EnTeam) <= Stewie_cPlayerManager::Tally_Team_Size(MyTeam) - 2) { // && MyCTeam->Score >= EnCTeam->Score + 1000
			Change_Team(o,EnTeam);
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have been switched to %s.",Get_Translated_Team_Name(EnTeam).c_str());
			HostMessage("[VGM] %s has changed to %s to even the teams.",Player_Name_From_ID(ID).c_str(),Get_Translated_Team_Name(EnTeam).c_str());
		} else {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You cannot change teams under these conditions.");
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!rtc")) {
		vPManager->RTC(ID);
		return false;
	} else if (!stricmp(cmd.c_str(),"!bounty")) {
		if (tokens >= 3) {
			std::string player = Str->Gettok(2);
			std::string amt = Str->Gettok(3);
			float amount;
			if (!stricmp(amt.c_str(),"all")) { amount = Commands->Get_Money(o); }
			else { amount = (float)atof(amt.c_str()); }
			if (Commands->Get_Money(o) < amount) {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] You don't have %.0f credits.",amount);
				return false;
			}
			if (amount < 1000.0f) {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] You must place at least $1000 to start a bounty.");
				return false;
			}
			GameObject *d = Player_GameObj_By_Name(player.c_str());
			if (!d) { d = Player_GameObj_By_Part_Name(player.c_str()); }
			if (!d) {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] Player \"%s\" not found.",player.c_str());
				return false;
			}
			if (d == o) {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] You cannot place a bounty on yourself.");
				return false;
			} else if (vPManager->Get_Bounty(ID,Get_Player_ID(d)) > 0.0f) {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] You must wait for your current bounty on this player to expire.");
				return false;
			}
			if (vPManager->Place_Bounty(ID,Get_Player_ID(d),amount)) {
				Commands->Give_Money(o,-1 * amount,false);
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] You have placed a bounty of $%.0f on %s.",amount,Player_Name_From_GameObj(d).c_str());
				PrivMsgColoredVA(Get_Player_ID(d),2,128,0,0,"[VGM] %s has placed a bounty of %.0f on you.",Player_Name_From_ID(ID).c_str(),amount);
				HostMessage("[VGM] %s has placed a bounty of %.0f on %s!",Player_Name_From_ID(ID).c_str(),amount,Player_Name_From_GameObj(d).c_str());
				Create_2D_WAV_Sound("l06b_11_rav03.wav");
			} else {
				PrivMsgColoredVA(ID,2,128,0,0,"[VGM] Invalid amount or target.");
			}
			return false;
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!vets")) {
		char rank[128],entry[64];
		int total = getProfileInt("VeteranPresets","Levels",0,"vgm_presets.ini");
		for (int i = 2; i <= total; i++) { // for (int i = total; i > 0; i--) {
			vTokenParser *S = new vTokenParser("");
			S->Parse(", ");
			for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
				if (!Node->NodeData) { continue; }
				if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
				vPlayer *p = vPManager->Get_Player(Node->NodeData->PlayerId);
				if (!p) { continue; }
				if (p->VetRank == i) { S->Addtok(p->Get_Name()); }
			}
			if (S->Numtok()) {
				sprintf(entry,"Level%d",i);
				getProfileString("VeteranPresets",entry,"Recruit",rank,128,"vgm_presets.ini");
				HostMessage("[VGM] %ss: %s",rank,S->Get().c_str());
			}
			S->Delete();
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!vet") || !stricmp(cmd.c_str(),"!vetstatus") || !stricmp(cmd.c_str(),"!vetstats")) {
		vPlayer *p = vPManager->Get_Player(ID);
		if (!p) { return false; }

		char rank[128],entry[64];
		sprintf(entry,"Level%d",p->VetRank);
		getProfileString("VeteranPresets",entry,"Recruit",rank,128,"vgm_presets.ini");
		if (!rank) { return false; }
		if (p->VetRank >= (int)vVetManager->Promo_ScoreReqd.size()) { return false; }

		float missingscore = vVetManager->Promo_ScoreReqd[p->VetRank] - Get_Score(ID);
		float missingpoints = vVetManager->Promo_VetPtsReqd[p->VetRank] - p->VetPoints;
		int missingtime = vVetManager->Promo_TimeIngameReqd[p->VetRank] - p->SecondsIngame;

		if (p->VetRank >= Config->VetLevels || (missingscore <= 0.0f && missingpoints <= 0.0f && missingtime < 0)) {
			PrivMsgColoredVA(ID,2,255,119,51,"[VGM] Rank: %s - Veteran Points: %.2f - You cannot receive any more promotions.",rank,p->VetPoints);
		} else {
			vTokenParser *Header = new vTokenParser("");
			Header->Parse("/");
			vTokenParser *Infos = new vTokenParser("");
			Infos->Parse("/");
			char m[512];
			if (missingscore > 0.0f) {
				Header->Addtok("Score");
				sprintf(m,"%.0f",MinZero(missingscore));
				Infos->Addtok(m);
			}
			if (missingpoints > 0.0f) {
				Header->Addtok("VetPoints");
				sprintf(m,"%.0f",MinZero(missingpoints));
				Infos->Addtok(m);
			}
			if (missingtime > 0) {
				Header->Addtok("Time");
				Infos->Addtok(Duration(missingtime).c_str());
			}
			if (strlen(Header->Get().c_str()) > 0 && strlen(Infos->Get().c_str()) > 0) {
				PrivMsgColoredVA(ID,2,255,119,51,"[VGM] Rank: %s - Veteran Points: %.2f - %s to promotion: %s",rank,p->VetPoints,Header->Get().c_str(),Infos->Get().c_str());
			} else {
				PrivMsgColoredVA(ID,2,255,119,51,"[VGM] Rank: %s - Veteran Points: %.2f - Promotion data unavailable. Please retry.",rank,p->VetPoints);
			}
			Header->Delete();
			Infos->Delete();
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!ping")) {
		Stewie_cPlayer *p = Stewie_cPlayerManager::Find_Player(ID);
		if (!p || p->IsInGame == false || p->IsActive == false) { return false; }
		HostMessage("[VGM] %s, your ping is: %d.",WCharToStr(p->PlayerName.m_Buffer).c_str(),p->Get_Ping());
		return false;
	} else if (!stricmp(cmd.c_str(),"!speed") || !stricmp(cmd.c_str(),"!setspeed") || !stricmp(cmd.c_str(),"!ss")) {
		if (Is_Spectating(o) == false) { return false; }
		int i = atoi(Str->Gettok(2).c_str());
		if (i <= 0 || i > 100) {
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Invalid speed.");
		} else {
			Set_Speed((Stewie_SoldierGameObj *)o,float(i));
			PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your speed is now %d.",i);
		}
		return false;
	} else if (!stricmp(cmd.c_str(),"!time")) {
		int time = vManager.DurationAsInt();
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The map %s has elapsed %s.",GameDataObj()->MapName,Duration(time).c_str());
		return false;
	}
	return true;
}
bool Hide_Command(int ID, vTokenParser *Str, int type) {
	std::string cmd = Str->Gettok(1);
	char cmd2[512];
	sprintf(cmd2," %s ",cmd.c_str());
	if (isin(" !afk !allow !amsg !atm !back !ban !cmd !disarm !disarmb !disarmc4 !disarmp !dtm !forgive !gameover !help !kb !kban !kick !kickban !kill !ladder !m !medals !mods !next !nextmap !ntc !observe !pamsg !qkick !qspec !qspectate !rank !refund !rotation !rules !seen !setjoin !setnext !showmods !shun !spec !spectate !tban !tc !tc2 !tele !ts !unwarn !teleport !unban !viewjoin !vjoin !vset !warn !web !website ",cmd2) && (type == 0 || type == 1)) {
		vLogger->Log(vLoggerType::vRENLOG,"NULL","%s: %s",Player_Name_From_ID(ID).c_str(),Str->Get().c_str());
		return true;
	}
	return false;
}
std::string Execute_Autocomplete(const char *str) {
	char completed[512];
	getProfileString("Autocomplete",str,"NULL",completed,512,"Autocomplete.ini");
	std::string Retn = completed;
	return Retn;
}
bool __stdcall RadioHook(Stewie_CSAnnouncement *Event) {
	if (Event->AnnouncementId >= 8535 && Event->AnnouncementId < 8565 && Event->IconId >= 0 && Event->IconId < 30) {
		vPlayer *p = vPManager->Get_Player(Event->PlayerId);
		if (!p) { return false; }
		if (p->muted) { return false; }
		if (p->serial == false && Config->BlockNoSerialActions) { return false; }

		int GameTime = vManager.DurationAsInt();
		if (p->RSpamExpire > GameTime) { return false; }
		if (p->FirstRSpam <= GameTime - 5) { p->RadioMsgs = 0; }
		if (p->RadioMsgs == 0) { p->FirstRSpam = GameTime; }
		p->RadioMsgs++;
		if (p->RadioMsgs >= 5) {
			p->RadioMsgs = 0;
			p->RSpamExpire = GameTime + 5;
		}

		int PID = Event->PlayerId;
		vLogger->Log(vLoggerType::vRENLOG,"_NULL","[Radio] %s: %s",Player_Name_From_ID(PID).c_str(),Get_Translated_Name(Event->AnnouncementId).c_str());
		return true;
	}
	return false;
}

int Check_Purchase_Event(Stewie_SoldierGameObj *obj, int vtype, int pressed, int alt) {
	if (Config->BlockAllPurchases) { return 3; }

	GameObject *Purchaser = (GameObject *)obj;
	if (!Purchaser) { return 4; }
	if (!stricmp(Get_Model(Purchaser),"NULL")) { return 4; }

	int team = Commands->Get_Player_Type(Purchaser);
	if (vtype == 1) {
		if (Config->GameMode == Config->vSNIPER || Config->GameMode == Config->vINFONLY) { return 3; }
		int count = ((Stewie_VehicleFactoryGameObj *)(Stewie_BaseControllerClass::Find_Base(team)->Find_Building(vVehicleFactory)))->Get_Team_Vehicle_Count();
		if (team == 1) {
			if (count >= Config->GDIVehicleLimit) { return 3; }
		} else if (team == 0) {
			if (count >= Config->NodVehicleLimit) { return 3; }
		}
	}

	int ID = Get_Player_ID(Purchaser);
	Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(ID);
	if (!player) { return 3; }
	std::string pname = Player_Name_From_ID(ID);
	float d = FLT_MAX;
	Vector3 p = Commands->Get_Position(Purchaser);
	if (team == 1) { d = Find_Distance_To_Closest_Object_By_Preset(p,"pct_zone_gdi"); }
	else if (team == 0) { d = Find_Distance_To_Closest_Object_By_Preset(p,"pct_zone_nod"); }
	else { return 3; }

	if (d < FLT_MAX) {
		if (d > Config->MaximumPurchaseDistance) {
			if (Config->OutputPurchases) {
				vLogger->Log(vLoggerType::vVGM,"_CHEAT","Invalid Purchase Distance blocked from %s. [Distance: %.2f; Max: %d; Purchased: %s; Ping: %d]",
					pname.c_str(),
					d,Config->MaximumPurchaseDistance,
					Get_Purchase_Type(vtype).c_str(),
					player->Get_Ping()
				);
			}
			if (Config->BlockPurchases) {
				if (d > (Config->MaximumPurchaseDistance * 1.5f)) {
					Kill_Player(Purchaser);
					Inc_Kills(Stewie_cPlayerManager::Find_Player(ID),-1);
				}
				return 3;
			}
		}
	}
	vPlayer *vplayer = vPManager->Get_Player(pname.c_str());
	if (vplayer && !vplayer->serial && Config->BlockNoSerialActions) { return 3; }
	return -1;
}
int __cdecl OnPurchaseHook(Stewie_SoldierGameObj *obj, int vtype, int pressed, int alt, bool unk) {
	__asm { push ecx }
	GameObject *o = (GameObject *)obj;
	int ID = Get_Player_ID(o);

	int CanPurchase = Check_Purchase_Event(obj,vtype,pressed,alt);
	if (CanPurchase == -1) {
		if (vCManager->Carrier == ID) {
			if (!vCManager->CanRefill && vtype == (int)vPREFILL) {
				PrivMsgColoredVA(vCManager->Carrier,2,0,255,64,"[VGM] You cannot refill while carrying the Tiberium Crystal.");
				CanPurchase = 3;
			} else if (vtype == (int)vPBASICCHAR || vtype == (int)vPADVCHAR) {
				PrivMsgColoredVA(vCManager->Carrier,2,0,255,64,"[VGM] You cannot buy characters while carrying the Tiberium Crystal.");
				CanPurchase = 3;
			}
		}
	}
	if (CanPurchase == -1) {
		if (vtype >= 0 && vtype <= 6) {
			int team = Commands->Get_Player_Type(o);
			std::string purchased = Get_Purchase_Unit(vtype,pressed,alt,team);
			if (stricmp(purchased.c_str(),"None")) {
				vLogger->Log(vLoggerType::vVGM,"_PURCHASE","%s purchased a %s",Player_Name_From_GameObj(o).c_str(),purchased.c_str());
			}
		}
		if (vtype == (int)vPVEHICLE) {
			if (Config->Sounds) {
				if (Commands->Get_Player_Type(o) == 1) { Create_2D_WAV_Sound_Player(o,"mxxdsgn_dsgn0048i1evan_snd.wav"); }
				else if (Commands->Get_Player_Type(o) == 0) { Create_2D_WAV_Sound_Player(o,"mxxdsgn_dsgn0050i1evag_snd.wav"); }
			}
		}
		__asm {
			pop ecx
			push unk
			push alt
			push pressed
			push vtype
			push obj
			mov eax,0x000000 // For security and licensing purposes, an address has been hidden from this line
			call eax
			add esp,0x14
			push ecx
		}
	}
	__asm { pop ecx }
	return CanPurchase > 0 ? CanPurchase : 0;
}
int __cdecl OnPurchaseCharacterHook(Stewie_BaseControllerClass *base, Stewie_SoldierGameObj *obj, int Price, int definition, bool Unk) {
	GameObject *o = (GameObject *)obj;
	if (!o || Commands->Get_ID(o) <= 0) { return 3; }
	if (Price > 0) {
		if (!Find_Soldier_Factory(base->Team) || Commands->Get_Health(Find_Soldier_Factory(base->Team)) <= 0.0f) { return 3; }
	}

	int ID = Get_Player_ID(o);

	DefinitionClass *def = Find_Definition(definition);
	if (definition <= 0 || !def) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The character you attempted to purchase has no definition map and therefore cannot be purchased.");
		return 4;
	}

	float Cost = float(Price);
	if (Commands->Get_Money(o) < Cost && Cost > 0) { return 2; }
	Set_Money(ID,(float)(Commands->Get_Money(o) - Cost));

	Change_Character(o,def->Get_Name());

	return -1;
}

bool __stdcall VehicleTransitionHook(GameObject *s, Stewie_TransitionInstanceClass *Transition) {
	if (Config->GameMode == Config->vSNIPER || Config->GameMode == Config->vINFONLY) { return false; }

	bool entered = Get_Vehicle(s) ? false : true;
	if (!Transition || !Transition->Target.Reference || !Transition->Target.Reference->obj) { return false; }
	GameObject *v = (GameObject *)(Transition->Target.Reference->obj);
	if (Is_Script_Attached(v,"Locked")) { return false; }

	vPlayer *p = vPManager->Get_Player(Player_Name_From_GameObj(s).c_str());
	Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(Get_Player_ID(s));
	if (p) { p->CheatPending = ((int)vManager.DurationAsInt() + (int)(float(player->Get_Ping() / 1000) + 1.0f) + 2); }

	Stewie_VehicleGameObjDef *vdef = Get_Vehicle_Definition(v);
	if (p && vdef) {
		p->LastVehType = vdef->VehicleType;
	}

	if (entered) {
		if (!v) { return false; }
		if (vObjects::Is_Drivable_Vehicle((Stewie_BaseGameObj *)v) == false) { return false; }
		if (Get_Definition(v)->Get_Class_ID() != VehicleCID) { return false; }

		int ID = Get_Player_ID(s);

		if (ID == vCManager->Carrier && vCManager->CanEnterVehicle == false) {
			PrivMsgColoredVA(vCManager->Carrier,2,0,255,64,"[VGM] You cannot enter vehicles while carrying the Tiberium Crystal, so you have dropped it on the ground.");
			vCManager->Drop();
		}

		vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(v));
		if (EMPon) {
			return false;
		} else if (Is_Script_Attached(v,"AntiEnemy")) {
			if (Get_Script_Int_Parameter(v,"AntiEnemy","team") != Commands->Get_Player_Type(s)) {
				Kill_Player(s);
				return false;
			}
		} else {
			if (veh && veh->Owner > 0) {
				std::string ownername = Player_Name_From_ID(veh->Owner);
				std::string soldiername = Player_Name_From_ID(ID);
				GameObject *owner = Player_GameObj(veh->Owner);
				if (Get_Player_ID(owner) != Get_Player_ID(s) && owner != s && stricmp(ownername.c_str(),soldiername.c_str())) {
					if (Commands->Get_Player_Type(s) == Commands->Get_Player_Type(owner)) {
						if (!Get_Vehicle_Driver(v)) { 
							if (veh->Locked == 2) {
								PrivMsgColoredVA(ID,2,0,200,0,"[VGM] This vehicle (%s) is locked and bound to %s.",Get_Pretty_Name(v).c_str(),ownername.c_str());
								int t = Find_First_Available_Seat(v,false);
								if (t >= 0) { Add_Occupant_Seat(s,v,t); }
								return false;
							} else if (veh->Locked == 1 && owner && Get_Player_ID(owner) > 0) {
								PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You have entered %s's bound %s.",ownername.c_str(),Get_Pretty_Name(v).c_str());
								PrivMsgColoredVA(Get_Player_ID(owner),2,0,200,0,"[VGM] %s has entered your bound %s. To remove them, type !vkick %s (or just part of the name).",soldiername.c_str(),Get_Pretty_Name(v).c_str(),soldiername.c_str());
							}
						}
					} else if (veh->Locked > 0) {
						PrivMsgColoredVA(Get_Player_ID(owner),2,0,200,0,"[VGM] The enemy has stolen your bound vehicle (%s)!",Get_Pretty_Name(v).c_str());
						if (Config->Sounds) {
							Create_2D_WAV_Sound_Player(owner,"m00ffire_004in_gcm2_snd.wav");
							Create_2D_WAV_Sound_Player(s,"m00vct2_kill0032i1gctk_snd.wav");
						}
						veh->Lock(ID,0);
						if (Is_VTOLVehicle(v)) { vMManager->Got_Medal(ID,vMManager->vHIJACKED,true); }
						else { vMManager->Got_Medal(ID,vMManager->vCARJACKED,true); }
					}
				}
			}
		}

		if (veh && Stewie_cPlayerManager::Is_Player_Present(veh->Owner)) { vLogger->Log(vLoggerType::vVGM,"_VEHENTER","%s entered a(n) %s as %s. (Owner: %s; ID: %d)",Player_Name_From_GameObj(s).c_str(),Get_Pretty_Name(v).c_str(),(Get_Vehicle_Driver(v) ? "Passenger" : "Driver"),Player_Name_From_ID(veh->Owner).c_str(),Commands->Get_ID(v)); }
		else { vLogger->Log(vLoggerType::vVGM,"_VEHENTER","%s entered a(n) %s as %s. (ID: %d)",Player_Name_From_GameObj(s).c_str(),Get_Pretty_Name(v).c_str(),(Get_Vehicle_Driver(v) ? "Passenger" : "Driver"),Commands->Get_ID(v)); }
	} else {
		if (p && vdef && vdef->VehicleType == 3 && p->VetRank >= 2) {
			Attach_Script_Once(s,"Parachute","");
		}
	}
	return true;
}
void __stdcall ExitDeadVehicleHook(GameObject *s) {
	vPlayer *p = vPManager->Get_Player(Player_Name_From_GameObj(s).c_str());
	Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(Get_Player_ID(s));
	if (p) {
		p->CheatPending = ((int)vManager.DurationAsInt() + (int)(float(player->Get_Ping() / 1000) + 1.0f) + 2);
		if (p->LastVehType == 3 && p->VetRank >= 2) {
			Attach_Script_Once(s,"Parachute","");
		}
	}
}

void __stdcall HarvesterDockedHook(Stewie_RefineryGameObj *Owner, Stewie_HarvesterClass *Harvester) {
	if (!Harvester) { return; }
	if (Harvester->ID <= 0) { return; }
	GameObject *HarvesterVehicleObj = (GameObject *)Owner->Get_Harvester_Vehicle();
	vLogger->Log(vLoggerType::vVGM,"_HARVESTER","The %s is now docking",Get_Pretty_Name(HarvesterVehicleObj).c_str());
	if (Config->Sounds) {
		if (Commands->Get_Player_Type(HarvesterVehicleObj) == 1) { Create_2D_WAV_Sound_Team("m00evag_dsgn0047i1evag_snd.wav",1); }
		else if (Commands->Get_Player_Type(HarvesterVehicleObj) == 0) { Create_2D_WAV_Sound_Team("m00evan_dsgn0042i1evan_snd.wav",0); }
	}
}

void vPlayerObjectHandler::Created(GameObject *obj) {
	Remove_Duplicate_Script(obj,"M00_GrantPowerup_Created");
	WasKilled = false;
	if (!stricmp(Get_Parameter("WeaponDef"),"POW_Pistol_Player")) {
		const char *Preset = Get_Preset_Name(obj);
		if (!stricmp(Preset,"CnC_GDI_Engineer_0") || !stricmp(Preset,"CnC_Nod_Engineer_0")) {
			for (unsigned int i = 0; i < Config->EngiSpawnWeapons.size(); i++) {
				Commands->Give_Powerup(obj,Config->EngiSpawnWeapons[i].c_str(),true);
			}
		} else if (stricmp(Preset,"CnC_GDI_Engineer_2SF") == 0 || stricmp(Preset,"CnC_Nod_Technician_0") == 0) {
			for (unsigned int i = 0; i < Config->AdvEngiSpawnWeapons.size(); i++) {
				Commands->Give_Powerup(obj,Config->AdvEngiSpawnWeapons[i].c_str(),true);
			}
		} else if (Config->GameMode != Config->vSNIPER) {
			for (unsigned int i = 0; i < Config->OtherSpawnWeapons.size(); i++) {
				Commands->Give_Powerup(obj,Config->OtherSpawnWeapons[i].c_str(),true);
			}
		}

		if (Config->GameMode == Config->vSNIPER) {
			if (stricmp(Preset,"CnC_GDI_MiniGunner_2SF") && stricmp(Preset,"CnC_Nod_MiniGunner_2SF")) {
				if (Commands->Get_Player_Type(obj) == 1) { Change_Character(obj,"CnC_GDI_MiniGunner_2SF"); }
				else { Change_Character(obj,"CnC_Nod_Minigunner_2SF"); }
			}
		}

		Commands->Attach_Script(obj,"vChatKey","C4Count,!c4,1");
		Commands->Attach_Script(obj,"vChatKey","VetInfo,!vet,0");
		Commands->Attach_Script(obj,"vChatKey","VehBind,!bind,1");
		Commands->Attach_Script(obj,"vChatKey","VehBL,!bl,1");
		Commands->Attach_Script(obj,"vChatKey","VoteYes,!vote yes,0");
		Commands->Attach_Script(obj,"vChatKey","VoteNo,!vote no,0");
		Commands->Attach_Script(obj,"vRadioKey","Radio1,1");
		Commands->Attach_Script(obj,"vRadioKey","Radio2,2");
		Commands->Attach_Script(obj,"vRadioKey","Radio3,3");
		Commands->Attach_Script(obj,"vRadioKey","Radio4,4");
		Commands->Attach_Script(obj,"vTauntKey","Taunt1,h_a_a0a0_l12");
		Commands->Attach_Script(obj,"vTauntKey","Taunt2,H_A_a0a0_L22");
		Commands->Attach_Script(obj,"vTauntKey","Taunt3,H_A_a0a0_L23");
		Commands->Attach_Script(obj,"vTauntKey","Taunt4,H_A_a0a0_L24");
		Commands->Attach_Script(obj,"vTauntKey","Taunt5,H_A_a0a0_L25");
		Commands->Attach_Script(obj,"vTauntKey","Taunt6,H_A_a0a0_L58");
		Commands->Attach_Script(obj,"vTauntKey","Taunt7,H_A_cresentkick");
		Commands->Attach_Script(obj,"vTauntKey","Taunt8,H_A_sidekick");
		Commands->Attach_Script(obj,"vTauntKey","Taunt9,H_A_punchcombo");

		SpawnPos = Commands->Get_Position(obj);
		if (Config->GameMode != Config->vSNIPER && !stricmp(GameDataObj()->MapName,"C&C_Field.mix") && Commands->Get_Player_Type(obj) == 0) {
			#pragma warning (disable: 4305)
			Vector3 BadSpawn; BadSpawn.X = -93.085350; BadSpawn.Y = 58.289268; BadSpawn.Z = 0.118000;
			if (Commands->Get_Distance(SpawnPos,BadSpawn) < 2) {
				Vector3 GoodSpawn; GoodSpawn.X = -88.991447; GoodSpawn.Y = 51.767906; GoodSpawn.Z = 0.118000;
				Commands->Set_Position(obj,GoodSpawn);
				SpawnPos = Commands->Get_Position(obj);
			}
		}

		Commands->Set_Health(obj,Commands->Get_Max_Health(obj));
		Commands->Set_Shield_Strength(obj,Commands->Get_Max_Shield_Strength(obj));

		vPlayer *p = vPManager->Get_Player(Player_Name_From_GameObj(obj).c_str());
		if (p) {
			Stewie_WeaponBagClass *bag = Get_Weapon_Bag((Stewie_ArmedGameObj *)obj);
			if (bag) {
				for (unsigned int i = 0; i < p->WeaponBag.size(); i++) {
					Stewie_WeaponDefinitionClass *Definition = p->WeaponBag[i];
					if (isin(Definition->Get_Name(),"ramjet") && isin(Get_Pretty_Name(obj).c_str(),"stealth")) {
						Definition = Stewie_WeaponManager::Find_Weapon_Definition("Weapon_RamjetRifle_Player");
					}
					int bullets = Definition->DSClipSize.Get() + Definition->DSMaxClipBullets.Get();
					if (Definition->DSMaxClipBullets.Get() == -1) { bullets = -1; }
					bag->Add_Weapon(Definition,bullets,true);
				}
			}
			if (Config->VetEnabled) {
				vVetManager->Upgrade(obj,p->VetRank - 1,1);
				float Refund = float(p->VetRank) * (100.0f / float(Config->VetLevels)) - 20.0f;
				if (Refund > 0.0f && Get_Cost(Get_Preset_Name(obj)) > 0) {
					Commands->Give_Money(obj,(Refund / 100.0f) * Get_Cost(Get_Preset_Name(obj)),false);
					PrivMsgColoredVA(Get_Player_ID(obj),2,0,200,0,"[VGM] You have been refunded %.0f%% (%.0f credits) for your %s.",Refund,(Refund / 100.0f) * Get_Cost(Get_Preset_Name(obj)),Get_Pretty_Name(obj).c_str());
				}
			}
		}
		
		Commands->Give_Powerup(obj,"CnC_POW_Ammo_ClipMax",true);

		if (Config->EnableAntiBighead) {
			Stewie_ScriptableGameObj *soldier = (Stewie_ScriptableGameObj *)As_ScriptableGameObj(obj);
			if (Has_Observer(*soldier,"vObserver0xA1") == false) {
				(*soldier).Add_Observer(new vBigheadObserver);
			}
		}

		Set_Is_Visible((Stewie_SoldierGameObj *)obj,true);
	}
}
void vPlayerObjectHandler::Damaged(GameObject *obj, GameObject *damager, float damage) {
	vVetManager->Add_Pending_VetPoints((Stewie_ScriptableGameObj *)damager,Commands->Get_ID(obj),damage);
	if (damage < 0.0f) {
		vPlayer *p = vPManager->Get_Player(Get_Player_ID(damager));
		if (p) { p->TotalValueRepaired -= damage; }
	}
}
void vPlayerObjectHandler::Killed(GameObject *obj, GameObject *shooter) {
	Stewie_SoldierGameObj *s = (Stewie_SoldierGameObj *)obj;
	s->HumanState.Type = Stewie_HumanStateClass::DEATH;

	vVetManager->Object_Destroyed(obj);

	int ID = Get_Player_ID(obj);
	DeathPlace = Commands->Get_Position(obj);

	if (ID == vCManager->Carrier) { vCManager->Drop(); }

	if (ID > 0 && s->Is_Human_Controlled()) {
		WasKilled = true;

		int SID = Get_Player_ID(shooter);

		if (Is_Script_Attached(shooter,"EnemyThief") && obj != shooter) {
			if (Config->GameMode != Config->GAMEMODE::vSNIPER) {
				if (Is_Soldier(shooter) && Get_Player_ID(shooter) > 0 && Get_Player_ID(obj) > 0) {
					PrivMsgColoredVA(SID,2,200,0,200,"[VGM] You have stolen half of %s's money!",Get_Player_Name(obj));
					PrivMsgColoredVA(ID,2,200,0,200,"[VGM] You have lost half of your money to %s, who had the Enemy Thief Crate.",Get_Player_Name(shooter));
					int amt = abs((int)ceil(Commands->Get_Money(obj) / 2.0f));
					Commands->Give_Money(shooter,float(amt),false);
					Commands->Give_Money(obj,-1 * float(amt),false);
				}
			}
			Remove_Script(shooter,"EnemyThief");
		}

		if (shooter && Get_Player_ID(shooter) > 0) {
			float Bounty = vPManager->Get_Bounty(-1,ID);
			if (Bounty > 0.0f) {
				if (Get_Player_ID(obj) != Get_Player_ID(shooter)) {
					Commands->Give_Money(shooter,Bounty,false);
					Create_2D_WAV_Sound("m00avis_kifi0003i1moac_snd.wav");
					HostMessage("[VGM] The bounty of $%.0f on %s's head has been claimed!",Bounty,Player_Name_From_ID(ID).c_str());
					vPManager->Remove_Bounty(-1,ID);
				} else {
					HostMessage("[VGM] The bounty of $%.0f on %s's head has been refunded!",Bounty,Player_Name_From_ID(ID).c_str());
					vPManager->Refund_Bounty(ID);
				}
			}
		}

		std::string VicName = Player_Name_From_GameObj(obj);
		vPlayer *player = vPManager->Get_Player(VicName.c_str());
		if (!player) { return; }
		std::string KillerName = Player_Name_From_GameObj(shooter);
		vPlayer *killer = vPManager->Get_Player(KillerName.c_str());
		if (FirstKillOfMap && killer && player->ID != killer->ID && Commands->Get_Player_Type(obj) != Commands->Get_Player_Type(shooter) && killer->ID > 0 && Stewie_cPlayerManager::Is_Player_Present(killer->ID)) {
			FirstKillOfMap = false;
			if (Config->FirstBloodCreds > 0.0f) {
				Commands->Give_Money(shooter,Config->FirstBloodCreds,false);
				HostMessage("[VGM] %s receives %.0f credits for first blood.",KillerName.c_str(),Config->FirstBloodCreds);
				if (Config->Sounds) { Create_2D_WAV_Sound_Player(shooter,"m00pc$$_aqob0002i1evag_snd.wav"); }
			}
		}

		player->Reset_KillSprees(); // MEDALS

		if (Config->Sounds && killer) {
			if (killer->KillsWithoutDying == 3)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"killing_spree.wav");
			else if (killer->KillsWithoutDying == 6)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"dominating.wav");
			else if (killer->KillsWithoutDying == 9)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"unstoppable.wav");
			else if (killer->KillsWithoutDying == 12)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"godlike.wav");
			else if (killer->KillsWithoutDying == 15)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"WhickedSick.wav");
			else if (killer->KillsWithoutDying == 18)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"HolyShit_F.wav");
			else if (killer->KillsWithoutDying == 20)
				Create_2D_WAV_Sound_Player(Player_GameObj(killer->ID),"perfect.wav");
		}

		// MEDALS
		Stewie_cGameData *data = GameDataObj();
		const char *Preset = Get_Preset_Name(shooter);
		const char *Weapon = Get_Current_Weapon(shooter);
		if (SID > 0 && Preset && Weapon) {
			if (obj != shooter && Commands->Get_Player_Type(obj) == Commands->Get_Player_Type(shooter)) { vMManager->Got_Medal(SID,vMManager->vTEAMKILL,true); }
			if (isin(Preset,"Visceroid")) { vMManager->Got_Medal(SID,vMManager->vTIBERIUMKILL,true); }
			if (isin(Weapon,"Ramjet") && player->LastDamage == 1000.0f) { killer->HSWithoutDying1000++; }
			else if (isin(Weapon,"Sniper") && player->LastDamage == 500.0f) { killer->HSWithoutDying500++; }
			else if (isin(Weapon,"ChemSprayer") || isin(Weapon,"Flamethrower")) { vMManager->Got_Medal(SID,vMManager->vINCINERATION,true); }
		}
		if (player->KillsWithoutDying >= 5) { vMManager->Got_Medal(SID,vMManager->vKILLJOY,true); }
		if (player->KillsWithoutDying >= 40) { vMManager->Got_Medal(ID,vMManager->vUNTOUCHABLE,true); }
		else if (player->KillsWithoutDying >= 25) { vMManager->Got_Medal(ID,vMManager->vRUNNINGRIOT,true); }
		else if (player->KillsWithoutDying >= 15) { vMManager->Got_Medal(ID,vMManager->vKILLFRENZY,true); }
		else if (player->KillsWithoutDying >= 5) { vMManager->Got_Medal(ID,vMManager->vKILLSPREE,true); }
		if (killer) {
			killer->KillsWithoutDying++;
			if (obj != shooter && (vManager.DurationAsInt() - killer->KillSpreeStart) <= vMManager->KillSpreeTime) {
				killer->KillsSinceSpreeStart++;
			}
			if (killer->KillsSinceSpreeStart == 10) { vMManager->Got_Medal(SID,vMManager->vKILLIONAIRE,true); }
			else if (killer->KillsSinceSpreeStart == 9) { vMManager->Got_Medal(SID,vMManager->vKILLPOCALYPSE,true); }
			else if (killer->KillsSinceSpreeStart == 8) { vMManager->Got_Medal(SID,vMManager->vKILLTASTROPHE,true); }
			else if (killer->KillsSinceSpreeStart == 7) { vMManager->Got_Medal(SID,vMManager->vKILLAMANJARO,true); }
			else if (killer->KillsSinceSpreeStart == 6) { vMManager->Got_Medal(SID,vMManager->vKILLTROCITY,true); }
			else if (killer->KillsSinceSpreeStart == 5) { vMManager->Got_Medal(SID,vMManager->vKILLTACULAR,true); }
			else if (killer->KillsSinceSpreeStart == 4) { vMManager->Got_Medal(SID,vMManager->vOVERKILL,true); }
			else if (killer->KillsSinceSpreeStart == 3) { vMManager->Got_Medal(SID,vMManager->vTRIPLEKILL,true); }
			else if (killer->KillsSinceSpreeStart == 2) { vMManager->Got_Medal(SID,vMManager->vDOUBLEKILL,true); }
			if ((vManager.DurationAsInt() - killer->KillSpreeStart) > vMManager->KillSpreeTime) {
				killer->KillsSinceSpreeStart = 0;
			}
			killer->KillSpreeStart = vManager.DurationAsInt();
		}

		if (Is_Script_Attached(obj,"Kamikaze")) {
			if (Get_Script_Int_Parameter(obj,"Kamikaze","type") == 1) {
				Commands->Create_Object("Beacon_Nuke_Strike_Anim_Post",Commands->Get_Position(obj));
				Commands->Create_Explosion("Explosion_NukeBeacon",Commands->Get_Position(obj),0);
			} else {
				Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",Commands->Get_Position(obj));
				Commands->Create_Explosion("Explosion_IonCannonBeacon",Commands->Get_Position(obj),0);
			}
			Damage_All_Soldiers_Area_Team(2500.0f,"None",Commands->Get_Position(obj),15.0f,obj,0,2);
			Damage_All_Soldiers_Area_Team(2500.0f,"None",Commands->Get_Position(obj),15.0f,obj,0,2);
			HostMessage("KAMIKAZE!");
		}
	}

	vPManager->Clear_Hits_By_Target_ID(Commands->Get_ID(obj));
}
void vPlayerObjectHandler::Destroyed(GameObject *obj) {
	if (Config->GameMode == Config->vSNIPER) { return; }

	int ID = Get_Player_ID(obj);
	if (ID == vCManager->Carrier) { vCManager->Drop(); }

	if (WasKilled) {
		Vector3 pos = Commands->Get_Position(obj);

		int Wep = Random(1,4);
		int Rand = Random(0,20,true);
		if (Wep <= 3) {
			vWManager->Create_Pack(ID,true);
		} else {
			GameObject *AddPowerupScript = NULL;
			if (Rand >= 0 && Rand < 4) { AddPowerupScript = Commands->Create_Object("POW_Tiberium_Shield",pos); } // Tib Suit
			else if (Rand >= 4 && Rand < 8) { AddPowerupScript = Commands->Create_Object("POW_Head_Band",pos); } // Logan Headband
			else if (Rand >= 8 && Rand < 10) { AddPowerupScript = Commands->Create_Object("POW_Uplink",pos); Commands->Set_Model(AddPowerupScript,"vehcol2m"); Attach_Script_Once(AddPowerupScript,"vCrateHandler",""); } // Crate
			else if (Rand >= 10 && Rand < 13) { AddPowerupScript = Commands->Create_Object("POW_Medal_Health",pos); } // Health
			else if (Rand >= 13 && Rand < 16) { AddPowerupScript = Commands->Create_Object("POW_Medal_Armor",pos); } // Armor
			else if (Rand >= 16 && Rand < 18) { AddPowerupScript = Commands->Create_Object("POW_Ammo_Regeneration",pos); } // Ammo
			else if (Rand >= 18 && Rand < 20) { AddPowerupScript = Commands->Create_Object("POW_Stealth_Suit",pos); Commands->Set_Model(AddPowerupScript,"P_BACKPACK"); } // Full Pack
			if (AddPowerupScript) {
				Set_Powerup_Always_Allow_Grant(AddPowerupScript,true);
				Attach_Script_Once(AddPowerupScript,"ExpirePowerup","30");
			}
		}
		WasKilled = false;
	}

	vPlayer *player = vPManager->Get_Player(ID);
	if (player) {
		player->WeaponBag.clear();
		player->Reset_KillSprees(); // MEDALS
		player->Clear_Hits();
	}
}
void vPlayerObjectHandler::Timer_Expired(GameObject *obj, int number) {
	if (Config->GameMode == Config->vSNIPER) { return; }
	if (number == 16384) {
		if (EMPon) {
			Commands->Create_Object("Spawner Created Special Effect",Commands->Get_Position(obj));
		}
		Commands->Start_Timer(obj,this,2.5f,number);
	}
}

void vVehicleObjectHandler::Created(Stewie_ScriptableGameObj *obj) {
	if (!obj) {
		Script_Remove(obj,"M00_Vehicle_Log");
		return;
	}
	IconID = 0;
	DamageExpire = 0;
	const char *preset = obj->Definition->Get_Name();
	if (isin(preset,"Nod_Turret") || isin(preset,"GDI_Guard_Tower")) {
		Script_Remove(obj,"M00_Vehicle_Log");
		Script_Attach(obj,"M00_Disable_Transition","",true);
		return;
	}
	if (Config->GameMode == Config->vSNIPER || (!vObjects::Is_Drivable_Vehicle(obj) && !vObjects::Is_Harvester(obj)) || isin(preset,"_Site")) {
		Script_Remove(obj,"M00_Vehicle_Log");
		return;
	}
	HarvesterShell = false;
	vVehicle *v = vVManager->Add_Vehicle(obj);
	if (v) {
		vLogger->Log(vLoggerType::vVGM,"_VEHPURCHASE","%s %s created (%d)",Get_Translated_Team_Name(obj->As_SmartGameObj()->PlayerType).c_str(),Get_Pretty_Name(preset).c_str(),obj->NetworkID);
	}
}
void vVehicleObjectHandler::Damaged(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *damager, float damage) {
	vVetManager->Add_Pending_VetPoints(damager,obj->NetworkID,damage);
	if (damage > 0.0f) {
		const char *preset = obj->Definition->Get_Name();
		if ((Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY) && DamageExpire < vManager.DurationAsUint()) {
			if (isin(preset,"Harv") && !isin(preset,"Destroyed")) {
				if (isin(preset,"GDI_")) {
					Create_2D_Sound_Team("M00VGHV_TFEA0002I1EVAN_SND",0);
					Create_2D_Sound_Team("M00VGHV_TFEA0001I1EVAG_SND",1);
				} else if (isin(preset,"Nod_")) {
					Create_2D_Sound_Team("M00VNHV_TFEA0001I1EVAG_SND",1);
					Create_2D_Sound_Team("M00VNHV_TFEA0002I1EVAN_SND",0);
				}
				vLogger->Log(vLoggerType::vVGM,"_HARVESTER","The %s is under attack",Get_Pretty_Name(preset).c_str());
				DamageExpire = (vManager.DurationAsInt() + 30);
			}
		}
	} else if (damage < 0.0f) {
		vPlayer *p = vPManager->Get_Player(Player_ID_From_GameObj(damager));
		if (p) { p->TotalValueRepaired -= damage; }
	}
}
void vVehicleObjectHandler::Killed(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *shooter) {
	vVetManager->Object_Destroyed(obj->NetworkID);

	int ID = Player_ID_From_GameObj(obj);
	Stewie_SoldierGameObj *VDriver = Vehicle_Driver(obj->As_VehicleGameObj());
	int DriverID = Player_ID_From_GameObj(VDriver);
	int ShooterID = Player_ID_From_GameObj(shooter);

	DeathFacing = obj->As_PhysicalGameObj()->Get_Facing();
	obj->Get_Position(&DeathPlace);
	WasKilled = true;

	vPlayer *dP = vPManager->Get_Player(DriverID);
	if (dP) { dP->SquishesWithoutDying = 0; }

	std::string KillerName = Player_Name_From_GameObj(shooter);
	vVehicle *veh = vVManager->Get_Vehicle(obj->NetworkID);
	char Owner[256],Driver[256];
	if (veh && Stewie_cPlayerManager::Is_Player_Present(veh->Owner)) { sprintf(Owner,"; Owner: %s",Player_Name_From_ID(veh->Owner).c_str()); } else { sprintf(Owner,""); }
	if (DriverID > 0) { sprintf(Driver,"; Driver: %s",dP->name.c_str()); } else { sprintf(Driver,""); }
	char creds[128];
	sprintf(creds,"(ID: %d%s%s)",obj->NetworkID,strlen(Owner) > 1 ? (const char*)Owner : "",strlen(Driver) > 1 ? (const char*)Driver : "");

	const char *preset = obj->Definition->Get_Name();
	if (veh && veh->WasFlipKilled) {
		vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s was destroyed by flip-kill. - %s",Get_Pretty_Name(preset).c_str(),creds);
	} else if (!shooter || shooter->NetworkID <= 0) {
		vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s was destroyed. - %s",Get_Pretty_Name(preset).c_str(),creds);
	} else if (ShooterID <= 0) {
		vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to the %s. - %s",Get_Pretty_Name(preset).c_str(),Get_Pretty_Name(shooter->Definition->Get_Name()).c_str(),creds);
		HarvesterShell = true;
	} else {
		vPlayer *killer = vPManager->Get_Player(KillerName.c_str());
		if (killer) {
			int Hits = 0;
			const char *Weapon = Get_Held_Weapon(shooter->As_SoldierGameObj())->Definition->Get_Name();
			std::string WeaponName = Get_Pretty_Name(Weapon);
			if (shooter->As_SoldierGameObj()->VehicleOccupied) {
				Stewie_DamageableGameObjDef *Def = (Stewie_DamageableGameObjDef *)(Stewie_DefinitionMgrClass::Find_Definition(shooter->As_SoldierGameObj()->VehicleOccupied->Definition->Get_Name()));
				Hits = killer->Get_Hits(obj->NetworkID,1.0f,Def->NameID);
			} else {
				Stewie_WeaponClass *Weapon = Get_Held_Weapon((Stewie_SoldierGameObj *)shooter);
				if (Weapon) {
					Hits = killer->Get_Hits(obj->NetworkID,1.0f,(Weapon->SecondaryTriggered ? Weapon->Definition->SecondaryAmmoDefId : Weapon->Definition->PrimaryAmmoDefId));
				}
			}
			if (Hits > 0) { vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to %s (%s). - %s - LF: %s; LD: %0.2f; Hits: %d.",Get_Pretty_Name(preset).c_str(),KillerName.c_str(),Get_Preset_Info((GameObject *)shooter).c_str(),creds,(killer->LastFiringMode == 1 ? "P" : "S"),veh ? veh->LastDamage : 0.0f,Hits); }
			else { vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to %s (%s). - %s - LF: %s; LD: %0.2f.",Get_Pretty_Name(preset).c_str(),KillerName.c_str(),Get_Preset_Info((GameObject *)shooter).c_str(),creds,(killer->LastFiringMode == 1 ? "P" : "S"),veh ? veh->LastDamage : 0.0f); }
			HarvesterShell = true;
		} else {
			vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to %s (%s). - %s.",Get_Pretty_Name(preset).c_str(),KillerName.c_str(),Get_Preset_Info((GameObject *)shooter).c_str(),creds);
			HarvesterShell = true;
		}
	}

	vVManager->Delete_Vehicle(obj->NetworkID);
	vPManager->Clear_Hits_By_Target_ID(obj->NetworkID);
}
void vVehicleObjectHandler::Destroyed(Stewie_ScriptableGameObj *obj) {
	bool IsHarvester = vObjects::Is_Harvester(obj);
	Stewie_PhysicalGameObj *Icon1 = Stewie_GameObjManager::Find_PhysicalGameObj(IconID), *Icon2 = Stewie_GameObjManager::Find_PhysicalGameObj(IconID2);
	if (Icon1) { Icon1->Set_Delete_Pending(); }
	if (Icon2) { Icon2->Set_Delete_Pending(); }
	Icon1 = 0, Icon2 = 0;
	if (WasKilled) {
		Stewie_BaseGameObj *Shell = NULL;
		const char *preset = obj->Definition->Get_Name();
		if (Config->EnableShells) {
			if (!stricmp(preset,"CnC_GDI_Humm-vee")) { Shell = Create_Object("GDI_Humm-vee_destroyed"); }
			else if (!stricmp(preset,"CnC_GDI_APC")) { Shell = Create_Object("GDI_APC_Destroyed"); }
			else if (!stricmp(preset,"CnC_GDI_Medium_Tank")) { Shell = Create_Object("GDI_Medium_Tank_Destroyed"); }
			else if (!stricmp(preset,"CnC_Nod_Buggy")) { Shell = Create_Object("Nod_Buggy_Destroyed"); }
			else if (!stricmp(preset,"CnC_Nod_Light_Tank")) { Shell = Create_Object("Nod_Light_Tank_Destroyed"); }
		}
		if (Config->EnableHShells && HarvesterShell && IsHarvester) {
			if (obj->As_SmartGameObj()->PlayerType == 1) {
				DeathPlace.Z -= 1;
				Shell = Create_Object("Nod_Harvester_Destroyed");
				Shell->As_SmartGameObj()->Set_Player_Type(1);
				Script_Attach(Shell->As_ScriptableGameObj(),"vHarvesterShellHandler");
			} else if (obj->As_SmartGameObj()->PlayerType == 0) {
				DeathPlace.Z -= 1;
				Shell = Create_Object("Nod_Harvester_Destroyed");
				Shell->As_SmartGameObj()->Set_Player_Type(0);
				Script_Attach(Shell->As_ScriptableGameObj(),"vHarvesterShellHandler");
			}
		}
		if (Shell) {
			Shell->As_PhysicalGameObj()->Set_Position(DeathPlace);
			Commands->Set_Facing((GameObject *)Shell,DeathFacing);
			if (!IsHarvester) { Script_Attach(Shell->As_ScriptableGameObj(),"M00_Rebuild_Shell",preset,true); }
			Script_Attach(Shell->As_ScriptableGameObj(),"M00_Vehicle_Log","",true);
		}
	}
	vVManager->Delete_Vehicle(obj->NetworkID);
}
void vVehicleObjectHandler::Custom(Stewie_ScriptableGameObj *obj, int message, int param, Stewie_ScriptableGameObj *sender) {
	if (message == 1000) {
		Stewie_PhysicalGameObj *Icon1 = Stewie_GameObjManager::Find_PhysicalGameObj(IconID), *Icon2 = Stewie_GameObjManager::Find_PhysicalGameObj(IconID2);
		if (Icon1) { Icon1->Set_Delete_Pending(); }
		if (Icon2) { Icon2->Set_Delete_Pending(); }
		Icon1 = 0, Icon2 = 0;
		if (obj && param > 0) {
			char bone1[128]; sprintf(bone1,"%s",Get_Icon_Bone(obj->Definition->Get_Name(),1).c_str());
			char bone2[128]; sprintf(bone2,"%s",Get_Icon_Bone(obj->Definition->Get_Name(),2).c_str());
			if (stricmp(bone1,"NULL")) {
				Stewie_PhysicalGameObj *Icon = Stewie_ObjectLibraryManager::Create_Object("Invisible_Object")->As_PhysicalGameObj();
				Icon->Phys->Set_Model_By_Name(param == 1 ? "p_keycrd_yel" : "p_keycrd_red");
				Icon->Attach_To_Object_Bone(obj->As_PhysicalGameObj(),bone1);
				IconID = Icon->NetworkID;
			}
			if (stricmp(bone2,"NULL")) {
				Stewie_PhysicalGameObj *Icon = Stewie_ObjectLibraryManager::Create_Object("Invisible_Object")->As_PhysicalGameObj();
				Icon->Phys->Set_Model_By_Name(param == 1 ? "p_keycrd_yel" : "p_keycrd_red");
				Icon->Attach_To_Object_Bone(obj->As_PhysicalGameObj(),bone2);
				IconID2 = Icon->NetworkID;
			}
		}
	}
}

void vVehicleShellHandler::Created(GameObject *obj) {
	Commands->Set_Player_Type(obj,-2);
	Commands->Set_Health(obj,50);
	Commands->Set_Shield_Strength(obj,0);
	Commands->Disable_Physical_Collisions(obj);
}
void vVehicleShellHandler::Damaged(GameObject *obj, GameObject *damager, float damage) {
	if (damage < 0) {
		float health = Get_Total_Health((Stewie_BaseGameObj *)obj);
		float maxhealth = Get_Total_Max_Health((Stewie_BaseGameObj *)obj);
		if (health >= maxhealth) {
			Vector3 pos = Commands->Get_Position(obj);
			pos.Z += 1;
			GameObject *tank = Commands->Create_Object(Get_Parameter("Preset"),pos);
			float facing = Commands->Get_Facing(obj);
			Commands->Set_Facing(tank,facing);
			Commands->Set_Health(tank,1.0f);
			Commands->Set_Shield_Strength(tank,0.0f);
			Commands->Set_Player_Type(tank,Commands->Get_Player_Type(damager));
			Commands->Destroy_Object(obj);
			vVehicle *veh = vVManager->Get_Vehicle(Commands->Get_ID(tank));
			int c = vVManager->Count_Player_Bound_Vehicles(Get_Player_ID(damager),false);
			if (veh) {
				if (c < Config->MaxFriendlyVeh) { veh->Lock(Get_Player_ID(damager),1); }
				else { veh->Lock(Get_Player_ID(damager),0); }
			}
		}
	}
}

void vHarvesterShellHandler::Created(GameObject *obj) {
	if (Config->GameMode == Config->vSNIPER) { Remove_Script(obj,"vHarvesterShellHandler"); }
	Remove_Duplicate_Script(obj,"vHarvesterShellHandler");
	Team = Get_Object_Type(obj);
	Commands->Set_Player_Type(obj,-2);
	Commands->Set_Health(obj,200);
	Commands->Set_Shield_Strength(obj,0);
	Commands->Set_Shield_Type(obj,"SkinVehicleMedium");
	Commands->Disable_Physical_Collisions(obj);
}
void vHarvesterShellHandler::Damaged(GameObject *obj, GameObject *damager, float damage) {
	if (damage == 0.0f) { return; }
	if (damage < 0) {
		float TotalHealth = Get_Total_Health((Stewie_BaseGameObj *)obj);
		const float MaxHealth = Get_Total_Max_Health((Stewie_BaseGameObj *)obj);
		if (TotalHealth == MaxHealth) {
			int money = Random(100,500);
			Give_Money_To_All_Players(money,Team);
			PrivMsgColoredVA(0,Team,0,200,0,"[VGM] The Harvester Shell has been repaired by %s and each member of your team has been awarded %d credits from the Tiberium inside.",Player_Name_From_GameObj(damager).c_str(),money);
			Commands->Destroy_Object(obj);
			if (Config->Sounds) { Create_2D_WAV_Sound_Team("m00pc$$_aqob0002i1evag_snd.wav",Team); }
		}
	}
}

void vTowerObjectHandler::Created(GameObject *obj) {
	Commands->Enable_Vehicle_Transitions(obj,false);
	if (isin(Get_Preset_Name(obj),"Harv") || Is_Harvester(obj)) {
		Remove_Script(obj,"M00_Disable_Transition");
		return;
	}
	Remove_Script(obj,"M00_Vehicle_Log");
	if (Config->GameMode == Config->vSNIPER || Config->GameMode == Config->vINFONLY) {
		Commands->Destroy_Object(obj);
	}
}
void vTowerObjectHandler::Damaged(GameObject *obj, GameObject *damager, float damage) {
	vVetManager->Add_Pending_VetPoints((Stewie_ScriptableGameObj *)damager,Commands->Get_ID(obj),damage);
	if (Config->GameMode == Config->vSNIPER) {
		Commands->Set_Health(obj,2000);
		Commands->Set_Shield_Strength(obj,2000);
		Commands->Give_Points(damager,(float)(-0.04f * damage),false);
	}
	if (damage < 0.0f) {
		vPlayer *p = vPManager->Get_Player(Get_Player_ID(damager));
		if (p) { p->TotalValueRepaired -= damage; }
	}
}
void vTowerObjectHandler::Killed(GameObject *obj, GameObject *shooter) {
	vVetManager->Object_Destroyed(obj);

	int ID = Get_Player_ID(obj);
	DeathFacing = Commands->Get_Facing(obj);
	DeathPlace = Commands->Get_Position(obj);
	WasKilled = true;
	std::string KillerName = Player_Name_From_GameObj(shooter);

	if (Config->Sounds && isin(Get_Preset_Name(obj),"turret")) {
		Commands->Create_2D_WAV_Sound("m00bntu_kill0001i1evan_snd.wav");
	}

	if (!shooter || Commands->Get_ID(shooter) <= 0) {
		vLogger->Log(vLoggerType::vVGM,"_VEHKILLED","%s was destroyed",Get_Pretty_Name(obj).c_str());
	} else if (Get_Player_ID(shooter) <= 0) {
		vLogger->Log(vLoggerType::vVGM,"_VEHKILLED","%s destroyed thanks to the %s",Get_Pretty_Name(obj).c_str(),Get_Pretty_Name(shooter).c_str());
	} else {
		vPlayer *killer = vPManager->Get_Player(KillerName.c_str());
		if (killer != NULL) {
			int Hits = 0;
			if (Get_Vehicle(shooter)) {
				Stewie_DamageableGameObjDef *Def = (Stewie_DamageableGameObjDef *)(Stewie_DefinitionMgrClass::Find_Definition(Get_Preset_Name(Get_Vehicle(shooter))));
				Hits = killer->Get_Hits(Commands->Get_ID(obj),1,Def->NameID);
			} else {
				Stewie_WeaponClass *Weapon = Get_Held_Weapon((Stewie_SoldierGameObj *)shooter);
				if (Weapon) {
					Hits = killer->Get_Hits(Commands->Get_ID(obj),1,(Weapon->SecondaryTriggered ? Weapon->Definition->SecondaryAmmoDefId : Weapon->Definition->PrimaryAmmoDefId));
				}
			}
			if (Hits > 0) { vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to %s (%s). - (ID: %d) - LF: %s; Hits: %d.",Get_Pretty_Name(obj).c_str(),KillerName.c_str(),Get_Preset_Info(shooter).c_str(),Commands->Get_ID(obj),(killer->LastFiringMode == 1 ? "P" : "S"),Hits); }
			else { vLogger->Log(vLoggerType::vVGM,"_VEHKILL","%s destroyed thanks to %s (%s). - (ID: %d) - LF: %s.",Get_Pretty_Name(obj).c_str(),KillerName.c_str(),Get_Preset_Info(shooter).c_str(),Commands->Get_ID(obj),(killer->LastFiringMode == 1 ? "P" : "S")); }
		} else {
			vLogger->Log(vLoggerType::vVGM,"_VEHKILLED","%s destroyed thanks to %s (%s)",Get_Pretty_Name(obj).c_str(),KillerName.c_str(),Get_Preset_Info(shooter).c_str());
		}
	}

	vPManager->Clear_Hits_By_Target_ID(Commands->Get_ID(obj));
}
void vTowerObjectHandler::Destroyed(GameObject *obj) {
	if (strstr(Get_Preset_Name(obj),"Nod_Turret_MP")) {
		GameObject *DestroyedTurret = Commands->Create_Object("Nod_Turret_Destroyed",Commands->Get_Position(obj));
		Commands->Set_Facing(DestroyedTurret,Commands->Get_Facing(obj));
	}
}

void vBuildingObjectHandler::Created(GameObject *obj) {
	Remove_Duplicate_Script(obj,"M00_BUILDING_EXPLODE_NO_DAMAGE_DAK");
	DamageExpire = 0;
	DamageExpire2 = 0;
	LastDamage = 0.0f;
	if (Config->GameMode == Config->vSNIPER) {
		if (Is_SoldierFactory(obj)) {
			Set_Max_Health(obj,2000);
			Set_Max_Shield_Strength(obj,2000);
			Commands->Set_Shield_Type(obj,"Blamo");
		} else {
			Commands->Set_Health(obj,0);
			Commands->Set_Shield_Strength(obj,0);
		}
	}
}
void vBuildingObjectHandler::Damaged(GameObject *obj, GameObject *damager, float damage) {
	if (damage > 0.0f) {
		if (Config->GameMode == Config->vSNIPER && Is_SoldierFactory(obj)) {
			Commands->Set_Health(obj,2000);
			Commands->Set_Shield_Strength(obj,2000);
			Commands->Give_Points(damager,(float)(-5.0f * damage),false);
			Commands->Give_Money(damager,(float)(-5.0f * damage),false);
		}
		if ((Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY) && DamageExpire2 < vManager.DurationAsUint()) {
			vLogger->Log(vLoggerType::vVGM,"_BUILDING2","The %s is under attack",Get_Pretty_Name(obj).c_str());
			DamageExpire2 = (vManager.DurationAsInt() + 30);
		}
	} else if (damage < 0.0f) {
		vPlayer *p = vPManager->Get_Player(Get_Player_ID(damager));
		if (p) { p->TotalValueRepaired -= damage; }
		if ((Config->GameMode == Config->vAOW || Config->GameMode == Config->vMONEY) && DamageExpire < vManager.DurationAsUint()) {

			float Health = Get_Total_Health((Stewie_BaseGameObj *)obj);
			float MaxHealth = Get_Total_Max_Health((Stewie_BaseGameObj *)obj);
			const char *Preset = Get_Preset_Name(obj);
			if (damage < 0.0f && Health >= MaxHealth && LastDamage < MaxHealth) {
				char *Announcement = "ERROR";
				if (isin(Preset,"mp_GDI_Advanced_Guard_Tower")) { Announcement = "M00BGAT_DSGN0007I1EVAG_SND"; }
				if (isin(Preset,"mp_GDI_Barracks")) { Announcement = "M00BGIB_DSGN0008I1EVAG_SND"; }
				if (isin(Preset,"mp_GDI_War_Factory")) { Announcement = "M00BGWF_DSGN0009I1EVAG_SND"; }
				if (isin(Preset,"mp_GDI_Power_Plant")) { Announcement = "M00BGPP_DSGN0009I1EVAG_SND"; }
				if (isin(Preset,"mp_GDI_Refinery")) { Announcement = "M00BGTR_DSGN0009I1EVAG_SND"; }
				if (isin(Preset,"mp_Nod_Obelisk")) { Announcement = "M00BNOL_DSGN0008I1EVAN_SND"; }
				if (isin(Preset,"mp_Hand_of_Nod")) { Announcement = "M00BNHN_DSGN0016I1EVAN_SND"; }
				if (isin(Preset,"mp_Nod_Airstrip")) { Announcement = "M00BNAF_DSGN0010I1EVAN_SND"; }
				if (isin(Preset,"mp_Nod_Power_Plant")) { Announcement = "M00BNPP_DSGN0010I1EVAN_SND"; }
				if (isin(Preset,"mp_Nod_Refinery")) { Announcement = "M00BNTR_DSGN0010I1EVAN_SND"; }
				if (Config->Sounds && stricmp(Announcement,"ERROR")) { Create_Sound_Team(Announcement,Commands->Get_Position(obj),obj,Get_Object_Type(obj)); }
				DamageExpire = (vManager.DurationAsInt() + 30);
			} else if (Health < MaxHealth * 0.2f) {
				char *Announcement = "ERROR";
				if (isin(Preset,"mp_GDI_Advanced_Guard_Tower")) { Announcement = "M00BGAT_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Barracks")) { Announcement = "M00BGIB_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_War_Factory")) { Announcement = "M00BGWF_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Power_Plant")) { Announcement = "M00BGPP_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Refinery")) { Announcement = "M00BGTR_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_ConstructionYard")) { Announcement = "M00BGCY_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Repair_Bay")) { Announcement = "M00BGRF_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Repair_Pad")) { Announcement = "M00BGRF_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_GDI_Tiberium_Silo")) { Announcement = "M00BGTS_HLTH0001I1EVAG_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Obelisk")) { Announcement = "M00BNOL_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Hand_of_Nod")) { Announcement = "M00BNHN_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Airstrip")) { Announcement = "M00BNAF_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Power_Plant")) { Announcement = "M00BNPP_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Refinery")) { Announcement = "M00BNTR_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_ConstructionYard")) { Announcement = "M00BNCY_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Repair_Bay")) { Announcement = "M00BNRF_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Repair_Pad")) { Announcement = "M00BNRF_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"mp_Nod_Tiberium_Silo")) { Announcement = "M00BNTS_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"_Site")) { Announcement = "M00BNSS_HLTH0001I1EVAN_SND.wav"; }
				else if (isin(Preset,"Nod_Heli_Port")) { Announcement = "M00BNHP_HLTH0001UI1EVAN_SND.wav"; }
				if (Config->Sounds && stricmp(Announcement,"ERROR")) { Create_Sound_Team(Announcement,Commands->Get_Position(obj),obj,Get_Object_Type(obj)); }
				DamageExpire = (vManager.DurationAsInt() + 30);
			} else if (Health < MaxHealth * 0.5f) {
				char *Announcement = "ERROR";
				if (Get_Object_Type(obj) == 1) Announcement = "MXXDSGN_DSGN0040I1EVAG_SND";
				else if (Get_Object_Type(obj) == 0) Announcement = "MXXDSGN_DSGN0038I1EVAN_SND";
				if (Config->Sounds && stricmp(Announcement,"ERROR")) { 
					Create_Sound_Team(Announcement,Commands->Get_Position(obj),obj,0);
					Create_Sound_Team(Announcement,Commands->Get_Position(obj),obj,1);
				}
				DamageExpire = (vManager.DurationAsInt() + 30);
			}
			LastDamage = Health;
		}
	}

	vVetManager->Add_Pending_VetPoints((Stewie_ScriptableGameObj *)damager,Commands->Get_ID(obj),damage);
}
void vBuildingObjectHandler::Killed(GameObject *obj, GameObject *shooter) {
	if (Is_Building_Dead(obj)) { return; }
	vVetManager->Object_Destroyed(obj);

	if (shooter && Get_Player_ID(shooter) > 0) {
		Set_Is_Visible((Stewie_ScriptableGameObj *)shooter,true);
		if (Get_Vehicle(shooter)) { Set_Is_Visible((Stewie_ScriptableGameObj *)Get_Vehicle(shooter),true); }
		std::string KillerName = Player_Name_From_GameObj(shooter);
		vPlayer *p = vPManager->Get_Player(KillerName.c_str());
		if (p) { p->BuildingsKilled++; } // MEDALS
		vLogger->Log(vLoggerType::vVGM,"_BUILDING","%s destroyed thanks to %s (%s)",Get_Pretty_Name(obj).c_str(),KillerName.c_str(),Get_Preset_Info(shooter).c_str());
		PrivMsgColoredVA(Get_Player_ID(shooter),2,0,200,0,"[VGM] The %s has been destroyed thanks to you! You have received a recommendation.",Get_Pretty_Name(obj).c_str());
		vPManager->Recommend(-1,Get_Player_ID(shooter),1);
	} else {
		vLogger->Log(vLoggerType::vVGM,"_BUILDING","%s destroyed",Get_Pretty_Name(obj).c_str());
	}
}
void vBuildingObjectHandler::Timer_Expired(GameObject *obj, int number) {
	if (number == 1500020) {
		int RandomAnnouncement = 0;
		char *Announcement = "ERROR";
		Vector3 Pos = Commands->Get_Position(obj);
		if (strstr(Commands->Get_Preset_Name(obj),"mp_GDI_Advanced_Guard_Tower")) {
			RandomAnnouncement = Random(1,2);
			if (RandomAnnouncement == 1) Announcement = "M00BGAT_DSGN0001I1EVAG_SND";
			else Announcement = "M00BGAT_DSGN0003I1EVAG_SND";
			Pos.Z -= 17.0f;
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_GDI_Barracks")) {
			RandomAnnouncement = Random(1,5);
			if (RandomAnnouncement == 1) Announcement = "M00BGIB_DSGN0001I1EVAG_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BGIB_DSGN0002I1EVAG_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BGIB_DSGN0003I1EVAG_SND";
			else if (RandomAnnouncement == 4) Announcement = "M00BGIB_DSGN0004I1EVAG_SND";
			else Announcement = "M00BGIB_DSGN0005I1EVAG_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_GDI_War_Factory")) {
			RandomAnnouncement = Random(1,4);
			if (RandomAnnouncement == 1) Announcement = "M00BGWF_DSGN0005I1EVAG_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BGWF_DSGN0006I1EVAG_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BGWF_DSGN0001I1EVAG_SND";
			else Announcement = "M00BGWF_DSGN0003I1EVAG_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_GDI_Power_Plant")) {
			RandomAnnouncement = Random(1,6);
			if (RandomAnnouncement == 1) Announcement = "M00BGPP_DSGN0001I1EVAG_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BGPP_DSGN0002I1EVAG_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BGPP_DSGN0003I1EVAG_SND";
			else if (RandomAnnouncement == 4) Announcement = "M00BGPP_DSGN0004I1EVAG_SND";
			else if (RandomAnnouncement == 5) Announcement = "M00BGPP_DSGN0005I1EEVAG_SND";
			else Announcement = "M00BGPP_DSGN0006I1EVAG_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_GDI_Refinery")) {
			RandomAnnouncement = Random(1,5);
			if (RandomAnnouncement == 1) Announcement = "M00BGTR_DSGN0001I1EVAG_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BGTR_DSGN0002I1EVAG_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BGTR_DSGN0003I1EVAG_SND";
			else if (RandomAnnouncement == 4) Announcement = "M00BGTR_DSGN0004I1EVAG_SND";
			else Announcement = "M00BGTR_DSGN0006I1EVAG_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_Nod_Obelisk")) {
			RandomAnnouncement = Random(1,4);
			if (RandomAnnouncement == 1) Announcement = "M00BNOL_DSGN0001I1EVAN_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BNOL_DSGN0002I1EVAN_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BNOL_DSGN0003I1EVAN_SND";
			else Announcement = "M00BNOL_DSGN0004I1EVAN_SND";
			Pos.Z += 12.0f;
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_Hand_of_Nod")) {
			RandomAnnouncement = Random(1,2);
			if (RandomAnnouncement == 1) Announcement = "M00BNHN_DSGN0001I1EVAN_SND";
			else Announcement = "M00BNHN_DSGN0002I1EVAN_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_Nod_Airstrip")) {
			RandomAnnouncement = Random(1,3);
			if (RandomAnnouncement == 1) Announcement = "M00BNAF_DSGN0001I1EVAN_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BNAF_DSGN0005I1EVAN_SND";
			else Announcement = "M00BNAF_DSGN0006I1EVAN_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_Nod_Power_Plant")) {
			RandomAnnouncement = Random(1,6);
			if (RandomAnnouncement == 1) Announcement = "M00BNPP_DSGN0001I1EVAN_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BNPP_DSGN0002I1EVAN_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BNPP_DSGN0003I1EVAN_SND";
			else if (RandomAnnouncement == 4) Announcement = "M00BNPP_DSGN0004I1EVAN_SND";
			else if (RandomAnnouncement == 5) Announcement = "M00BNPP_DSGN0005I1EVAN_SND";
			else Announcement = "M00BNPP_DSGN0006I1EVAN_SND";
		} else if (strstr(Commands->Get_Preset_Name(obj),"mp_Nod_Refinery")) {
			RandomAnnouncement = Random(1,4);
			if (RandomAnnouncement == 1) Announcement = "M00BNTR_DSGN0001I1EVAG_SND";
			else if (RandomAnnouncement == 2) Announcement = "M00BNTR_DSGN0002I1EVAG_SND";
			else if (RandomAnnouncement == 3) Announcement = "M00BNTR_DSGN0003I1EVAG_SND";
			else Announcement = "M00BNTR_DSGN0004I1EVAG_SND";
		}
		if (Config->Sounds && stricmp(Announcement,"ERROR") && Commands->Get_Health(obj) > 0.0f) {
			Commands->Create_Sound(Announcement,Pos,obj);
			Commands->Start_Timer(obj,this,(float)Random(60,180),number);
		}
	}
}

void vSAMSite::Created(GameObject *obj) {
	Attach_Script_Once(obj,"M00_Disable_Transition","");
	EnemyID = 0;
	Set_Skin(obj,"CNCStructureHeavy");
	Commands->Set_Shield_Type(obj,"CNCStructureHeavy");
	CurrHealth = Commands->Get_Health(obj);
}
void vSAMSite::Damaged(GameObject *obj, GameObject *damager, float damage) {
	if (damage == 0.0f) { return; }
	Commands->Set_Health(obj,CurrHealth - (damage));
	CurrHealth = Commands->Get_Health(obj);
	if (Commands->Get_Health(obj) <= 0.0f) {
		deathdamage = damage;
		return;
	}
}
void vSAMSite::Destroyed(GameObject *obj) {
	Commands->Action_Reset(obj,100);
}
void vSAMSite::Timer_Expired(GameObject *obj, int number) {
	if (number == 1) {
		if (IsValidEnemy(obj,Commands->Find_Object(EnemyID)) == false) {
			Commands->Action_Reset(obj,100);
			EnemyID = 0;
		} else {
			Commands->Start_Timer(obj,this,0.1f,number);
		}
	}
}
void vSAMSite::Enemy_Seen(GameObject *obj, GameObject *enemy) {
	if (IsValidEnemy(obj,enemy) == false) { return; } // Acquired Enemy is not valid.

	if (IsValidEnemy(obj,Commands->Find_Object(EnemyID))) { return; } // Current Enemy is already valid.

	if (Is_Base_Powered(Get_Object_Type(obj)) == false) { return; }

	ActionParamsStruct ActionParams;
	ActionParams.Set_Basic(this,100,1);
	ActionParams.Set_Face_Location(Commands->Get_Position(enemy),0);
	ActionParams.Set_Attack_Position(Commands->Get_Position(enemy),100,0,true);
	ActionParams.Set_Attack_Hold(enemy,100,0,true,false);
	Commands->Action_Attack(obj,ActionParams);

	Commands->Start_Timer(obj,this,0.1f,1);
	
	EnemyID = Commands->Get_ID(enemy);
}
bool vSAMSite::IsValidEnemy(GameObject *obj, GameObject *enemy) {
	if (!enemy) { return false; }

	if (Is_VTOLVehicle(enemy) == false) { return false; }

	if (Commands->Get_Health(enemy) <= 0) { return false; }

	if (Is_Script_Attached(enemy,"Invisible")) { return false; }
	if (Commands->Is_Object_Visible(obj,enemy) == false) { return false; }

	if (Commands->Get_Player_Type(enemy) == Commands->Get_Player_Type(obj)) { return false; }

	Vector3 GunObjPos = Commands->Get_Position(obj);
	Vector3 EnemyObjPos = Commands->Get_Position(enemy);

	return Commands->Get_Distance(GunObjPos,EnemyObjPos) > 20;
}

void SelfRepair::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,Get_Float_Parameter("time"),1);
}
void SelfRepair::Timer_Expired(GameObject *obj, int number) {
	if (Commands->Get_Health(obj) > 0.0f) {
		if (Commands->Get_Health(obj) < Commands->Get_Max_Health(obj)) {
			Commands->Set_Health(obj,(float)(Commands->Get_Health(obj) + Get_Float_Parameter("amount")));
		} else if (Commands->Get_Shield_Strength(obj) < Commands->Get_Max_Shield_Strength(obj)) {
			Commands->Set_Shield_Strength(obj,(float)(Commands->Get_Shield_Strength(obj) + Get_Float_Parameter("amount")));
		}
		Commands->Start_Timer(obj,this,Get_Float_Parameter("time"),1);
	}
}
void VetSelfRepair::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,Get_Float_Parameter("time"),1);
}
void VetSelfRepair::Timer_Expired(GameObject *obj, int number) {
	if (Commands->Get_Health(obj) > 0.0f) {
		if (Commands->Get_Health(obj) < Commands->Get_Max_Health(obj)) {
			Commands->Set_Health(obj,(float)(Commands->Get_Health(obj) + Get_Float_Parameter("amount")));
		} else if (Commands->Get_Shield_Strength(obj) < Commands->Get_Max_Shield_Strength(obj)) {
			Commands->Set_Shield_Strength(obj,(float)(Commands->Get_Shield_Strength(obj) + Get_Float_Parameter("amount")));
		}
		Commands->Start_Timer(obj,this,Get_Float_Parameter("time"),1);
	}
}

void ExpirePowerup::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,MinZero(Get_Float_Parameter("time") - 5.0f),1);
	Commands->Start_Timer(obj,this,Get_Float_Parameter("time"),2);
}
void ExpirePowerup::Timer_Expired(GameObject *obj, int number) {
	if (number == 1) {
		Commands->Create_Object("Spawner Created Special Effect",Commands->Get_Position(obj));
	} else if (number == 2) {
		Commands->Expire_Powerup(obj);
		if (obj) { Commands->Destroy_Object(obj); }
	}
}

void Parachute::Created(GameObject *obj) {
	Attached = false;
	StartPos = Commands->Get_Position(obj);
	Owner = Get_Player_ID(obj);
	floater = NULL;
	chute = NULL;
	LastZPos = FLT_MAX;
	Commands->Start_Timer(obj,this,0.05f,1600000);
}
void Parachute::Custom(GameObject *obj, int message, int param, GameObject *sender) {
	if (message == 1600003) {
		this->Timer_Expired(obj,1600002);
	}
}
void Parachute::Killed(GameObject *obj, GameObject *shooter) {
	this->Timer_Expired(obj,1600002);
}
void Parachute::Destroyed(GameObject *obj) {
	this->Timer_Expired(obj,1600002);
}
void Parachute::Timer_Expired(GameObject *obj, int number) {
	if (number == 1600000) {
		if (!Attached && (Commands->Get_Position(obj).Z <= StartPos.Z - 5.0f)) {
			floater = Commands->Create_Object("CnC_Beacon_IonCannon",Commands->Get_Position(obj));
			Commands->Set_Model(floater,"NULL");
			Commands->Set_Player_Type(floater,-4);

			chute = Commands->Create_Object("Invisible_Object",Commands->Get_Position(obj));
			Commands->Set_Model(floater,"X5D_Parachute");
			Commands->Set_Facing(chute,Commands->Get_Facing(obj) - 90.0f);
			Commands->Disable_All_Collisions(chute);

			Commands->Attach_To_Object_Bone(obj,floater,"Origin");
			Commands->Attach_To_Object_Bone(chute,obj,"Origin");
			Commands->Start_Timer(obj,this,0.05f,1600001);

			Attached = true;
		} else {
			Commands->Start_Timer(obj,this,0.05f,1600000);
		}
	} else if (number == 1600001 && Attached) {
		Vector3 P = Commands->Get_Position(floater);
		Stewie_BaseGameObj *bFloater = (Stewie_BaseGameObj *)floater;
		if (bFloater && chute && bFloater->As_PhysicalGameObj() && bFloater->As_PhysicalGameObj()->Phys) {
			P.Z += 0.02f;
			Commands->Set_Position(floater,P);
			Commands->Start_Timer(obj,this,0.05f,1600001);
			if ((Commands->Get_Position(obj).Z - LastZPos) <= 0.01f) { this->Timer_Expired(obj,1600002); }
			LastZPos = Commands->Get_Position(obj).Z;
		} else {
			this->Timer_Expired(obj,1600002);
		}
	} else if (number == 1600002) {
		if (obj && Attached && Player_GameObj(Owner) && !Get_Vehicle(obj)) {
			if (chute && Commands->Get_ID(chute)) { Commands->Destroy_Object(chute); }
			if (floater && Commands->Get_ID(floater)) { Commands->Destroy_Object(floater); }
		}
		Remove_Script(obj,"Parachute");
	}
}

void OffensiveScript::Created(GameObject *obj) {
	Commands->Start_Timer(obj,this,1.0f,1500000);
}
void OffensiveScript::Timer_Expired(GameObject *obj, int number) {
	if (number == 1500000) {
		// CHECK FOR REPAIR RECOMMENDATIONS
		vPManager->Check_All_Repair_Recommendations();
		// OFFENSIVE STUFF
		unsigned int t = vManager.DurationAsInt();
		if (t < 0) {
			Remove_Script(obj,"OffensiveScript");
			return;
		}
		if (t > (unsigned int)(Config->OffensiveModeTime)) {
			if (t - (unsigned int)(Config->OffensiveModeTime) % 300 == 150) {
				Attach_Script_Once(obj,"Ion_Storm","0");
			} else if (t - (unsigned int)(Config->OffensiveModeTime) % 300 == 0) {
				Attach_Script_Once(obj,"Ion_Storm","1");
			}
		} else if (t == (unsigned int)(Config->OffensiveModeTime)) {
			InitiateOffensive();
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 1)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0086i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 2)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0085i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 3)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0084i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 4)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0083i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 5)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0082i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 6)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0081i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 7)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0080i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 8)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0079i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 9)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0078i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 10)) {
			if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0077i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 30)) {
			HostMessage("[VGM] Warning: Offensive Mode begins in 30 seconds!");
			if (Config->Sounds) { Create_2D_WAV_Sound("m07dsgn_dsgn0011i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 60)) {
			HostMessage("[VGM] Warning: Offensive Mode begins in 1 minute!");
			if (Config->Sounds) { Create_2D_WAV_Sound("m07dsgn_dsgn0010i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 120)) {
			HostMessage("[VGM] Warning: Offensive Mode begins in 2 minutes!");
			if (Config->Sounds) { Create_2D_WAV_Sound("m07dsgn_dsgn0009i1evag_snd.wav"); }
		} else if (t == (unsigned int)(Config->OffensiveModeTime - 300)) {
			HostMessage("[VGM] Warning: Offensive Mode begins in 5 minutes!");
			if (Config->Sounds) { Create_2D_WAV_Sound("m00dsgn_dsgn1001i1ncxk_snd.wav"); }
		}

		// RESTART TIMER
		Commands->Start_Timer(obj,this,1.0f,1500000);
	}
}

void TeamCommander::Custom(GameObject *obj, int message, int param, GameObject *sender) {
	if (message == 800001) {
		if (Stewie_cPlayerManager::Is_Player_Present(param) == false) { return; }
		GameObject *o = Player_GameObj(param);
		if (!o) { return; }
		int team = Commands->Get_Player_Type(o);
		if (Has_Commander(team) == false) {
			const char *name = Get_Player_Name(o);
			if (Is_Commander(name,team,true)) {
				Set_Team_Commander(param,team);
			}
		}
	} else if (message == 800002) {
		if (Has_Commander(param) == false) {
			int ID = Get_Next_Commander(param);
			GameObject *o = Player_GameObj(ID);
			if (!o) { return; }
			const char *name = Get_Player_Name(o);
			if (Is_Commander(name,param,true)) {
				Set_Team_Commander(ID,param);
			}
		}
	}
}

void C4PokeScript::Poked(GameObject *obj, GameObject *poker) {
	if (Get_C4_Planter(obj)) {
		PrivMsgColoredVA(Get_Player_ID(poker),2,0,200,0,"[VGM] This %s C4 belongs to %s.",Get_Translated_C4_Mode(Get_C4_Mode(obj)).c_str(),Player_Name_From_GameObj(Get_C4_Planter(obj)).c_str());
	}
}

void Ion_Storm::Created(GameObject *obj) {
	Commands->Set_Rain(5.0f,3.5f,true);
	Commands->Set_Fog_Enable(1);
	Commands->Set_Fog_Range(0.5,55,3.5f);
	Commands->Set_Wind(0.7f,2.0f,1.0f,3.5f);
	if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0069i1evag_snd.wav"); }
	HostMessage("[VGM] WARNING, Ion Storm approaching the GDI base!");
	Commands->Start_Timer(obj,this,3.0f,1);
}
void Ion_Storm::Timer_Expired(GameObject *obj, int number) {
	if (number == 1) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0097i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,2);
	} else if (number == 2) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0076i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,0.7f,3);
	} else if (number == 3) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0098i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,10.0f,4);
	} else if (number == 4) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0082i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,5);
	} else if (number == 5) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0083i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,6);
	} else if (number == 6) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0084i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,7);
	} else if (number == 7) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0085i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,8);
	} else if (number == 8) {
		if (Config->Sounds) { Create_2D_WAV_Sound("m00evag_dsgn0086i1evag_snd.wav"); }
		Commands->Start_Timer(obj,this,1.0f,9);
	} else if (number == 9) {
		if (Find_War_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_War_Factory(Get_Int_Parameter("team")));
			position.Y += 20.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.7f,10);
	} else if (number == 10) {
		if (Find_Refinery(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Refinery(Get_Int_Parameter("team")));
			position.Y -= 20.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,1.0f,11);
	} else if (number == 11) {
		if (Find_Soldier_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Soldier_Factory(Get_Int_Parameter("team")));
			position.X += 15.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,12);
	} else if (number == 12) {
		if (Find_Refinery(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Refinery(Get_Int_Parameter("team")));
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
			float refhealth = Commands->Get_Health(Find_Refinery(Get_Int_Parameter("team")));
			Commands->Set_Health((Find_Refinery(Get_Int_Parameter("team"))),(refhealth/1.3f));
		}
		Commands->Start_Timer(obj,this,1.0f,13);
	} else if (number == 13) {
		if (Find_War_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_War_Factory(Get_Int_Parameter("team")));
			position.Y -= 20.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,14);
	} else if (number == 14) {
		if (Find_Soldier_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Soldier_Factory(Get_Int_Parameter("team")));
			position.Y -= 20.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,15);
	} else if (number == 15) {
		if (Find_Base_Defense(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Base_Defense(Get_Int_Parameter("team")));
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
			float agthealth = Commands->Get_Health(Find_Base_Defense(Get_Int_Parameter("team")));
			Commands->Set_Health((Find_Base_Defense(Get_Int_Parameter("team"))),(agthealth/1.4f));
		}
		Commands->Start_Timer(obj,this,0.6f,16);
	} else if (number == 16) {
		if (Find_Base_Defense(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Base_Defense(Get_Int_Parameter("team")));
			position.X += 10.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,17);
	} else if (number == 17) {
		if (Find_Soldier_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Soldier_Factory(Get_Int_Parameter("team")));
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
			float barhealth = Commands->Get_Health(Find_Soldier_Factory(Get_Int_Parameter("team")));
			Commands->Set_Health((Find_Soldier_Factory(Get_Int_Parameter("team"))),(barhealth/1.35f));
		}
		Commands->Start_Timer(obj,this,0.6f,18);
	} else if (number == 18) {
		if (Find_Refinery(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Refinery(Get_Int_Parameter("team")));
			position.Y -= 10.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,19);
	} else if (number == 19) {
		if (Find_Refinery(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Refinery(Get_Int_Parameter("team")));
			position.X += 5.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,1.0f,20);
	} else if (number == 20) {
		if (Find_War_Factory(Get_Int_Parameter("team")))
		{
			Vector3 position;
			position = Commands->Get_Position(Find_War_Factory(Get_Int_Parameter("team")));
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
			float wepshealth = Commands->Get_Health(Find_War_Factory(Get_Int_Parameter("team")));
			Commands->Set_Health((Find_War_Factory(Get_Int_Parameter("team"))),(wepshealth/1.4f));
		}
		Commands->Start_Timer(obj,this,0.6f,21);
	} else if (number == 21) {
		if (Find_War_Factory(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_War_Factory(Get_Int_Parameter("team")));
			position.X += 10.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.8f,22);
	} else if (number == 22) {
		if (Find_Base_Defense(Get_Int_Parameter("team")))
		{
			Vector3 position;
			position = Commands->Get_Position(Find_Base_Defense(Get_Int_Parameter("team")));
			position.Y += 15.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,0.6f,23);
	} else if (number == 23) {
		if (Find_Base_Defense(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Base_Defense(Get_Int_Parameter("team")));
			position.X += 5.0f;
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
		}
		Commands->Start_Timer(obj,this,1.0f,24);
	} else if (number == 24) {
		if (Find_Power_Plant(Get_Int_Parameter("team"))) {
			Vector3 position;
			position = Commands->Get_Position(Find_Power_Plant(Get_Int_Parameter("team")));
			if (Config->Sounds) { Create_2D_WAV_Sound("ion_fire.wav"); }
			Commands->Create_Explosion("Explosion_Mine_Remote_01",position,obj);
			Commands->Create_Object("Beacon_Ion_Cannon_Anim_Post",position);
			float pphealth = Commands->Get_Health(Find_Power_Plant(Get_Int_Parameter("team")));
			Commands->Set_Health((Find_Power_Plant(Get_Int_Parameter("team"))),(pphealth/1.3f));
		}
		Commands->Set_Fog_Range (0.5f,350.0f,12.0f);
		Commands->Set_Rain(0.0f,10.0f,true);
		Commands->Set_Wind(0.0f,0.0f,0.0f,3.5f);
		Commands->Start_Timer(obj,this,11.0f,25);
	} else if (number == 25) {
		Commands->Set_Fog_Enable(0);
		HostMessage("[VGM] The Ion Storm has subsided the %s base.",Get_Translated_Team_Name(Get_Int_Parameter("team")).c_str());
		Commands->Set_Rain(0.0f,0.0f,true);
		Remove_Script(obj,"Ion_Storm");
	}
}

ScriptRegistrant<vPlayerObjectHandler> vPlayerObjectHandler_Registrant("M00_GrantPowerup_Created","WeaponDef:string");
STSCRIPT(vVehicleObjectHandler,VehicleObjectRegistrant,"M00_Vehicle_Log","");
ScriptRegistrant<vVehicleShellHandler> vVehicleShellHandler_Registrant("M00_Rebuild_Shell","Preset:string");
ScriptRegistrant<vHarvesterShellHandler> vHarvesterShellHandler_Registrant("vHarvesterShellHandler","");
ScriptRegistrant<vTowerObjectHandler> vTowerObjectHandler_Registrant("M00_Disable_Transition","");
ScriptRegistrant<vBuildingObjectHandler> vBuildingObjectHandler_Registrant("M00_BUILDING_EXPLODE_NO_DAMAGE_DAK","");
ScriptRegistrant<vSAMSite> vSAMSite_Registrant("vSAMSite","");
ScriptRegistrant<SelfRepair> SelfRepair_Registrant("SelfRepair","amount:float,time:float");
ScriptRegistrant<VetSelfRepair> VetSelfRepair_Registrant("VetSelfRepair","amount:float,time:float");
ScriptRegistrant<Parachute> Parachute_Registrant("Parachute","");
ScriptRegistrant<ExpirePowerup> ExpirePowerup_Registrant("ExpirePowerup","time:float");
ScriptRegistrant<OffensiveScript> OffensiveScript_Registrant("OffensiveScript","");
ScriptRegistrant<TeamCommander> TeamCommander_Registrant("TeamCommander","");
ScriptRegistrant<C4PokeScript> C4PokeScript_Registrant("C4PokeScript","");
ScriptRegistrant<Ion_Storm> Ion_Storm_Registrant("Ion_Storm","team:int");
STSCRIPT(IAmGod,GodCrateRegistrant,"IAmGod","");
STSCRIPT(Invisible,InvisibleCharRegistrant,"Invisible","");

void __stdcall C4InitHook(Stewie_C4GameObj *obj) {
	Stewie_SoldierGameObj *Planter = Get_C4_Owner(obj);
	if (!Planter || !Planter->Player) { return; }

	vPlayer *player = vPManager->Get_Player(Planter->Player->PlayerId);
	if (player && !player->serial && Config->BlockNoSerialActions) { Kill_C4(obj); return; } // Don't allow people with no serial to throw C4

	int Type = Get_C4_Type(obj);
	Stewie_C4GameObj *FirstC4 = Find_First_C4_By_Player(player->ID,Type);

	if (Get_Player_C4_Count((GameObject *)Planter,1) > 20) { Kill_C4(FirstC4); return; } // Maintain Player's Remote Limit
	if (Get_Player_C4_Count((GameObject *)Planter,2) > 5) { Kill_C4(FirstC4); return; } // Maintain Player's Timed Limit

	int Team = obj->PlayerType;
	int Limit = Get_C4_Limit_Type(Team,Type);
	if (Get_C4_Count_Type(Team,Type) > Limit) { Kill_C4(FirstC4); return; } // Maintain Team's C4 Limit

	Attach_Script_Once((GameObject *)obj,"C4PokeScript",""); // Attach poking script
}
void __cdecl C4LimitHook(int team) {
	// This is all handled by the C4 Init Hook.
	return;
}
bool __stdcall C4ShouldExplodeHook(Stewie_DamageableGameObj *enemy) {
	Stewie_DamageableGameObj *o;
	__asm { mov o,ecx }

	// If there's no C4, and no enemy, then why detonate?
	if (!o || !enemy) { return false; }
	if (!o->obj) { return false; }
	if (!enemy->obj) { return false; }
	if (enemy->obj->As_BuildingGameObj()) { return false; }

	Stewie_C4GameObj *obj = o->As_PhysicalGameObj()->As_C4GameObj();

	// If it's dead, or hovering, don't detonate.
	if (enemy->Defence.Get_Health() <= 0.0f) { return false; }
	if (Is_Script_Attached((GameObject *)(enemy->obj),"Hover")) { return false; }
	
	// Check if enemy is spectating...
	Stewie_PhysicalGameObj *AsPhys = enemy->obj->As_PhysicalGameObj();
	if (!AsPhys || !AsPhys->Phys || !AsPhys->Phys->Model) { return false; }
	if (!stricmp(AsPhys->Phys->Model->Get_Name(),"NULL")) { return false; }

	// Every C4 detonates if someone goes to bluehell, cause the distance is NaN and treated as zero.
	Stewie_Vector3 Position = World_Position(obj);
	Stewie_Vector3 PositionTrigger = World_Position(enemy->obj);
	if (_isnan(PositionTrigger.X) || _isnan(PositionTrigger.Y) || _isnan(PositionTrigger.Z)) { return false; }

	// Check distance for proximity mines.
	if (Get_C4_Type(obj) == 3 && obj->Ammo) {
		float Distance = Position.Distance(PositionTrigger);
		if (Distance > obj->Ammo->C4TriggerRange3) { return false; }
		if (Distance > GameDataObj()->MaxWorldDistance) { return false; }
	}

	// Default call.
	return o->Is_Enemy(enemy);
}
bool __stdcall C4DefuseHook(GameObject *obj, Stewie_OffenseObjectClass &offense) {
	if (!obj) { return false; }
	if (offense.dmgr.Reference && offense.dmgr.Reference->obj && offense.dmgr.Reference->obj->As_SoldierGameObj()) {
		Stewie_C4GameObj *C4Obj = (Stewie_C4GameObj *)obj;
		float TotalDmg = C4Obj->Defence.HealthMax.Get() + C4Obj->Defence.ShieldStrengthMax.Get();
		vVetManager->Add_Pending_VetPoints(offense.dmgr.Reference->obj,Commands->Get_ID(obj),TotalDmg);
	}
	vVetManager->Object_Destroyed(obj);
	return true;
}
bool __stdcall C4DetonationHook(GameObject *obj) {
	if (!obj) { return false; }
	GameObject *Planter = Get_C4_Planter(obj);
	if (!Planter) { return true; }

	int Mode = Get_C4_Mode(obj);

	if (Mode == 3) {
		Stewie_Vector3 Position = World_Position((Stewie_BaseGameObj *)obj);
		Stewie_Vector3 PositionOwner = World_Position((Stewie_BaseGameObj *)Planter);
		if (_isnan(PositionOwner.X) || _isnan(PositionOwner.Y) || _isnan(PositionOwner.Z)) { return false; }
		if (Position.Length() > GameDataObj()->MaxWorldDistance) { return false; }
		if (PositionOwner.Length() > GameDataObj()->MaxWorldDistance) { return false; }
	}

	if (Mode > 1) {
		GameObject *o = Get_C4_Attached(obj);
		if (o) { vLogger->Log(vLoggerType::vVGM,"_C4","%s C4 explosion (Planter: %s - Attached to: %s)",Get_Translated_C4_Mode(Mode).c_str(),Player_Name_From_GameObj(Planter).c_str(),Get_Pretty_Name(o).c_str()); }
		else { vLogger->Log(vLoggerType::vVGM,"_C4","%s C4 explosion (Planter: %s)",Get_Translated_C4_Mode(Mode).c_str(),Player_Name_From_GameObj(Planter).c_str()); }
	}
	return true;
}
void __stdcall C4PlantedHook(GameObject *obj) {
	if (!obj) { return; }
	GameObject *Attached = Get_C4_Attached(obj);
	if (!Attached) { return; }
	GameObject *Planter = Get_C4_Planter(obj);
	bool IsVeh = Is_Vehicle(Attached);
	if (IsVeh || Is_Soldier(Attached)) {
		if (Get_Object_Type(Attached) == Get_Object_Type(Planter) && GameDataObj()->IsFriendlyFirePermitted == false) {
			Kill_C4(obj);
		} else if (IsVeh && Is_Script_Attached(Attached,"AntiMine")) {
			Kill_C4(obj);
		}
	}
}

bool __stdcall EnemySeenHook(Stewie_DamageableGameObj *enemy) {
	Stewie_DamageableGameObj *o;
	__asm { mov o,ecx }
	if (!o || !o->obj || !enemy || !enemy->obj) { return false; }

	if (Script_Attached(o->obj,"Invisible")) { return false; }
	if (enemy->Defence.Get_Health() <= 0.0f) { return false; }

	Stewie_PhysicalGameObj *eAsPhys = enemy->obj->As_PhysicalGameObj();
	if (!eAsPhys || !eAsPhys->Phys || !eAsPhys->Phys->Model || !stricmp(eAsPhys->Phys->Model->Get_Name(),"NULL")) { return false; }
	
	Stewie_SmartGameObj *oAsSmart = o->obj->As_SmartGameObj();
	if (oAsSmart && !oAsSmart->Is_Obj_Visible(eAsPhys)) { return false; }

	if (Config->CrazyDefensesEnabled) {
		if (eAsPhys->As_SoldierGameObj() || (eAsPhys->As_VehicleGameObj() && Vehicle_Driver(eAsPhys->As_VehicleGameObj()))) {
			if (enemy->PlayerType == o->PlayerType || enemy->PlayerType == 1 || enemy->PlayerType == 0) { return true; }
		}
	}

	return o->Is_Enemy(enemy);
}

void __stdcall BeaconStateHook(GameObject *obj, int State) {
	if (State == 1) { // Laid
		if (Config->GameMode == Config->vSNIPER) {
			Commands->Destroy_Object(obj);
		} else {
			Stewie_BeaconGameObj *b = (Stewie_BeaconGameObj *)obj;
			GameObject *layer = Get_Beacon_Planter(obj);
			vLogger->Log(vLoggerType::vVGM,"_BEACON2","%s placed %s %s",Player_Name_From_GameObj(layer).c_str(),(isin(Get_Preset_Name(obj),"Ion") ? "an" : "a"),Get_Pretty_Name(obj).c_str());			
		}
	} else if (State == 2) { // Deployed
		GameObject *layer = Get_Beacon_Planter(obj);
		int ID = Get_Player_ID(layer);
		if (Get_Player_Beacon_Count(ID) > 1) {
			PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You can only have one beacon deployed at a time.");
			Disarm_Beacon(obj);
		} else {
			int ETeam = (int)(1 - Get_Object_Type(layer));
			char m[512];
			std::string bldgs = Buildings_Beacon_Will_Hit(obj,ETeam,true);
			if (!stricmp(bldgs.c_str(),"None")) { vLogger->Log(vLoggerType::vVGM,"_BEACON2","%s has deployed a beacon that will not strike any enemy buildings, therefore it may be a spam or fake beacon.",Player_Name_From_GameObj(layer).c_str()); }
			sprintf(m,"ATTENTION: A(n) %s has been placed. Building(s) that will be damaged: %s.",Get_Pretty_Name(obj).c_str(),bldgs.c_str());
			New_Chat_Message(ID,-1,(const char *)m,false,1);
			Attach_Script_Once(obj,"vBeaconObjectHandler","");
			vLogger->Log(vLoggerType::vVGM,"_BEACON","%s deployed %s %s",Player_Name_From_GameObj(layer).c_str(),(isin(Get_Preset_Name(obj),"Ion") ? "an" : "a"),Get_Pretty_Name(obj).c_str());
		}
	} else if (State == 3) { // Disarmed
		//
	} else if (State == 4) { // Detonated
		vLogger->Log(vLoggerType::vVGM,"_BEACON","%s detonated",Get_Pretty_Name(obj).c_str());
		PrivMsgColoredVA(Get_Player_ID(Get_Beacon_Planter(obj)),2,128,0,128,"[VGM] Your %s has detonated.",Get_Pretty_Name(obj).c_str());
	} else if (State == 5) { // Warning
		const char *preset = Get_Preset_Name(obj);
		if (strstr(preset,"annon")) {
			vLogger->Log(vLoggerType::vVGM,"_BEACON","Warning - Ion Cannon Satellite approaching");
		} else {
			vLogger->Log(vLoggerType::vVGM,"_BEACON","Warning - Nuclear Strike approaching");
		}
	} else if (State == 6) { // Initiated
		const char *preset = Get_Preset_Name(obj);
		if (strstr(preset,"annon")) {
			vLogger->Log(vLoggerType::vVGM,"_BEACON","Ion Cannon Strike initiated");
		} else {
			vLogger->Log(vLoggerType::vVGM,"_BEACON","Nuclear Strike initiated");
		}
	}
}
bool __stdcall BeaconRequestedHook(Stewie_BeaconGameObj *obj, Stewie_Vector3& pos) {
	GameObject *o = (GameObject *)obj->Get_Owner();
	GameObject *bobj = (GameObject *)obj;
	int ID = Get_Player_ID(o);

	if (Get_Player_Beacon_Count(ID) > 1) {
		PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You can only lay one beacon at a time.");
		return false;
	}
	if (obj && bobj) {
		Vector3 Position = Commands->Get_Position(bobj);
		if (!stricmp(GameDataObj()->MapName,"C&C_Field.mix") &&
			Position.Z > 3.0f && Position.Z < 5.0f) {
			GameObject *Closest = Find_Closest_Building_By_Team(2,Position);
			if (Closest == Find_Refinery(1) || Closest == Find_Refinery(0)) {
				PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You cannot place your beacon here.");
				return false;
			}
		} else if (!stricmp(GameDataObj()->MapName,"C&C_Canyon.mix") &&
			((Position.Z < -1.8f && Position.Y > -108.0f) ||
			(Position.Z < -3.0f && Position.Y < -68.0f))) {
			PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You cannot place your beacon here.");
			return false;
		} else if (!stricmp(GameDataObj()->MapName,"C&C_Snow.mix") &&
			Position.Z > 4.5f) {
			PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You cannot place your beacon here.");
			return false;
		} else if (!stricmp(GameDataObj()->MapName,"C&C_Glacier_Flying.mix") &&
			(Position.Z > 7.01f && Position.Z < 7.03f) &&
			(Position.Y < -75.0f || (Position.Y > 75.0f && Position.X < 100.0f))) {
			PrivMsgColoredVA(ID,2,128,0,128,"[VGM] You cannot place your beacon here.");
			return false;
		}
	}

	vPlayer *player = vPManager->Get_Player(Player_Name_From_ID(ID).c_str());
	if (player != NULL) {
		if (player->serial == false && Config->BlockNoSerialActions) {
			Kill_Player(o);
			Commands->Give_Money(o,1000,false);
			Inc_Kills(Stewie_cPlayerManager::Find_Player(ID),-1);
			return false;
		}
	}
	return true;
}
void __stdcall BeaconDisarmedHook(GameObject *obj, Stewie_OffenseObjectClass *Disarmer) {
	bool IsShooter = true;
	if (!Disarmer || !Disarmer->dmgr.Reference || !Disarmer->dmgr.Reference->obj || !Disarmer->dmgr.Reference->obj->As_SoldierGameObj()) { IsShooter = false; }
	const char *preset = Get_Pretty_Name(obj).c_str();
	if (strlen(preset) <= 1) { preset = "Beacon"; }
	if (IsShooter == false) {
		vLogger->Log(vLoggerType::vVGM,"_BEACON","%s's %s was disarmed.",Player_Name_From_GameObj(Get_Beacon_Planter(obj)).c_str(),preset);
	} else {
		Stewie_BeaconGameObj *BeaconObj = (Stewie_BeaconGameObj *)obj;
		float TotalDmg = BeaconObj->Defence.HealthMax.Get() + BeaconObj->Defence.ShieldStrengthMax.Get();
		vVetManager->Add_Pending_VetPoints(Disarmer->dmgr.Reference->obj,Commands->Get_ID(obj),TotalDmg);
		vVetManager->Object_Destroyed(obj);
		vLogger->Log(vLoggerType::vVGM,"_BEACON","%s disarmed %s's %s.",Player_Name_From_GameObj(Disarmer->dmgr.Reference->obj).c_str(),Player_Name_From_GameObj(Get_Beacon_Planter(obj)).c_str(),preset);
	}
}
bool __stdcall BeaconOwnerInterruptedHook(Stewie_BeaconGameObj *beacon) {
	return beacon->Was_Owner_Interrupted();
}
bool __stdcall PedestalTimerHook(Stewie_BeaconGameObj *beacon) {
	if (beacon->DetonateTime <= 0.0f) { return true; }
	return false;
}
bool __stdcall PedestalHook(GameObject *beacon) {
	GameObject *o = Get_Beacon_Planter(beacon);
	if (stricmp(Get_Preset_Name(beacon),"CnC_Beacon_IonCannon") && stricmp(Get_Preset_Name(beacon),"CnC_Beacon_NukeStrike")) { return false; }
	int ID = Get_Player_ID(o);
	if (vManager.DurationAsInt() < 900) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] You must wait at least 15 minutes to use the Pedestal.");
		return false;
	} else if (Get_Building_Count_Team(Commands->Get_Player_Type(o)) >= Get_Building_Count_Team((int)(1 - Commands->Get_Player_Type(o)))) {
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] Your team must have less buildings than the other team to be able to use the Pedestal.");
		return false;
	}
	return true;
}

void Create_2D_WAV_Sound(const char *sound) {
	for (Stewie_SLNode<Stewie_cPlayer>* Node = Stewie_cPlayerManager::PlayerList.HeadNode; Node; Node = Node->NodeNext) {
		if (!Node->NodeData) { continue; }
		if (Node->NodeData->IsActive == false || Node->NodeData->IsInGame == false) { continue; }
		Create_2D_WAV_Sound_Player(Player_GameObj(Node->NodeData->PlayerId),sound);
	}
}
