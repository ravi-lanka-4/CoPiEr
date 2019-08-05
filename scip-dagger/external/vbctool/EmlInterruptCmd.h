/*****************************************************************

            Filename    :  EmlInterruptCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Interrupts the emulation of a tree
                           algorithm.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef EMLINTERRUPTCMD_H
#define EMLINTERRUPTCMD_H


#include "NoUndoCmd.h"

class EmlInterruptCmd : public NoUndoCmd{

public:

   EmlInterruptCmd(char*,int);

protected:

   void doit();

private:



};
#endif
