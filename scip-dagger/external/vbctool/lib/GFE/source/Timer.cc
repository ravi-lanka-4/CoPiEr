/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Timer.h
//
//      Version  :      07.02.95
//
//      Author   :      Joachim Kupke / D. A. Young
//
//      Language :      C++
//
//      Purpose  :      Abstract Base-Class for Timer-interruption in Event-Queue
//                      interval sets the Timer-Interval in Milliseconds
//                      heavily depend on Timer Class described by D.A.Young, page 76
//
//
////////////////////////////////////////////////////////////////////////

#include "Timer.h"
#include "Application.h"

Timer::Timer(int interval)  // interval milliseconds
{
   _id = 0;
   _app = theApplication->appContext();
   _counter = 0;
   _interval = interval;
}

void Timer::start()
{
   _counter = 0;

   if (_id)
      XtRemoveTimeOut(_id);

   _id = XtAppAddTimeOut (_app,
			  _interval,
			  &Timer::tickCallback,
			  (XtPointer) this);
}

void Timer::stop()
{
   if (_id)
      XtRemoveTimeOut(_id);

   _id = 0;
}

void Timer::tickCallback(XtPointer clientData, XtIntervalId *)
{
   Timer *obj = (Timer*) clientData;

   obj->tick();
}

void Timer::tick()
{
   _counter++;

   if (action())
      _id = XtAppAddTimeOut (_app,
			     _interval,
			     &Timer::tickCallback,
			     (XtPointer) this);
}
