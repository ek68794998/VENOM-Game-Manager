#include "vgm_scripts.h"

bool Script_Attach(Stewie_ScriptableGameObj *obj, const char *Name, const char *Params, bool Once) {
	if (!obj || !Name) { return false; }
	if (Once && Script_Attached(obj,Name)) { return true; }
	Stewie_GameObjObserverClass *Observer = Stewie_ScriptManager::Create_Script(Name);
	if (Observer) {
		((Stewie_ScriptImpClass *)Observer)->Set_Parameters_String(Params);
		obj->Add_Observer(Observer);
		return true;
	}
	return false;
}
void Script_Remove(Stewie_ScriptableGameObj *obj, const char *Name) {
	if (!obj) { return; }
	Stewie_SimpleDynVecClass<Stewie_GameObjObserverClass *> *Observers = &(obj->Observers);
	int x = Observers->Count();
	for (int i = 0; i < x; i++) {
		if (!stricmp((*Observers)[i]->Get_Name(),Name)) { obj->Remove_Observer(obj->Observers[i]); }
	}
}
void Script_Remove_Duplicate(Stewie_ScriptableGameObj *obj, const char *Name) {
	if (!obj) { return; }
	bool Remove = false;
	Stewie_SimpleDynVecClass<Stewie_GameObjObserverClass *> *Observers = &(obj->Observers);
	int x = Observers->Count();
	for (int i = 0; i < x; i++) {
		if (Remove == true) { obj->Remove_Observer(obj->Observers[i]); }
		if (!stricmp((*Observers)[i]->Get_Name(),Name)) { Remove = true; }
	}
}
void Script_Remove_All(Stewie_ScriptableGameObj *obj) {
	if (!obj) { return; }
	obj->Remove_All_Observers();
}
bool Script_Attached(Stewie_ScriptableGameObj *obj, const char *Name) {
	return (Script_Count(obj,Name) > 0);
}
int Script_Count(Stewie_ScriptableGameObj *obj, const char *Name) {
	if (!obj) { return 0; }
	int Count = 0;
	Stewie_SimpleDynVecClass<Stewie_GameObjObserverClass *> *Observers = &(obj->Observers);
	int x = Observers->Count();
	for (int i = 0; i < x; i++) {
		if (!stricmp((*Observers)[i]->Get_Name(),Name)) { Count++; }
	}
	return Count;
}

/////////////////////////////////
// Default Westwood stuff...
/////////////////////////////////
Stewie_ScriptFactory *Stewie_ScriptRegistrar::mScriptFactories;
Stewie_ScriptImpClass *Stewie_ScriptRegistrar::CreateScript(const char *scriptName) {
	if (!scriptName) { return NULL; }
	Stewie_ScriptFactory *x = mScriptFactories;
	while (x) {
		if (!stricmp(x->GetName(),scriptName)) {
			return x->Create();
		}
		x = x->GetNext();
	}
	return NULL;
}
void Stewie_ScriptRegistrar::UnregisterScript(Stewie_ScriptFactory *ptr) {
	Stewie_ScriptFactory *x = mScriptFactories;
	Stewie_ScriptFactory *y;
	Stewie_ScriptFactory *z = 0;
	while (x) {
		y = x->GetNext();
		if (x == ptr) {
			if (!z) {
				mScriptFactories = y;
			} else {
				z->SetNext(y);
			}
		}
		z = x;
		x = y;
	}
}
void Stewie_ScriptRegistrar::RegisterScript(Stewie_ScriptFactory *ptr) {
	ptr->SetNext(mScriptFactories);
	mScriptFactories = ptr;
}
Stewie_ScriptFactory::Stewie_ScriptFactory(char *n, char *p) {
	name = n;
	params = p;
	next = 0;
	Stewie_ScriptRegistrar::RegisterScript(this);
}
Stewie_ScriptFactory *Stewie_ScriptFactory::GetNext() {
	return next;
}
const char *Stewie_ScriptFactory::GetName() {
	return (const char *)name;
}
const char *Stewie_ScriptFactory::GetParamDescription() {
	return (const char *)params;
}
Stewie_ScriptFactory::~Stewie_ScriptFactory() {
	Stewie_ScriptRegistrar::UnregisterScript(this);
	name = 0;
	params = 0;
}
void Stewie_ScriptFactory::SetNext(Stewie_ScriptFactory *link) {
	next = link;
}
Stewie_ScriptImpClass::Stewie_ScriptImpClass() {
	ID = 0;
	Attached = 0;
	mArgC = 0;
	mArgV = 0;
	SerializeInfo = 0;
}
const char *Stewie_ScriptImpClass::Get_Name() {
	return Factory->GetName();
}
void Stewie_ScriptImpClass::Attach(Stewie_ScriptableGameObj *newobj) {
	Attached = newobj;
}
void Stewie_ScriptImpClass::Destroy_Script() {
	Script_Remove(Owner(),this->Get_Name());
}
void Stewie_ScriptImpClass::Detach(Stewie_ScriptableGameObj *obj) {
	Attached = 0;
	Destroy_Script();
}
void Stewie_ScriptImpClass::Created(Stewie_ScriptableGameObj *obj) {
}
void Stewie_ScriptImpClass::Destroyed(Stewie_ScriptableGameObj *obj) {
}
void Stewie_ScriptImpClass::Damaged(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *damager, float damage) {
}
void Stewie_ScriptImpClass::Killed(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *shooter) {
}
void Stewie_ScriptImpClass::Sound_Heard(Stewie_ScriptableGameObj *obj, const Stewie_CombatSound & sound) {
}
void Stewie_ScriptImpClass::Custom(Stewie_ScriptableGameObj *obj, int message, int param, Stewie_ScriptableGameObj *sender) {
}
void Stewie_ScriptImpClass::Action_Complete(Stewie_ScriptableGameObj *obj, int action, Stewie_ActionCompleteReason reason) {
}
void Stewie_ScriptImpClass::Enemy_Seen(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *seen) {
}
void Stewie_ScriptImpClass::Timer_Expired(Stewie_ScriptableGameObj *obj, int number) {
}
void Stewie_ScriptImpClass::Animation_Complete(Stewie_ScriptableGameObj *obj, const char *anim) {
}
void Stewie_ScriptImpClass::Poked(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *poker) {
}
void Stewie_ScriptImpClass::Entered(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *enter) {
}
void Stewie_ScriptImpClass::Exited(Stewie_ScriptableGameObj *obj, Stewie_ScriptableGameObj *exit) {
}
Stewie_ScriptableGameObj *Stewie_ScriptImpClass::Owner() {
	return Attached;
}
Stewie_ScriptableGameObj **Stewie_ScriptImpClass::Get_Owner_Ptr() {
	return &Attached;
}
void Stewie_ScriptImpClass::Set_Parameters_String(const char *params) {
	if (!params || !*params) { return; }
	Get_Name();
	char *a = newstr(params);
	if (a[strlen(a) - 1] == '\n') { a[strlen(a) - 1] = 0; }
	int b = 1;
	{
		for (const char *r = a; *r;) {
			if (*r++ == ',') { b++; }
		}
	}
	Clear_Parameters();
	mArgC = b;
	mArgV = new char *[b];
	memset(mArgV,0,b << 2);
	memcpy(a,params,strlen(params));
	int i = 0;
	for (char *r = a; *r; i++) {
		char *r2 = r;
		while (*r2 && *r2 != ',') { r2++; }
		if (*r2) { *r2++ = 0; }
		Set_Parameter(i,r);
		r = r2;
	}
	if (mArgC != i) { // never true
		Set_Parameter(i,NULL);
	}
	delete[] a;
}

void Stewie_ScriptImpClass::Get_Parameters_String(char *params, unsigned int size) {
	*params = 0;
	unsigned int l = 0;
	for (int i = 0; i < mArgC; i++) {
		if (l >= size)
			return;
		if (i > 0) {
			l++;
			strcat(params,",");
		}
		const char *s = Get_Parameter(i);
		l += strlen(s);
		if (l > size)
			continue;
		strcat(params,s);
	}
}
void Stewie_ScriptImpClass::Register_Auto_Save_Variables() {
}
void Stewie_ScriptImpClass::Auto_Save_Variable(int num, int size, void *var) {
	Stewie_SerializeInfoStr *d,*d2;
	if (num < 0 || num > 0xff || size > 0xfa)
		return;
	d = SerializeInfo;
	while (d) {
		if (d->number == num) {
			return;
		}
		d = d->next;
	}
	d2 = new Stewie_SerializeInfoStr;
	if (!d2) {
		SerializeInfo = 0;
		return;
	}
	d = SerializeInfo;
	d2->number = num;
	d2->size = size;
	d2->data = var;
	d2->next = d;
	SerializeInfo = d2;
}
Stewie_ScriptImpClass::~Stewie_ScriptImpClass()  {
	while (SerializeInfo) {
		Stewie_SerializeInfoStr *s = SerializeInfo;
		SerializeInfo = s->next;
		delete s;
	}
	for (int i = 0;i < mArgC;i++) {
		delete[] mArgV[i];
	}
	delete[] mArgV;
}
void Stewie_ScriptImpClass::Clear_Parameters() {
	if (mArgV) {
		while (mArgC--) {
			if (mArgV[mArgC])
				delete[] mArgV[mArgC];
		}
		delete[] mArgV;
	}
	mArgV = 0;
	mArgC = 0;
}
const char *Stewie_ScriptImpClass::Get_Parameter(const char *v) {
	return Get_Parameter(Get_Parameter_Index(v));
}
int Stewie_ScriptImpClass::Get_Int_Parameter(const char *v) {
	return atoi(Get_Parameter(Get_Parameter_Index(v)));
}
float Stewie_ScriptImpClass::Get_Float_Parameter(const char *v) {
	return (float)atof(Get_Parameter(Get_Parameter_Index(v)));
}
char *sStrtrim(char *v) {
	if (v) {
		char *r = v;
		while (*r > 0 && *r < 0x21)
			r++;
		strcpy(v,r);
		r = v + strlen(v);
		while (r > v && r[-1] > 0 && r[-1] < 0x21)
			r--;
		*r = 0;
	}
	return v;
}
Stewie_Vector3 Stewie_ScriptImpClass::Get_Vector3_Parameter(const char *v) {
	return Get_Vector3_Parameter(Get_Parameter_Index(v));
}
int Stewie_ScriptImpClass::Get_Parameter_Index(const char *v) {
	char b[0x2000];
	strncpy(b,Factory->GetParamDescription(),0x1fff);
	b[0x1fff] = 0;
	char *r = b;
	for (int i = 0; *r ; i++) {
		char *r2 = r;
		while (*r2 && *r2 != ',')
			r2++;
		if (*r2)
			*r2++ = 0;
		char *r3 = strpbrk(r,"=:\n");
		if (r3)
			*r3 = 0;
		sStrtrim(r);
		if (!_stricmp(r,v))
			return i;
		r = r2;
	}
	return -1;
}
int Stewie_ScriptImpClass::Get_Parameter_Count() {
	return mArgC;
}
int Stewie_ScriptImpClass::Get_Int_Parameter(int v) {
	return atoi(Get_Parameter(v));
}
float Stewie_ScriptImpClass::Get_Float_Parameter(int v) {
	return (float)atof(Get_Parameter(v));
}
void Stewie_ScriptImpClass::Set_Parameter(int pos,char const *v) {
	if (mArgV[pos]) {
		delete[] mArgV[pos];
	}
	mArgV[pos] = newstr(v);
}
const char *Stewie_ScriptImpClass::Get_Parameter(int v) {
	if ((v >= 0) && (v < mArgC)) {
		return mArgV[v];
	} else {
		return NULL;
	}
}
Stewie_Vector3 Stewie_ScriptImpClass::Get_Vector3_Parameter(int v) {
	Stewie_Vector3 r;
	if (sscanf(Get_Parameter(v),"%f %f %f",&r.X,&r.Y,&r.Z) != 3) {
		return Stewie_Vector3();
	} else {
		return r;
	}
}
