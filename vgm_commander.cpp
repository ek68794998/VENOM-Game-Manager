#include "vgm_commander.h"

int GDICommander;
int NodCommander;
int GDITeamPool;
int NODTeamPool;

int Get_Next_Commander(int team) {
	if (Is_Listed_Commander_Available(team) == false) { return 0; }
	int c = Config->TotalCommanders;

	char com[512],title[256];
	std::vector<int> available;
	for (int i = 0; i <= c; i++) {
		sprintf(title,"Commander%d",i);
		getProfileString("CommanderList",title,"*",com,512,"vgm_users.ini");
		if (stricmp((const char *)com,"*")) {
			GameObject *o = Player_GameObj_By_Name((const char *)com);
			if (!o || Commands->Get_ID(o) <= 0) { continue; }
			if (Commands->Get_Player_Type(o) == team) {
				available.push_back(Get_Player_ID(o));
			}
		} else { continue; }
	}

	if (available.size() <= 0) { return 0; }
	int r = Random(0,available.size(),true);
	return available[r];
}
bool Is_Listed_Commander_Available(int team) {
	int c = Config->TotalCommanders;
	if (c <= 0) { return false; }

	char com[512],title[256];
	sprintf(title,"Commander%d",c);
	getProfileString("CommanderList",title,"*",com,512,"vgm_users.ini");
	if (!stricmp((const char *)com,"*")) { return false; } // Max # Commander not found

	return true;
}
bool Has_Commander(int team) {
	int com = (int)(team == 1 ? GDICommander : NodCommander);
	if (com >= 1 && com <= 128) {
		if (Stewie_cPlayerManager::Is_Player_Present(com) == false) { return false; }
		Stewie_cPlayer *p = Stewie_cPlayerManager::Find_Player(com);
		if (!p || !p->IsInGame || !p->IsActive) { return false; }
		if (p->PlayerType != team) {
			Remove_Team_Commander(team);
			return false;
		}
		return true;
	}
	return false;
}
void Set_Team_Commander(int ID, int team) {
	if (ID > 0) {
		if (team == 1) { GDICommander = ID; }
		else { NodCommander = ID; }
		std::string newcom = Player_Name_From_ID(ID);
		HostMessage("[VGM] %s is now the %s Commander.",newcom.c_str(),Get_Translated_Team_Name(team).c_str());
		PrivMsgColoredVA(ID,2,0,200,0,"[VGM] The %s Team Pool currently has %d credits.",Get_Translated_Team_Name(team).c_str(),team == 1 ? GDITeamPool : NODTeamPool);
	} else {
		if (team == 1) { GDICommander = 0; }
		else { NodCommander = 0; }
	}
}
void Remove_Team_Commander(int team) {
	int i = 0;
	if (team == 1) { i = GDICommander; }
	else if (team == 0) { i = NodCommander; }
	if (i > 0) {
		HostMessage("[VGM] %s is no longer the %s Commander.",Player_Name_From_ID(i).c_str(),Get_Translated_Team_Name(team).c_str());
		Set_Team_Commander(0,team);
	}
}
bool Is_Commander(int ID, int team, bool potential) {
	return Is_Commander(Player_Name_From_ID(ID).c_str(),team,potential);
}
bool Is_Commander(const char *name, int team, bool potential) {
	if (potential == false) {
		if (Has_Commander(team)) {
			if (!name) { return false; }
			else if (team == 1 && GDICommander == Player_ID_From_Name(name)) { return true; }
			else if (team == 0 && NodCommander == Player_ID_From_Name(name)) { return true; }
		}
	} else {
		int c = Config->TotalCommanders;
		if (c <= 0) { return false; }

		char com[512],title[256];
		for (int i = 0; i <= c; i++) {
			sprintf(title,"Commander%d",i);
			getProfileString("CommanderList",title,"*",com,512,"vgm_users.ini");
			if (stricmp((const char *)com,"*")) {
				if (!stricmp(com,name)) { return true; }
			} else { continue; }
		}
	}
	return false;
}
