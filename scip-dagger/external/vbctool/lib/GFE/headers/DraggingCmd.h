/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   OpenZoomRectCmd.h
//
//	Version	 :   18.02.94
//
//	Author	 :   Martin Diehl & Joachim Kupke
//
//	Language :   C++ 
//
//	Purpose	 :   see .cc
// 
//
////////////////////////////////////////////////////////////////////////	
#ifndef DRAGGINGCMD_H
#define DRAGGINGCMD_H

#include "NoUndoCmd.h"
#include "DrawArea.h"
#include "Geometric.h"
#include <X11/cursorfont.h>


class DraggingCmd : public NoUndoCmd
{
   private:
   
      Point<int>  _oldPoint;
      Point<int>  _point;
      Boolean	  _first;
      Cursor	  _cursor;
   
   protected:
     virtual void doit();

   public:
   
      void executeWithPoint(Point<int> *point)
      { // Gets the point where the event occurs and
	// calls the execute() function.

	if (_first)
	{
	 _first = FALSE;
	 _oldPoint.newInit(point);
	 XDefineCursor(theDrawArea->display(), theDrawArea->window(), _cursor);
	}
	else
	{
	  _point.newInit(point);
	
	  execute();

	  _oldPoint.newInit(point);
	}

      }

      DraggingCmd(char*, int);
      ~DraggingCmd();
      
      void endOfDragging()
      {
	_first = TRUE;
        XDefineCursor(theDrawArea->display(),
		      theDrawArea->window(), (Cursor) NULL);   // NULL switches back to
			                            // default cursor. (JK96)
      }

      virtual const char *const className() {return ("DraggingCmd");}      
}; // DraggingCmd

#endif
