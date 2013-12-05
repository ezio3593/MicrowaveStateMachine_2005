#include "stdafx.h"
#include <process.h>
#include <windows.h>

class CriticalSection
{
	CRITICAL_SECTION cs;
	public:
		CriticalSection() { InitializeCriticalSection(&cs); }
		CRITICAL_SECTION& getNativeCriticalSection() { return cs; }
		~CriticalSection() { DeleteCriticalSection(&cs); }
};

class Lock
{
	CriticalSection& cs;
	public:
		Lock(CriticalSection& _cs) : cs(_cs) { EnterCriticalSection(&cs.getNativeCriticalSection()); }
		~Lock() { LeaveCriticalSection(&cs.getNativeCriticalSection()); }
};

class Event
{
	HANDLE hEvent;
	public:
		Event(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName)
		{ hEvent = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName); }
		BOOL set() { return SetEvent(hEvent); }
		BOOL reset() { return ResetEvent(hEvent); }
		HANDLE getNativeEvent() { return hEvent; }
		~Event() {}
};