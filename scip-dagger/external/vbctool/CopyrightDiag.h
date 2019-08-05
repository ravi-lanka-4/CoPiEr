/*****************************************************************

            Filename    :  CopyrightDiag.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages a PopUp Menue, that shows
	                   the copyright of the Tree Interface
			   with some general information and
			   e-mail adresses.
			   A derived Class from PopupManager.
			   
******************************************************************/


#ifndef COPYRIGHTDIAG_H
#define COPYRIGHTDIAG_H

#include "GFE.inc"
#include "PopupManager.h"

class CopyrightDiag : public PopupManager {

public:

   CopyrightDiag(char*,Widget);
   ~CopyrightDiag();

   virtual void post();

   int    IsPosted(){ return _isPosted; };

protected:

   virtual void ok();

private:

   virtual void createControlArea();
   virtual void createActionArea();

   Widget         _copyrightControl;
   Widget	  _copyrightHeading;
   Widget         _copyrightDisplayOne;
   Widget	  _copyrightDisplayTwo;
   Widget         _copyrightFrame;
   Widget	  _copyrightAdress;

   
   int            _isPosted;

};

#endif

