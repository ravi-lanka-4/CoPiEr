/*****************************************************************

            Filename    :  EmulationCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Emulates any kind of Tree Algorithm.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef EMULATIONCMD_H
#define EMULATIONCMD_H


#include "NoUndoCmd.h"



class EmulationCmd : public NoUndoCmd{

public:

   EmulationCmd(char*, int);
   ~EmulationCmd();

   
   void setFileName(char* filename);
   void Interrupt();
   void Stop();
   void Continue();
   void StepWise(int boolean){ STEPWISE = boolean;
			       STEPWISECHANGED = TRUE - boolean; };
   void completeTree(int boolean);
   

   int  maxHour(){ return maxhour; };
   int  maxMin(){ return maxmin; };
   int  maxSec(){ return maxsec; };
   int  maxMilli(){ return maxmilli; };

   void  getRemainingTime(int*,int*,int*,int*);
   void  interpretScaler(int,int,int,int);
   void  nextStep();


protected:

   void doit();

private:

   char      _FileName[INFOSIZE];
   
   int  hour;
   int  min;
   int  sec;
   int  millisec;
   int  maxhour;
   int  maxmin;
   int  maxsec;
   int  maxmilli;
   int  position;

   int  STOPPED;
   int  INTERRUPT;
   int  STEPWISE;
   int  STEPWISECHANGED;
   int  COMPLETE;
   int  MODECOMPLETE;



   unsigned long _time;
   unsigned long _totaltime;
   unsigned long _lasttime;
   unsigned long _interval;
   unsigned long _interruptInterval;

   double _scaler;
   
   XtIntervalId  _timerID;
   XtIntervalId  _interruptID;
   XtAppContext  _app;

   void nextAction();
   void installTimer(std::istream &);

   static void actionCallback(XtPointer, XtIntervalId *);
   static void interruptCallback(XtPointer, XtIntervalId *);

   void interruptAction();
   void installInterruptTimer();
   
   void computeTree(int filepos);
};

#endif
