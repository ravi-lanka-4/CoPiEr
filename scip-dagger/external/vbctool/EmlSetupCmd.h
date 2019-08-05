/*****************************************************************

            Filename    :  EmlSetupCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the command in the menue Bar
	                   for scaling the Tree.
			   A derived Class from NoUndoCmd.
			   
******************************************************************/


#ifndef EMLSETUPCMD_H
#define EMLSETUPCMD_H

#include "NoUndoCmd.h"
#include "EmlSetupDiag.h"


class EmlSetupCmd : public NoUndoCmd{

public:

   EmlSetupCmd(char*,int);
   ~EmlSetupCmd();

   void newTime(){ _setupDialog->newTime(); };
   int isPosted(){ return _setupDialog->isPosted(); };
   
   
protected:

   void doit();

   void cleanup();
   
private:

   EmlSetupDiag*   _setupDialog;
   
};

#endif
