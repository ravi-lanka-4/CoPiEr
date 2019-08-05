/*****************************************************************

            Filename    :  SwitchToSearcherCmd.cc

	    Version     :  01.1995

	    Author      :  Sebastian Leipert

	    Language    :  C++

            Purpose     :  See .h File.
	    
******************************************************************/

#include "SwitchToSearcherCmd.h"



/*****************************************************************
                 SwitchToSearcherCmd
******************************************************************/


SwitchToSearcherCmd::SwitchToSearcherCmd(char* name,int active,DrawArea* da)
        : NoUndoCmd(name,active)

   // Constructor
{
   _drawArea = da;

                                     // Create the Cursor that should
				     // appear when the user switches
				     // to Browser Mode.
   int cursorshape = XC_crosshair;
   _cursor = XCreateFontCursor(_drawArea->display(),cursorshape);
}



/*****************************************************************
                 ~SwitchToSearcherCmd
******************************************************************/


SwitchToSearcherCmd::~SwitchToSearcherCmd()

   // Destructor
{
   XFreeCursor(_drawArea->display(), _cursor);

   XDefineCursor(_drawArea->display(), _drawArea->window(), (Cursor) NULL);
                                     // NULL switches back to default cursor.
}




/*****************************************************************
                    doit
******************************************************************/


void SwitchToSearcherCmd::doit()

   // Procedure called when the button Broser Mode
   // in the menue Bar is pressed.
{
   XDefineCursor(_drawArea->display(), _drawArea->window(), _cursor);
                                     // When the user switches to Browser Mode
				     // by pushing the button in the menue-bar
				     // the Cursor is changed.
}



/*****************************************************************
                   cleanup
******************************************************************/


void SwitchToSearcherCmd::cleanup()

   // Procedure called when the button Normal Mode
   // in the menue Bar is pressed.   
{
   XDefineCursor(_drawArea->display(), _drawArea->window(), (Cursor) NULL);
                                     // NULL switches back to default cursor.
}
