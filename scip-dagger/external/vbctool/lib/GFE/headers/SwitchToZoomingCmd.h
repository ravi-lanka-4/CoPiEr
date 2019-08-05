/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   SwitchToZoomingCmd.h
//
//	Version	 :   18.02.94
//
//	Author	 :   Martin Diehl & Joachim Kupke
//
//	Language :   C++ 
//
//	Purpose	 :   Switches the "mode" to zooming mode, so that the
//		     user can mark an area with the mouse pointer that
//		     represent the new visible area.
//
////////////////////////////////////////////////////////////////////////	
#ifndef SWITCHTOZOOMINGCMD_H
#define SWITCHTOZOOMINGCMD_H

#include "Cmd.h"
#include "DrawArea.h"
#include "Geometric.h"
#include <X11/cursorfont.h>

class SwitchToZoomingCmd : public Cmd
{
   private:
      Cursor   _cursor;
      char     _zoomMode;
      
   protected:
     virtual void doit();

   public:
     virtual void undoit();
      SwitchToZoomingCmd(char*, int);
      ~SwitchToZoomingCmd();
      virtual const char *const className() {return ("SwitchToZoomingCmd");}
}; // SwitchToZoomingCmd

#endif
