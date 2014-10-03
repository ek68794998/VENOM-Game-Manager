#include "vgm_bighead.h"

/* Due to licensing, portions of this file have been omitted. */

vBigheadObserver::vBigheadObserver() : head01(NULL), head02(NULL), head03(NULL), head04(NULL), head05(NULL), head06(NULL), head07(NULL), head08(NULL) {
}

const char* vBigheadObserver::Get_Name() {
	return "vObserver0xA1";
}

void vBigheadObserver::Created(Stewie_ScriptableGameObj *Observee) {
	/* Due to licensing, portions of this file have been omitted. */
}
void vBigheadObserver::Killed(Stewie_ScriptableGameObj *Observee, Stewie_ScriptableGameObj *Killer) {
	Remove_Observer(*Observee,"vObserver0xA1");
}
void vBigheadObserver::Detach(Stewie_ScriptableGameObj *Observee) {
	Destroyed(Observee);
	delete this;
}
void vBigheadObserver::Destroyed(Stewie_ScriptableGameObj *Observee) {
	/* Due to licensing, portions of this file have been omitted. */
}
/* Due to licensing, portions of this file have been omitted. */
