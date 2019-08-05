/*****************************************************************

            Filename    :  StdinContinueCmd.h

	    Version     :  08.1997

	    Author      :  Joachim Kupke

	    Language    :  C++

            Purpose     :  Continues the emulation of reading stdin.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef STDINCONTINUECMD_H
#define STDINCONTINUECMD_H


#include "NoUndoCmd.h"

class StdinContinueCmd : public NoUndoCmd{

public:

   StdinContinueCmd(char*,int);

protected:

   void doit();

private:



};

#endif
