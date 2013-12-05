#include "stdafx.h"
#include "MicrowaveStateMachine.h"

typedef void (MicrowaveStateMachine::* ACTION)();

MicrowaveStateMachine::MicrowaveStateMachine()
{
	timerValue = 0;
	callbackFunc = NULL;
	currentState = IDLE_CLOSE_DOOR; 
	callback = new CallbackTimer<MicrowaveStateMachine, event>(this, cs, &MicrowaveStateMachine::handleEvent, TIME_OUT);
	timer = new Timer<CallbackTimer<MicrowaveStateMachine, event>>(callback);
	
	addTransition(IDLE_CLOSE_DOOR, OPEN_DOOR, IDLE_OPEN_DOOR, NULL);
	addTransition(IDLE_OPEN_DOOR, CLOSE_DOOR, IDLE_CLOSE_DOOR, NULL);

	addTransition(IDLE_CLOSE_DOOR, SET_TIMER, TIMER_CLOSE_DOOR, getAction(SET_TIMER_ACTION));
	addTransition(IDLE_OPEN_DOOR, SET_TIMER, TIMER_OPEN_DOOR, getAction(SET_TIMER_ACTION));

	addTransition(TIMER_CLOSE_DOOR, SET_TIMER, TIMER_CLOSE_DOOR, getAction(SET_TIMER_ACTION));
	addTransition(TIMER_OPEN_DOOR, SET_TIMER, TIMER_OPEN_DOOR, getAction(SET_TIMER_ACTION));

	addTransition(TIMER_CLOSE_DOOR, OPEN_DOOR, TIMER_OPEN_DOOR, NULL);
	addTransition(TIMER_OPEN_DOOR, CLOSE_DOOR, TIMER_CLOSE_DOOR, NULL);

	addTransition(TIMER_CLOSE_DOOR, START, COOKING, getAction(START_ACTION));
	addTransition(COOKING, OPEN_DOOR, TIMER_OPEN_DOOR, getAction(INTERRUPT_COOKING_ACTION));
	addTransition(COOKING, SET_TIMER, TIMER_CLOSE_DOOR, getAction(INTERRUPT_COOKING_ACTION));

	addTransition(COOKING, TIME_OUT, IDLE_CLOSE_DOOR, getAction(END_COOKING_ACTION));
}

void MicrowaveStateMachine::addTransition(state s, event e, state nextState, ACTION action)
{
	transMap.insert(std::pair<TransMapKey, Trans>(TransMapKey(s, e), Trans(nextState, action)));
}

ACTION MicrowaveStateMachine::getAction(action act)
{
	switch (act)
	{
		case SET_TIMER_ACTION:
			return &MicrowaveStateMachine::setTimerAction;
		case START_ACTION:
			return &MicrowaveStateMachine::startAction;
		case INTERRUPT_COOKING_ACTION:
			return &MicrowaveStateMachine::interruptCookingAction;
		case END_COOKING_ACTION:
			return &MicrowaveStateMachine::endCookingAction;
	}
}

void MicrowaveStateMachine::handleEvent(event e)
{
	std::map<TransMapKey, Trans>::iterator it;
	TransMapKey key(currentState, e);
	if ((it = transMap.find(key)) != transMap.end())
	{
		Trans tr = it->second;
		currentState = tr.getNextState();
		if(tr.getAction()) (this->*tr.getAction())();
	} else throw ImpossibleEventException();
}

void MicrowaveStateMachine::openDoor()
{
	Lock l(cs);
	handleEvent(OPEN_DOOR);
}

void MicrowaveStateMachine::closeDoor() 
{
	Lock l(cs);
	handleEvent(CLOSE_DOOR);
}

void MicrowaveStateMachine::setTimer(unsigned char timer)
{
	Lock l(cs);
	timerValue = timer;
	handleEvent(SET_TIMER);
}

void MicrowaveStateMachine::start() 
{
	Lock l(cs);
	handleEvent(START);
}

std::wstring MicrowaveStateMachine::getCurrentStateName()
{
	switch (currentState)
	{
		case IDLE_CLOSE_DOOR:
			return L"IDLE_CLOSE_DOOR";
		case IDLE_OPEN_DOOR:
			return L"IDLE_OPEN_DOOR";
		case TIMER_OPEN_DOOR:
			return L"TIMER_OPEN_DOOR";
		case TIMER_CLOSE_DOOR:
			return L"TIMER_CLOSE_DOOR";
		case COOKING:
			return L"COOKING";
	}
}

void MicrowaveStateMachine::setCallbackFunc(CallbackFunc* func)
{
	Lock l(cs);
	callbackFunc = func;
}

MicrowaveStateMachine::~MicrowaveStateMachine()
{
	Lock l(cs);
	transMap.clear();
	delete callback;
	delete timer;
}

template<class T, class Arg> void MicrowaveStateMachine::CallbackTimer<T, Arg>::operator()()
{
	Lock l(cs);
	(obj->*f)(arg);
}

template<class Func> Timer<Func>::Timer(Func* _func)
{
	func = _func;
	stopEvent = new Event(NULL, FALSE, FALSE, NULL);
	time = 0;
}

template<class Func> void Timer<Func>::threadFunctionCaller(void *p)
{
	Timer *pt;
	pt = reinterpret_cast<Timer*>(p);
	pt->threadFunction(pt->func);
}

template<class Func> void Timer<Func>::start() 
{
	stopEvent->reset();
	_beginthread(threadFunctionCaller, 0, reinterpret_cast<void*>(this) );
}

template<class Func> void Timer<Func>::stop() 
{ 
	Lock l(cs);
	stopEvent->set();
}

template<class Func> void Timer<Func>::threadFunction(Func *func) 
{
	DWORD res = WaitForSingleObject(stopEvent->getNativeEvent(), 1000 * time );
	
	if (res == WAIT_TIMEOUT && func != NULL) (*func)();
};