#include "vgm_dll.h"

HSZ ServiceName;
HSZ TopicName;
HSZ ItemName_Command;
DWORD Inst = 0;
HDDEDATA EXPENTRY FDSDDECallBack(UINT wType, UINT fmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD dwData1, DWORD dwData2) {
	switch (wType) {
	case XTYP_CONNECT:
		{
			if (!DdeCmpStringHandles(hsz1,TopicName) && !DdeCmpStringHandles(hsz2,ServiceName)) { return (HDDEDATA)TRUE; }
			else { return (HDDEDATA)FALSE; }
		}
	case XTYP_POKE:
		{
			unsigned char *DataRead = DdeAccessData(hData,NULL);
			datafromdde((LPTSTR)DataRead);
			return (HDDEDATA)DDE_FACK;
		}
	default:
		return (HDDEDATA)NULL;
	}
}

void DllInit() {
	char Name[30];
	getProfileString("Settings","DDEName","0",Name,30,"vgm.ini");
	if (stricmp(Name,"0")) {
		DdeInitialize(&Inst,FDSDDECallBack,APPCLASS_STANDARD,0);
		ServiceName = DdeCreateStringHandle(Inst,Name,0);
		TopicName = DdeCreateStringHandle(Inst,"FDSCommand",0);
		ItemName_Command = DdeCreateStringHandle(Inst,"Command",0);
		DdeNameService(Inst,ServiceName,0,DNS_REGISTER);
		ConsoleOut("%s DDE channel initialized",Name);
	}
}
