#include "vgm_logging.h"

vLoggerClass *vLogger;

void vLoggerClass::Log(vLoggerType type, const char *header, char *string, ...) {
	vDate logdate;
	logdate.refreshdate();

	char filename[512];
	if (type == vLoggerType::vRENLOG) { sprintf(filename,"renlog_%s.txt",logdate.get_date_string()); }
	else if (type == vLoggerType::vSSAOW) { sprintf(filename,"ssaow_%s.txt",logdate.get_date_string()); }
	else if (type == vLoggerType::vVGM) { sprintf(filename,"%s_%s.txt",Config->vLogFile,logdate.get_date_string()); }
	else if (type == vLoggerType::vCHEAT) { sprintf(filename,"cheat_%s.txt",logdate.get_date_string()); }
	else if (type == vLoggerType::vGAMELOG) { sprintf(filename,"gamelog2.txt"); }
	else { return; }

	FILE *FP = fopen(filename,"a");
	if (!FP) { return; }

	char msgout[512];
	va_list args;
	va_start(args,string);
	vsnprintf(msgout,sizeof(msgout),string,args);
	va_end(args);

	char hours[5];
	char mins[5];
	char secs[5];
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	int hour = timeinfo->tm_hour;
	int min = timeinfo->tm_min;
	int sec = timeinfo->tm_sec;
	if (hour < 10) { sprintf(hours,"0%d",hour); }
	else { sprintf(hours,"%d",hour); }
	if (min < 10) { sprintf(mins,"0%d",min); }
	else { sprintf(mins,"%d",min); }
	if (sec < 10) { sprintf(secs,"0%d",sec); }
	else { sprintf(secs,"%d",sec); }

	char m[1024];
	if (type == vLoggerType::vRENLOG || type == vLoggerType::vGAMELOG || type == vLoggerType::vCHEAT) { sprintf(m,"[%s:%s:%s] %s\n",hours,mins,secs,msgout); }
	else if (type == vLoggerType::vSSAOW) { sprintf(m,"[%s:%s:%s] %s %s\n",hours,mins,secs,header,msgout); }
	else { sprintf(m,"(%s:%s:%s) %s %s\n",hours,mins,secs,header,msgout); }

	fputs(m,FP);
	fclose(FP);
}
