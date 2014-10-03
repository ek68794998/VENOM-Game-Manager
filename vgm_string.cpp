#include "vgm_string.h"

#pragma warning (disable: 4244)

std::string WCharToStr(const wchar_t *wcs) {
	int length = wcslen(wcs);
	char *text = new char[length+1];
	wcstombs(text,wcs,length+1);
	std::string Retn = text;
	delete[] text;
	return Retn;
}
int numtok(char *string, int seps) {
	int tokens = 1;
	for (int i = 0; i < (int)strlen(string); i++) {
		if (string[i] == seps) { tokens++; }
	}
	return tokens;
}
int numtok(const char *string, int seps) {
	char* in = new char[strlen(string) + 1];
	strcpy(in,string);
	int tokens = numtok(in,seps);
	delete[] in;
	return tokens;
}
std::string To_Lowercase(char *in) {
	for (int i = 0; i < (int)strlen(in); i++) {
		if (in[i] >= 'A' && in[i] <= 'Z') { in[i] = tolower(in[i]); }
	}
	std::string Retn = in;
	return Retn;
}
std::string To_Lowercase(const char *string) {
	char* in = new char[strlen(string) + 1];
	strcpy(in,string);
	for (int i = 0; i < (int)strlen(in); i++) {
		if (in[i] >= 'a' && in[i] <= 'z') { in[i] = tolower(in[i]); }
	}
	std::string Retn = in;
	delete[] in;
	return Retn;
}
std::string To_Uppercase(char *in) {
	for (int i = 0; i < (int)strlen(in); i++) {
		if (in[i] >= 'A' && in[i] <= 'Z') { in[i] = toupper(in[i]); }
	}
	std::string Retn = in;
	return Retn;
}
std::string To_Uppercase(const char *string) {
	char* in = new char[strlen(string) + 1];
	strcpy(in,string);
	for (int i = 0; i < (int)strlen(in); i++) {
		if (in[i] >= 'a' && in[i] <= 'z') { in[i] = toupper(in[i]); }
	}
	std::string Retn = in;
	delete[] in;
	return Retn;
}

int wildcmp(const char *wild, const char *string) {
	const char *cp = NULL, *mp = NULL;
	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}
	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;
		} else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}
	while (*wild == '*') {
		wild++;
	}
	return !*wild;
}

vTokenParser::vTokenParser() {
	Reset();
	deleted = false;
}
vTokenParser::vTokenParser(char *string) {
	Reset();
	deleted = false;
	this->Set(string);
}
void vTokenParser::Parse(const char *separators) {
	std::string NewStr(String);
	separator = std::string(separators);
	int find = NewStr.find(separators);
	int iterations = 0;
	while (find >= 0) {
		char *Buffer = new char[512];
		memset(Buffer,'\0',512);
		NewStr.copy(Buffer,find);
		Addtok(Buffer);
		delete[] Buffer;
		NewStr.erase(0,find + 1);
		find = NewStr.find(separators);
		iterations++;
		if (iterations >= 300) { break; }
	}
	if (NewStr.length() > 0) {
		char *Buffer = new char[512];
		memset(Buffer,'\0',512);
		NewStr.copy(Buffer,NewStr.length());
		Addtok(Buffer);
		delete[] Buffer;
	}
}
void vTokenParser::Addtok(const char* String) {
	Tokens.push_back(std::string(String));
}
std::string vTokenParser::Gettok(int Token) {
	if (Token < 1 || Token > Numtok()) { return String; }
	return Tokens[Token - 1];
}
std::string vTokenParser::Gettok(int Start, int End) {
	if (Start > Numtok()) { return String; }
	if (Start <= 0) { Start = 1; }
	if (End == -1 || End > Numtok()) { End = Numtok(); }
	std::string m("");
	for (int i = Start; i <= End; i++) {
		if (i < 1 || i > Numtok()) { continue; }
		if (strlen(m.c_str()) > 0) { m.append(separator.c_str()); }
		m.append(Tokens[i - 1].c_str());
	}
	return m;
}
void vTokenParser::Reset() {
	String = std::string("");
	Tokens.clear();
}
void vTokenParser::Delete() {
	if (!this || deleted) { return; }
	deleted = true;
	Reset();
	delete this;
}
vTokenParser::~vTokenParser() {
	Tokens.clear();
};

Stewie_WideStringClass::Stewie_WideStringClass(const wchar_t* string) {
	m_Buffer = *m_EmptyString;
	if (string) {
		int x = wcslen(string);
		m_Buffer = Allocate_Buffer(x+1);
		memcpy(m_Buffer,string,(x+1)*sizeof(wchar_t));
	}
}
void Stewie_WideStringClass::Free_String() {
	if (!Is_Empty()) {
		bool temp = false;
		for (int i = 0;i < 3;i++) {
			if (m_Buffer == m_ResTempPtr[i]) {
				m_TempMutex->Lock();
				m_Buffer[0] = 0;
				m_FreeTempPtr[i] = m_Buffer;
				m_ResTempPtr[i] = 0;
				(*m_UsedTempStringCount)--;
				temp = true;
				m_TempMutex->Unlock();
				break;
			}
		}
		if (!temp) {
			Stewie_OperatorDelete(static_cast<void*>(Get_Header()));
		}
		m_Buffer = *m_EmptyString;
	}
}
Stewie_WideStringClass::~Stewie_WideStringClass() {
	Free_String();
}
