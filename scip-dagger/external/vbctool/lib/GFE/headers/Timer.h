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
//			interval sets the Timer-Interval in Milliseconds
//			heavily depend on Timer Class described by D.A.Young, page 76
//
//
////////////////////////////////////////////////////////////////////////

#ifndef TIMER_H
#define TIMER_H

#include <Xm/Xm.h>

class Timer
{
   private:

      static void tickCallback(XtPointer, XtIntervalId* );

      void tick();

      int _counter;
      int _interval;	   // _interval milliseconds
      XtIntervalId _id;
      XtAppContext _app;

   protected:

      virtual Boolean action() = 0; // returns TRUE if it should be activated
				    // once again

   public:

      Timer (int interval);   // interval milliseconds
      ~Timer() { stop(); }

      void start();
      void stop();
      float elapsedTime() { return ( (float) _counter * _interval / 1000.0); }
};

#endif
