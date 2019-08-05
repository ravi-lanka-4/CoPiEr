/*****************************************************************

            Filename    :  StandardInputCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Reads a tree from standard input and
	                   prints it on the screen. Especially
			   useful for piping the uotput of a
			   program to the TreeInterface.
			   Is a derived class from NoUndoCmd.h.
			   
******************************************************************/

#ifndef STANDARDINPUTCMD_H
#define STANDARDINPUTCMD_H


#include "NoUndoCmd.h"
#include "GFE.inc"
#include <iostream>

class TreeInterface;

class StandardInputCmd : public NoUndoCmd{
friend class TreeInterface;
public:

   StandardInputCmd(char*,int);
   void Interrupt() { _interrupted = TRUE;  }
   void Continue()  { _interrupted = FALSE; }

protected:

   void doit();

private:

   Boolean _interrupted;

   void standardIn();


   // The following functions are necessary, because there is a bug
   // in using the "std::cin"-stream with motif or an X-application in LINUX.
   // These functions manage the destinction between LINUX and the
   // other supported operating systems until the bug is fixed.
   
   int      _peek();
   std::istream &_getline(char *str, int lng, char trenn = '\n');
   int      _get();

};

#endif
