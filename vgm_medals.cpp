#include "vgm_player.h"
#include "vgm_medals.h"

vMedalManager *vMManager;

char *AwardNames[37] = {
	{ "Kill Spree" },
	{ "Kill Frenzy" },
	{ "Running Riot" },
	{ "Untouchable" },
	{ "Headshot Spree (500)" },
	{ "Headshot Spree (1000)" },
	{ "Splatter Spree" },
	{ "Double Kill" },
	{ "Triple Kill" },
	{ "Overkill" },
	{ "Killtacular" },
	{ "Killtrocity" },
	{ "Killamanjaro" },
	{ "Killtastrophe" },
	{ "Killpocalypse" },
	{ "Killionaire" },
	{ "Ion Kill" },
	{ "Nuke Kill" },
	{ "Tiberium Kill" },
	{ "Timed Kill" },
	{ "Team Kill" },
	{ "Incineration" },
	{ "Kill Joy" },
	{ "Carjack" },
	{ "Hijack" },
	{ "Perfection" },
	{ "Rampage" },
	{ "Wrecking Ball" },
	{ "Dynamite Expert" },
	{ "Demolition Man" },
	{ "Atom Blaster" },
	{ "Destroyer of Worlds" },
	{ "Deconstructivist" },
	{ "Air Suicide" },
	{ "Harvester Suicide" },
	{ "Tiberium Suicide" },
	{ "C4 Junkie" }
};

void vMedalManager::Init() {
	vPManager->Clear_All_Medals();
}
void vMedalManager::Got_Medal(const char *nick, vMedalType Type, bool Notify) {
	if (Config->MedalsEnabled == false) { return; }
	vPlayer *p = vPManager->Get_Player(nick);
	if (p) {
		Stewie_cPlayer *player = Stewie_cPlayerManager::Find_Player(p->ID);
		if (!player || stricmp(WCharToStr(player->PlayerName.m_Buffer).c_str(),nick)) { Notify = false; }
		if (Notify) { PrivMsgColoredVA(p->ID,2,0,200,200,"[VGM] Received '%s' achievement.",AwardNames[Type]); }
		p->Medals[(int)Type]++;
	}
}
void vMedalManager::Got_Medal(int ID, vMedalType Type, bool Notify) {
	Got_Medal(Player_Name_From_ID(ID).c_str(),Type,Notify);
}
