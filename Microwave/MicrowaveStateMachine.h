#include "stdafx.h"
#include <map>
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

class CallbackFunc
{
	public:
		virtual void operator()() = 0;
};

template<class E, class S, class T> class Transitions
{
	typedef void (T::* ACTION)();
	S state;
	ACTION action;

	public:
		Transitions(S _state, ACTION _action): state(_state), action(_action) {}
		S getNextState() {return state;}
		ACTION getAction() {return action;}
};

template<class F> class Timer
{
	unsigned int time;
	Event* stopEvent;
	F* func;
	
	static void threadFunctionCaller(void *p);

	void threadFunction(F* func);

	public:
		Timer(F* _func);
		void start();
		void stop();
		void set(unsigned int _time) {time = _time;}
		~Timer() { delete stopEvent; }
};

class ImpossibleEventException: public std::exception
{
	public:
		ImpossibleEventException() : exception() {}
};

class MicrowaveStateMachine
{
	enum event
	{
		SET_TIMER,
		OPEN_DOOR,
		CLOSE_DOOR,
		START,
		TIME_OUT
	};

	enum state
	{
		IDLE_OPEN_DOOR,
		IDLE_CLOSE_DOOR,
		TIMER_OPEN_DOOR,
		TIMER_CLOSE_DOOR,
		COOKING
	};
	
	enum action
	{
		SET_TIMER_ACTION,
		START_ACTION,
		INTERRUPT_COOKING_ACTION,
		END_COOKING_ACTION
	};

	class TransMapKey
	{
		state st;
		event ev;
		public:
			TransMapKey(state s, event e): st(s), ev(e) {}
			friend bool operator<(const TransMapKey& key, const TransMapKey& key1) {return (key.st < key1.st) || (key.st == key1.st) && (key.ev < key1.ev);}
			~TransMapKey() {}
 	};


	template<class T, class Arg> class CallbackTimer
	{
		typedef void (T::* F)(Arg);
		T* obj;
		Arg arg;
		F f;
		CriticalSection &cs;

		public:
			CallbackTimer(T* _obj, CriticalSection& _cs, F _f, Arg _arg): obj(_obj), f(_f), arg(_arg), cs(_cs) {}
			void operator()();
			~CallbackTimer() {}
	};

	typedef Transitions<event, state, MicrowaveStateMachine> Trans;
	typedef void (MicrowaveStateMachine::* ACTION)();

	unsigned char timerValue;
	state currentState;
	Timer<CallbackTimer<MicrowaveStateMachine, event>>* timer;
	CallbackTimer<MicrowaveStateMachine, event> *callback;
	std::map<TransMapKey, Trans> transMap;
	CallbackFunc* callbackFunc;
	CriticalSection cs;

	void handleEvent(event e);
	void setTimerAction() {timer->set(timerValue);}
	void startAction() {timer->start();}
	void interruptCookingAction() {timerValue = 0; timer->set(timerValue); timer->stop();}
	void endCookingAction() {timerValue = 0; timer->set(timerValue); if(callbackFunc) (*callbackFunc)();}

	void addTransition(state s, event e, state nextState, ACTION action);
	ACTION getAction(action act);

	public:

		void openDoor();
		void closeDoor();
		void setTimer(unsigned char timer);
		void start();
		void setCallbackFunc(CallbackFunc* func);
		std::wstring getCurrentStateName();
		unsigned char getTimerValue() {return timerValue;}
		MicrowaveStateMachine();
		~MicrowaveStateMachine();
};