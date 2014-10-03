#include "vgm_threads.h"

std::vector<HostnameFetcher *> Resolvers;

CriticalSection::CriticalSection() {
	InitializeCriticalSection(&handle);
}
CriticalSection::CriticalSection(CRITICAL_SECTION& _handle) : handle(_handle) {
}
CriticalSection::~CriticalSection() {
	DeleteCriticalSection(&handle);
}
void CriticalSection::enter() {
	EnterCriticalSection(&handle);
}
void CriticalSection::leave() {
	LeaveCriticalSection(&handle);
}

CriticalSectionEntry::CriticalSectionEntry(CriticalSection& _criticalSection) : criticalSection(_criticalSection) {
	criticalSection.enter();
}
CriticalSectionEntry::~CriticalSectionEntry() {
	criticalSection.leave();
}

Event::Event(bool initialState, bool manualReset) {
	handle = CreateEvent(NULL, manualReset, initialState, NULL);
}
void Event::wait(int timeout) {
	WaitForSingleObject(handle, timeout);
}
void Event::set() {
	SetEvent(handle);
}
void Event::reset() {
	ResetEvent(handle);
}
void Event::pulse() {
	PulseEvent(handle);
}

TaskList::TaskList() : hasPendingTasks(false, true) {
	isRunning = false;
}
TaskList::~TaskList() {
}
void TaskList::add(Task& task) {
	CriticalSectionEntry lockEntry(lock);
	tasks.push_back(&task);
	hasPendingTasks.set();
}
void TaskList::execute() {
	CriticalSectionEntry lockEntry(lock);
	while (isRunning) {
		if (!tasks.empty()) {
			Task* task = tasks.front();
			tasks.pop_front();
			lock.leave();
			task->perform();
			delete task; // TODO: dirty.
			lock.enter();

			if (tasks.empty()) { hasPendingTasks.reset(); }
		}

		lock.leave();
		hasPendingTasks.wait();
		lock.enter();
	}
}
void TaskList::start() {
	isRunning = true;
	execute();
}
void TaskList::stop() {
	isRunning = false;
}

Thread::Thread() {
	isRunning = false;
}
Thread::~Thread() {
	stop();
	wait();
}
int __cdecl Thread::ThreadMain(LPVOID parameter) {
	((Thread*)parameter)->execute();
	return true;
}
void Thread::start() {
	isRunning = true;
	handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadMain, this, CREATE_SUSPENDED, (LPDWORD)&id);
	ResumeThread(handle);
}
void Thread::stop() {
	isRunning = false;
}
bool Thread::wait(int timeout) {
	return WaitForSingleObject(handle, timeout) == WAIT_OBJECT_0;
}

HostnameFetcher::HostnameFetcher() {
	Resolved = -1;
	Resolvers.push_back(this);
}
HostnameFetcher::~HostnameFetcher() {
	delete this->HostConnection;
}
