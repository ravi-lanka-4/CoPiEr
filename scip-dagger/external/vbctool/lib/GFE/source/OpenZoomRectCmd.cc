/////////////////////////////////////////////////////////////////////////
//
//
// Filename :  OpenZoomRectCmd.C
//
// Version  :  18.02.94
//
// Author   :  Martin Diehl & Joachim Kupke
//
// Language :  C++
//
// Purpose  :  Cmd to open the Zoom-rect
// 
//
////////////////////////////////////////////////////////////////////////   

#include "OpenZoomRectCmd.h"
#include "DrawArea.h"
#include "Geometric.h"

OpenZoomRectCmd::OpenZoomRectCmd(char *name,
			      int active) : NoUndoCmd(name, active)
{   
   _dragging = 0;
}

void OpenZoomRectCmd::doit()
{

   if (!_dragging)
   {
       _dragging = 1;
       _firstPoint.newInit(&_point);
   }
   else theDrawArea->drawBoxWithXOr (&_zoomRect);
	
   _zoomRect.newInit (&_firstPoint, &_point);

   theDrawArea->drawBoxWithXOr (&_zoomRect);
   		
}

