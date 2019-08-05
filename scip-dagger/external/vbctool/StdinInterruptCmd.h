/*****************************************************************

            Filename    :  StdinInterruptCmd.h

	    Version     :  08.1997

	    Author      :  Joachim Kupke

	    Language    :  C++

            Purpose     :  Interrupts the standard input
			   
******************************************************************/

#ifndef STDININTERRUPTCMD_H
#define STDININTERRUPTCMD_H


#include "NoUndoCmd.h"

class StdinInterruptCmd : public NoUndoCmd{

public:

   StdinInterruptCmd(char*,int);

protected:

   void doit();

private:



};
#endif
