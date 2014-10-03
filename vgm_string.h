#pragma once

#include "vgm_engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

std::string WCharToStr(const wchar_t *wcs);
int numtok(char *string, int seps);
int numtok(const char *string, int seps);
std::string To_Lowercase(char *in);
std::string To_Lowercase(const char *string);
std::string To_Uppercase(char *in);
std::string To_Uppercase(const char *string);
int wildcmp(const char *wild, const char *string);

//#define isin(haystack, needle) strstr(To_Lowercase(haystack).c_str(),To_Lowercase(needle).c_str())
#define isin(haystack, needle) stristr(haystack,needle)

inline char* UnmangleString(const char* string, char Hash = 15) {
	int strlength = strlen(string);
	char* Newstring = new char[strlength + 1];
	
	for (int i = strlength - 1; i >= 0; i--) {
		Newstring[i] = string[i] ^ Hash;
	}
	Newstring[strlength] = '\0';
	return Newstring;
}

class vTokenParser {
protected:
	std::string String;
	std::vector<std::string> Tokens;
	std::string separator;
	bool deleted;

public:
	vTokenParser();
	vTokenParser(char *string);
	~vTokenParser();
	std::string Get() { return this->Gettok(0,-1); }
	void Set(char *Str) { Reset(); String = std::string(Str); };
	void Parse(const char *separators);
	void Addtok(const char* String);
	std::string Gettok(int Token);
	std::string Gettok(int Start, int End);
	int Numtok() { return Tokens.size(); };
	void Reset();
	void Delete();
};
