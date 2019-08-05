/////////////////////////////////////////////////////////////////////////
//
//
// Filename :  SwitchToZoomingCmd.cc
//
// Version  :  18.02.94
//
// Author   :  Martin Diehl & Joachim Kupke
//
// Language :  C++
//
// Purpose  :  defines Cursor, etc.
// 
//
////////////////////////////////////////////////////////////////////////   

#include "SwitchToZoomingCmd.h"

SwitchToZoomingCmd::SwitchToZoomingCmd(char *name,
				       int active) : Cmd(name, active)
{
   _hasUndo = TRUE;
   
   // create the Cursor that should appear when the user
   // switches to zooming mode
   int cursorShape = XC_sizing;
   _cursor = XCreateFontCursor(theDrawArea->display(), cursorShape);
}

SwitchToZoomingCmd::~SwitchToZoomingCmd()
{
   XFreeCursor(theDrawArea->display(), _cursor);
   XDefineCursor(theDrawArea->display(),
		 theDrawArea->window(), (Cursor) NULL);   // NULL switches back to
					// default cursor.
}

void SwitchToZoomingCmd::doit()
{
   	XDefineCursor(theDrawArea->display(), theDrawArea->window(), _cursor);
}

void SwitchToZoomingCmd::undoit()
{
   	XDefineCursor(theDrawArea->display(),
	        	 theDrawArea->window(), (Cursor) NULL);   // NULL switches back to
					// default cursor.
}
