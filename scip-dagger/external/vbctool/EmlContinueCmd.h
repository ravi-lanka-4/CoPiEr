/*****************************************************************

            Filename    :  EmlContinueCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Continues the emulation of a tree
                           algorithm.
	                   A derived Class from NoUndoCmd.
			   
******************************************************************/

#ifndef EMLCONTINUECMD_H
#define EMLCONTINUECMD_H


#include "NoUndoCmd.h"

class EmlContinueCmd : public NoUndoCmd{

public:

   EmlContinueCmd(char*,int);

protected:

   void doit();

private:



};

#endif
