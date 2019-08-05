/*****************************************************************

            Filename    :  EmlStopCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Stops the emulation of a tree
                           algorithm.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef EMLSTOPCMD_H
#define EMLSTOPCMD_H


#include "NoUndoCmd.h"

class EmlStopCmd : public NoUndoCmd{

public:

   EmlStopCmd(char*,int);

protected:

   void doit();

private:



};
#endif
