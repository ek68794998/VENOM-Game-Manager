#pragma once
#pragma warning (disable: 4512)

#include "vgm_engine.h"
#include "vgm_net.h"
#include "engine.h"
#include "scripts.h"
#include "renegade.h"
#include "vgm_asm.h"

class HostnameFetcher;

extern std::vector<HostnameFetcher *> Resolvers;

class CriticalSection {
protected:
	CRITICAL_SECTION handle;

public:
	CriticalSection();
	CriticalSection(CRITICAL_SECTION& _handle);
	~CriticalSection();

	void enter();
	void leave();
};

class CriticalSectionEntry {
protected:
	CriticalSection& criticalSection;

public:
	CriticalSectionEntry(CriticalSection& _criticalSection);
	~CriticalSectionEntry();
};

class Event {
protected:
	HANDLE handle;

public:
	Event(bool initialState = false, bool manualReset = false);

	void wait(int timeout = INFINITE);
	void set();
	void reset();
	void pulse();
};

class Task {
public:
	virtual ~Task() {};
	virtual void perform() = 0;
};

class TaskList {
protected:
	list<Task*> tasks;
	CriticalSection lock;
	Event hasPendingTasks;

	bool isRunning;
	void execute();

public:
	TaskList();
	~TaskList();

	void add(Task& task);
	void start();
	void stop();
};

class Thread {
protected:
	HANDLE handle;
	int id;
	bool isRunning;

	static int __cdecl ThreadMain(LPVOID parameter);
	virtual void execute() = 0;

public:
	Thread();
	~Thread();
	virtual void start();
	virtual void stop();
	virtual bool wait(int timeout = INFINITE);
};



class HostnameFetcher : public Thread {
public:
	HostnameFetcher();
	~HostnameFetcher();

	in_addr ip;
	sockaddr_in Buffer;
	char host[NI_MAXHOST];
	int Resolved;
	vConnectionClass *HostConnection;

	virtual void execute() {
		Buffer.sin_family = AF_INET;
		Buffer.sin_addr = ip;
		Resolved = getnameinfo((sockaddr *)&Buffer,sizeof(sockaddr_in),host,NI_MAXHOST,NULL,0,0);
	};
};
