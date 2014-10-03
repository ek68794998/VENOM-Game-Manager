#pragma once

#include "vgm_engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class vLoggerClass;

extern vLoggerClass *vLogger;

enum vLoggerType {
	vRENLOG = 1,
	vSSAOW,
	vVGM,
	vGAMELOG,
	vCHEAT
};

class vLoggerClass {
public:
	void Log(vLoggerType type, const char *header, char *string, ...);
};