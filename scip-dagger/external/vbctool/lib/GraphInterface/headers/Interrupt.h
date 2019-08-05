/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Interrupt.h
//
//      Version  :      22.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////
#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "InterruptBasePopup.h"


class Interrupt : public InterruptBasePopup
{
   private:

      Boolean _interrupted;

   protected:

      void createActionArea();
      void createControlArea() {}

      void ok();

   public:

      Interrupt();

      void post();

      Boolean interrupted()
      {
	 dispatchEvents();
	 Boolean i = _interrupted;
	 _interrupted = False;
	 return i;
      }
};

extern Interrupt* theInterrupt;

#endif
