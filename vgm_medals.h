#pragma once

#include "vgm_engine.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vMedalManager;

extern vMedalManager *vMManager;
extern char *AwardNames[37];

class vMedalManager {
public:
	enum vMedalType {
		vKILLSPREE,
		vKILLFRENZY,
		vRUNNINGRIOT,
		vUNTOUCHABLE,
		v500SPREE,
		v1000SPREE,
		vSPLATTERSPREE,
		vDOUBLEKILL,
		vTRIPLEKILL,
		vOVERKILL,
		vKILLTACULAR,
		vKILLTROCITY,
		vKILLAMANJARO,
		vKILLTASTROPHE,
		vKILLPOCALYPSE,
		vKILLIONAIRE,
		vIONKILL,
		vNUKEKILL,
		vTIBERIUMKILL,
		vTIMEDKILL,
		vTEAMKILL,
		vINCINERATION,
		vKILLJOY,
		vCARJACKED,
		vHIJACKED,
		vPERFECTION,
		vRAMPAGE,
		vBLDGKILL1,
		vBLDGKILL2,
		vBLDGKILL3,
		vBLDGKILL4,
		vBLDGKILL5,
		vBLDGKILL6,
		vAIRSUICIDE,
		vHARVYSUICIDE,
		vTIBSUICIDE,
		vC4JUNKIE,
		vLAST
	};

	int KillSpreeTime;

	void Init();
	void Got_Medal(const char *nick, vMedalType Type, bool Notify);
	void Got_Medal(int ID, vMedalType Type, bool Notify);

	vMedalManager() {
		KillSpreeTime = 4;
	};
	~vMedalManager();
};