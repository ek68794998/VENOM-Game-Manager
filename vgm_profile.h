#ifndef	__PROFILE_H
#define	__PROFILE_H

int getProfileInt(const char *section, const char *entry, int defaultInt, const char *fileName);
float getProfileFloat(const char *section, const char *entry, float defaultInt, const char *fileName);
int getProfileString( const char *section, const char *entry, const char *defaultString, char *buffer, int bufLen, const char *fileName);
int	writeProfileString(	const char *section, const char	*entry, const char *string, const char *fileName);
int	deleteProfileString(const char	*section, const char *entry, const char *string, const char *fileName);

#endif