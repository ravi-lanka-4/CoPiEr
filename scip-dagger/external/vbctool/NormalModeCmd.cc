/*****************************************************************

            Filename    :  NormalModeCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  Manages the Command in the menue Bar

******************************************************************/


#include "NormalModeCmd.h"

NormalModeCmd::NormalModeCmd(char* name,int active,DrawArea* da)
      : NoUndoCmd(name,active)
{
   _drawArea = da;
}


NormalModeCmd::~NormalModeCmd()
{
}


void NormalModeCmd::doit()
{
   XDefineCursor(_drawArea->display(),_drawArea->window(), (Cursor) NULL);
                                     // NULL switches back to default cursor.
}


     
