/*****************************************************************

            Filename    :  SwitchToSearcherCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the Command in the menue Bar
	                   for browsing the tree. Activates the
			   Browser described in the class
			   TreeSearcherCmd.
			   A derived class from NoUndoCmd.

******************************************************************/


#ifndef SWITCHTOSEARCHERCMD_H
#define SWITCHTOSEARCHERCMD_H

#include "NoUndoCmd.h"
#include "def.glb"
#include "DrawArea.h"
#include <X11/cursorfont.h>

class SwitchToSearcherCmd : public NoUndoCmd{

public:

   SwitchToSearcherCmd(char* = NULL,int = 0,DrawArea* = NULL);
   ~SwitchToSearcherCmd();

   void cleanup();
   
protected:

   void doit();

private:

   Cursor            _cursor;
   DrawArea*         _drawArea;

};

#endif
