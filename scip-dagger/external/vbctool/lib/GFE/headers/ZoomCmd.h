/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   ZoomCmd.h
//
//	Version	 :   18.02.94
//
//	Author	 :   Martin Diehl & Joachim Kupke
//
//	Language :   C++ 
//
//	Purpose	 :
//
////////////////////////////////////////////////////////////////////////	
#ifndef ZOOMCMD_H
#define ZOOMCMD_H

#include "Cmd.h"
#include "DrawArea.h"
#include "Geometric.h"
#include "OpenZoomRectCmd.h"

class ZoomCmd : public Cmd
{
   private:
      OpenZoomRectCmd	*_ozrc;
      Rect<int>		_zoomRect;
      Rect<CoordType>   _oldZoomRange;
      
   protected:
     virtual void doit();
     virtual  void undoit();

   public:
      ZoomCmd(char*, int);

      void executeWithOpenZoomRectCmd(OpenZoomRectCmd* ozrc)
      {
	 _ozrc = ozrc;
	 execute();
      }
      
      virtual const char *const className() {return ("ZoomCmd");}
}; // ZoomCmd

#endif
