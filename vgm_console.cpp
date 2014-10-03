#include "vgm_actions.h"
#include "vgm_player.h"
#include "vgm_console.h"

class Ban_Hash : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_banhash"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_BANHASH <HASH> <0/1> - Bans or unbans a hash (0 = unban, 1 = ban).");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			const char *hash = strtok((char *)input," ");
			if (strlen(hash) == 32) {
				int m = atoi(strtok(NULL," "));
				if (m == 1) {
					writeProfileString("HashBans",hash,"1","vgm_bans.ini");
					ConsoleOut("[VGM] Hash %s has been banned.",hash);
				} else if (m == 0) {
					if (getProfileInt("HashBans",hash,0,"vgm_bans.ini") == 1) { 
						ConsoleOut("[VGM] Hash %s has been removed from the banlist.",hash);
						writeProfileString("HashBans",hash,"0","vgm_bans.ini");
					} else {
						ConsoleOut("[VGM] Hash %s is not in the banlist.",hash);
					}
				} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Buildings : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_buildinginfo"; }
	const char* Get_Alias() { return "v_buildings"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_BUILDINGINFO - Shows building status for both teams.");
		return "";
	}
	void Activate(const char* input) {
		Building_Status();
	}
};

class CharacterChg_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_character"; }
	const char* Get_Alias() { return "v_char"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_CHARACTER <ID> <unit> - Changes a player's character.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			if (ID > 0) {
				char w[256];
				sprintf(w,"%s",Get_Preset_From_Short("char",strtok(NULL," ")).c_str());
				if (stricmp((const char *)w,"None")) {
					Change_Character(Player_GameObj(ID),(const char *)w);
					ConsoleOut("[VGM] Player %s is now a %s.",Player_Name_From_ID(ID).c_str(),Get_Pretty_Name((const char *)w).c_str());
				} else { ConsoleOut("[VGM] Unknown unit."); }
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Create_Veh : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_create"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_CREATE <ID> <vehicle> - Gives a vehicle to a player.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			if (ID > 0) {
				char* V = strtok(NULL," ");
				std::string Veh = Get_Preset_From_Short("veh",V);
#ifdef _DEV_
				if (!stricmp(Veh.c_str(),"None")) { Veh = std::string(V); }
#else
				if (!stricmp(Veh.c_str(),"None")) { ConsoleOut("[VGM] Unknown unit."); return; }
#endif
				GameObject *o = Player_GameObj(ID);
				GameObject *f = Find_Vehicle_Factory(Commands->Get_Player_Type(o));
				if (f && Commands->Get_Health(f) > 0.0f) {
					Create_Vehicle(Veh.c_str(),0.0f,o,Commands->Get_Player_Type(o));
					ConsoleOut("[VGM] Player %s has been given a %s.",Player_Name_From_ID(ID).c_str(),Get_Pretty_Name(Veh.c_str()).c_str());
				} else { ConsoleOut("[VGM] The %s for team %s is either dead or not available. Use V_SPAWN instead.",f ? Get_Pretty_Name(f).c_str() : "Vehicle Factory",Get_Translated_Team_Name(Commands->Get_Player_Type(o)).c_str()); }
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Credit_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_credit"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_CREDIT <ID> <amount> - Gives a player an amount of credits. Negative numbers accepted.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				GameObject *o = Player_GameObj(ID);
				Commands->Give_Money(o,x,false);
				ConsoleOut("[VGM] Player %s has been given %.0f credits.",Player_Name_From_ID(ID).c_str(),x);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Credit2_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_credit2"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_CREDIT2 <ID> <amount> - Sets a player's credits to a given amount. Negative numbers accepted.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) {
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				GameObject *o = Player_GameObj(ID);
				Commands->Give_Money(o,-1 * Commands->Get_Money(o) + x,false);
				ConsoleOut("[VGM] Player %s now has %.0f credits.",Player_Name_From_ID(ID).c_str(),x);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Emergency_Shutdown : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_emergencyshutdown"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_EMERGENCYSHUTDOWN - Emergency shutdown for the server in case of internal or severe errors. USE AT OWN RISK.");
		return "";
	}
	void Activate(const char* input) {
		EmergencyShutdown = true;
	}
};

class GetPos_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_getpos"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_GETPOS <ID> - Show's a player's X,Y,Z coords and their Facing Angle.");
		return "";
	}
	void Activate(const char* input) {
		int ID = atoi(input);
		if (ID > 0) {
			Vector3 v = Commands->Get_Position(Player_GameObj(ID));
			float f = Commands->Get_Facing(Player_GameObj(ID));
			ConsoleOut("[VGM] Player %s is at %fX,%fY,%fZ, facing %f.",Player_Name_From_ID(ID).c_str(),v.X,v.Y,v.Z,f);
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Get_Serial : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_getserial"; }
	const char* Get_Alias() { return "v_serial"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_GETSERIAL <ID> - Gets the hash of a player's serial number.");
		return "";
	}
	void Activate(const char* input) {
		int ID = atoi(input);
		if (ID > 0) {
			Request_Serial(ID);
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Grant_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_grant"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_GRANT <ID> <weapon> - Gives a weapon to a player.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			if (ID > 0) {
				const char *t = strtok(NULL," ");
				if (!stricmp(t,"all")) {
					Stewie_SoldierGameObj *s = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
					if (Commands->Get_Health((GameObject *)s) <= 0.0f) { return; }
					Give_All_Weapons(ID);
					ConsoleOut("[VGM] Player %s has been given all weapons.",Player_Name_From_ID(ID).c_str());
				} else {
					char w[256];
					sprintf(w,"%s",Get_Preset_From_Short("weapons",t).c_str());
					if (stricmp((const char *)w,"None")) { Give_Weapon(ID,(const char *)w); }
					Stewie_SoldierGameObj *s = Stewie_GameObjManager::Find_Soldier_Of_Client_ID(ID);
					if (Commands->Get_Health((GameObject *)s) <= 0.0f) { return; }
					ConsoleOut("[VGM] Player %s has been given a(n) %s.",Player_Name_From_ID(ID).c_str(),Get_Pretty_Name((const char *)w).c_str());
				}
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Kill_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_kill"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_KILL <ID> - Kills a player.");
		return "";
	}
	void Activate(const char* input) {
		int ID = atoi(input);
		if (ID > 0) {
			Kill_Player(ID);
			ConsoleOut("[VGM] Player %s has been killed.",Player_Name_From_ID(ID).c_str());
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Move_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_move"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_MOVE <ID> <x> <y> <z> - Moves a player X, Y, Z respectively.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 4) { 
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			float y = (float)atof(strtok(NULL," "));
			float z = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				Move_Player(ID,x,y,z,true,true);
				ConsoleOut("[VGM] Player %s has been moved %fX,%fY,%fZ.",Player_Name_From_ID(ID).c_str(),x,y,z);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Mute_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_mute"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_MUTE <ID> <0/1> - Toggles player's muting (0 = off, 1 = on).");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			int m = atoi(strtok(NULL," "));
			if (ID > 0) {
				std::string rname = Player_Name_From_ID(ID);
				vPlayer *player = vPManager->Get_Player(rname.c_str());
				if (player) {
					if (m == 1) {
						player->Set_Muted(true);
						ConsoleOut("[VGM] Player %s has been muted.",rname.c_str());
					} else if (m == 0) {
						player->Set_Muted(false);
						ConsoleOut("[VGM] Player %s is no longer muted.",rname.c_str());
					}
				}
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class PInfo : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_pinfo"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_PINFO - Shows all players ingame and corresponding info such as score, kills, deaths, etc..");
		return "";
	}
	void Activate(const char* input) {
		ConsoleOut("Start PInfo output");
		for (int i = 1; i <= 128; i++) {
			Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(i);
			if (player && player->IsInGame && player->IsActive) {
				GameObject *o = Player_GameObj(i);
				ConsoleOut("%d,%s,%.0f,%d,%d,%s;%d,%d,%d,%d,%d,%.0f,%f,%s,%s",
					i,Player_Name_From_ID(i).c_str(),
					(float)(player->Score.Get()),
					(int)(player->PlayerType.Get()),
					player->Get_Ping(),
					inet_ntoa(player->IpAddress),
					Stewie_cNetwork::PServerConnection->RemoteHosts[i]->address.sin_port,
					Get_KBPS(i),
					0,
					(int)(player->Kills.Get()),
					(int)(player->Deaths.Get()),
					(float)(player->Money.Get()),
					Get_KD_Ratio(player),
					Get_Pretty_Name(o).c_str(),
					Get_Pretty_Name(Get_Vehicle(o)).c_str()
				);
			}
		}
		ConsoleOut("End PInfo output");
	}
};

class PLimit : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_plimit"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_PLIMIT <num> - Sets the number of players allowed ingame.");
		return "";
	}
	void Activate(const char* input) {
		unsigned int Plimit = (unsigned int)atoi(input);
		if (Plimit > 0 && Plimit <= 127) {
			bool Success = GameDataObj()->Set_Max_Players(Plimit);
			if (Success) { ConsoleOut("[VGM] Player limit set to %u.",Plimit); }
			else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class QSpec_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_qspectate"; }
	const char* Get_Alias() { return "v_qspec"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_QSPECTATE <ID> - Activate spectating for player.");
		return "";
	}
	void Activate(const char* input) {
		int ID = atoi(input);
		if (ID > 0) {
			if (Stewie_cPlayerManager::Find_Player(ID)) {
				int s = Spectate_Player(ID,true);
				if (s == false) { ConsoleOut("[VGM] Player %s is no longer quietly spectating.",Player_Name_From_ID(ID).c_str()); }
				else { ConsoleOut("[VGM] Player %s is now in quiet spectate mode.",Player_Name_From_ID(ID).c_str()); }
			} else {
				ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str());
			}
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Rehash : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_rehash"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_REHASH - Rehashes all Settings (vgm.ini changes).");
		return "";
	}
	void Activate(const char* input) {
		Config->Rehash(false,false,true);
		ConsoleOut("[VGM] Configuration rehashed.");
	}
};

class Score_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_score"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_SCORE <ID> <amount> - Increases/decreases a player's score by a given amount. Negative numbers accepted.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) {
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				GameObject *o = Player_GameObj(ID);
				Commands->Give_Points(o,x,false);
				ConsoleOut("[VGM] Player %s has been given %.0f points.",Player_Name_From_ID(ID).c_str(),x);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Score2_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_score2"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_SCORE2 <ID> <amount> - Sets a player's score to a given amount. Negative numbers accepted.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) {
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				GameObject *o = Player_GameObj(ID);
				Commands->Give_Points(o,-1 * Commands->Get_Points(o) + x,false);
				ConsoleOut("[VGM] Player %s now has %.0f points.",Player_Name_From_ID(ID).c_str(),x);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class SetPos_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_setpos"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_SETPOS <ID> <x> <y> <z> - Moves a player to a set of X, Y, Z coords.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 4) { 
			int ID = atoi(strtok((char *)input," "));
			float x = (float)atof(strtok(NULL," "));
			float y = (float)atof(strtok(NULL," "));
			float z = (float)atof(strtok(NULL," "));
			if (ID > 0) {
				Move_Player(ID,x,y,z,false);
				ConsoleOut("[VGM] Player %s has been moved to %fX,%fY,%fZ.",Player_Name_From_ID(ID).c_str(),x,y,z);
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class SetFPS : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_sfpslimit"; }
	const char* Get_Alias() { return "v_sfps"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("V_SFPSLIMIT <FPS> - Sets FPS limit for the server.");
		return "";
	}
	void Activate(const char* input) {
		if (input) {
			int NewFps = atoi(input);
			int Success = Set_SFPS_Limit(NewFps);
			if (Success == -1) { ConsoleOut("[VGM] Could not set server FPS at this time."); }
			else if (Success == 0) { ConsoleOut("[VGM] Server FPS uncapped."); }
			else { ConsoleOut("[VGM] New server FPS is now: %d",Success); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Spawn_Veh : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_spawn"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_SPAWN <ID> <vehicle> <amount> - Gives an amount of a vehicle to a player. If no amount specified, 1 is used.");
		return "";
	}
	void Activate(const char* input) {
		vTokenParser *Str = new vTokenParser();
		Str->Set((char *)input);
		Str->Parse(" ");
		if (Str->Numtok() >= 2) {
			int ID = atoi(Str->Gettok(1).c_str());
			if (ID > 0) {
				std::string w = Get_Preset_From_Short("veh",Str->Gettok(2).c_str());
				if (stricmp(w.c_str(),"None")) {
					int a = 1;
					if (Str->Numtok() >= 3) { a = atoi(Str->Gettok(3).c_str()); }
					if (a <= 0) { a = 1; }
					Vector3 pos = Commands->Get_Position(Player_GameObj(ID));
					for (int i = 0; i < a; i++) {
						pos.Z += 5.0f;
						GameObject *o = Commands->Create_Object(w.c_str(),pos);
						Commands->Set_Player_Type(o,-2);
					}
					ConsoleOut("[VGM] Player %s has been given %d '%s's.",Player_Name_From_ID(ID).c_str(),a,w.c_str());
				} else { ConsoleOut("[VGM] Unknown unit."); }
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		Str->Delete();
	}
};

class Spec_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_spectate"; }
	const char* Get_Alias() { return "v_spec"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("V_SPECTATE <ID> - Activate spectating for player.");
		return "";
	}
	void Activate(const char* input) {
		int ID = atoi(input);
		if (ID > 0) {
			if (Stewie_cPlayerManager::Find_Player(ID)) {
				int s = Spectate_Player(ID,false);
				if (s == false) { ConsoleOut("[VGM] Player %s is no longer spectating.",Player_Name_From_ID(ID).c_str()); }
				else { ConsoleOut("[VGM] Player %s is now in spectate mode.",Player_Name_From_ID(ID).c_str()); }
			} else {
				ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str());
			}
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Tele_Pl : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_tele"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_TELE <ID> <x> <y> <z> - Moves a player to a set of X, Y, Z coords.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 2) { 
			int ID = atoi(strtok((char *)input," "));
			int target = atoi(strtok(NULL," "));
			if (ID > 0) {
				GameObject *obj = Player_GameObj(target);
				Vector3 pos = Commands->Get_Position(obj);
				Move_Player(ID,pos.X,pos.Y,(pos.Z + 3),false);
				ConsoleOut("[VGM] Player %s has been teleported to %s.",Player_Name_From_ID(ID).c_str(),Player_Name_From_ID(target).c_str());
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

class Vault_Display : public Stewie_ConsoleCommandClass {
public:
	const char* Get_Name() { return "v_vault"; }
	const char* Get_Help() {
		if (ServerInit == false) { return ""; }
		ConsoleOut("[VGM] V_VAULT <name> - Displays all medals for a specific player.");
		return "";
	}
	void Activate(const char* input) {
		if (numtok(input,32) >= 1) { 
			const char *nick = strtok((char *)input," ");
			if (nick && strlen(nick) > 0) {
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
					ConsoleOut("[VGM] Displaying %s Achievements:",nick);
					int TotalMedals = 0;
					for (unsigned int j = 0; j < (int)vMManager->vLAST; j++) {
						char m[16],f[128];
						sprintf(m,"Medal%d",j);
						sprintf(f,"vgm\\medals_%s.ini",nick);
						int Medals = getProfileInt("Medals",(const char*)m,0,(const char*)f);
						TotalMedals += Medals;
						if (Medals > 0) { ConsoleOut("[VGM] %s times Achieved: %d",AwardNames[j],Medals); }
					}
					if (TotalMedals == 0) { ConsoleOut("[VGM] %s has no achievements.",nick); }
					else { ConsoleOut("[VGM] Total %d achievements.",TotalMedals); }
				} else {
					ConsoleOut("[VGM] Server does not record achievements for %s.",nick);
				}
			} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
		} else { ConsoleOut("[VGM] Invalid use of command %s.",To_Uppercase(Get_Name()).c_str()); }
	}
};

void ConsoleCommandHook(Stewie_DynamicVectorClass<Stewie_ConsoleCommandClass*>& Functions) {
	Stewie_ConsoleCommandClass* cf;
	Functions.Add(cf = new Ban_Hash);
	Functions.Add(cf = new Buildings);
	Functions.Add(cf = new CharacterChg_Pl);
	Functions.Add(cf = new Create_Veh);
	Functions.Add(cf = new Credit_Pl);
	Functions.Add(cf = new Credit2_Pl);
	//Functions.Add(cf = new Emergency_Shutdown);
	Functions.Add(cf = new GetPos_Pl);
	Functions.Add(cf = new Get_Serial);
	Functions.Add(cf = new Grant_Pl);
	Functions.Add(cf = new Kill_Pl);
	Functions.Add(cf = new Move_Pl);
	Functions.Add(cf = new Mute_Pl);
	Functions.Add(cf = new PInfo);
	Functions.Add(cf = new PLimit);
	Functions.Add(cf = new QSpec_Pl);
	Functions.Add(cf = new Rehash);
	Functions.Add(cf = new Score_Pl);
	Functions.Add(cf = new Score2_Pl);
	Functions.Add(cf = new SetFPS);
	Functions.Add(cf = new SetPos_Pl);
	Functions.Add(cf = new Spawn_Veh);
	Functions.Add(cf = new Spec_Pl);
	Functions.Add(cf = new Tele_Pl);
	Functions.Add(cf = new Vault_Display);
}
ASMHackCall ConsoleCommandHack(0x000000, &ConsoleCommandHook); // For security and licensing purposes, an address has been hidden from this line
