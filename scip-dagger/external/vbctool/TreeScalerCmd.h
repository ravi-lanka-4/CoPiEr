/*****************************************************************

            Filename    :  TreeScalerCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the command in the menue Bar
	                   for scaling the Tree.
			   A derived Class from NoUndoCmd.
			   
******************************************************************/


#ifndef TREESCALERCMD_H
#define TREESCALERCMD_H

#include "NoUndoCmd.h"
#include "TreeScalerDiag.h"


class TreeScalerCmd : public NoUndoCmd{

public:

   TreeScalerCmd(char*,int);
   ~TreeScalerCmd();

   void cleanup();
   void set_separation_values(int sibling,int level);
   
protected:

   void doit();

private:

   TreeScalerDiag*   _scalerDialog;
   
};

#endif
