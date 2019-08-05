/*****************************************************************

            Filename    :  NormalModeCmd.h

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the Command in the menue Bar

******************************************************************/

#ifndef NORMALMODECMD_H
#define NORMALMODECMD_H

#include "NoUndoCmd.h"
#include "def.glb"
#include "DrawArea.h"
#include <X11/cursorfont.h>

class NormalModeCmd : public NoUndoCmd{

public:

   NormalModeCmd(char* = NULL,int = 0,DrawArea* = NULL);
   ~NormalModeCmd();

protected:

   void doit();

private:

   Cursor      _cursor;
   DrawArea*   _drawArea;


};

#endif

