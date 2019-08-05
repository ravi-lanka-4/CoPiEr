/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      SingleStep.h
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
#ifndef SINGLESTEP_H
#define SINGLESTEP_H

#include "InterruptBasePopup.h"
#include "Interrupt.h"

enum Sign { NOTHING, NEXT, CONTINUE };

class SingleStep : public InterruptBasePopup
{
   private:

      Sign _sign;
      Boolean _interruptAllowed;

   protected:

      void createActionArea();
      void createControlArea() {}

      void apply();
      void cancel();

   public:

      SingleStep();
      ~SingleStep();

      void post(Boolean ia = True);

      Boolean interrupted()
      {
	 if (_sign == CONTINUE)
	    if (_interruptAllowed)
	       return theInterrupt->interrupted();
	    else return False;
	 
	 while(_sign == NOTHING) {
	    XtAppContext app = theApplication->appContext();
	    XEvent event;
	    XtAppNextEvent(app, &event);
	    XtDispatchEvent(&event);
	 }

	 if (_sign == NEXT) _sign = NOTHING;
	 else
	    if (_interruptAllowed)
	       theInterrupt->post();
	 return False;
      }

      Sign sign() { return _sign; }
};

extern SingleStep* theSingleStep;

#endif
