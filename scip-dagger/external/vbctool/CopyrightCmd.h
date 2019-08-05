/*****************************************************************

            Filename    :  CopyrightCmd.h

	    Version     :  08.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Pops up a window informing the user about
			   the Tree Interface.
			   Is a derived class from NoUndoCmd.
			   
******************************************************************/

#ifndef COPYRIGHTCMD_H
#define COPYRIGHTCMD_H


#include "NoUndoCmd.h"
#include "CopyrightDiag.h"
#include <iostream>

class CopyrightCmd : public NoUndoCmd{

public:

   CopyrightCmd(char*,int);
   ~CopyrightCmd();


protected:

   void doit();

   void cleanup();

private:

   CopyrightDiag*   _copyrightDiag;

};

#endif
