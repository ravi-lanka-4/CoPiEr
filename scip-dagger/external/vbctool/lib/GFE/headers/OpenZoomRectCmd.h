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
#ifndef OPENZOOMRECTCMD_H
#define OPENZOOMRECTCMD_H

#include "NoUndoCmd.h"
#include "DrawArea.h"
#include "Geometric.h"


class OpenZoomRectCmd : public NoUndoCmd
{
   friend class ZoomCmd;

   private:
   
      Point<int>  _point;
      Point<int>  _firstPoint;
      Rect<int>   _zoomRect;
      char	  _dragging;
   
   
   protected:
     virtual void doit();

   public:
   
      void executeWithPoint(Point<int> *point)
      { // Gets the point where the event occurs and
	// calls the execute() function.
	
	_point.newInit(point);

	if (_point.x() < (theDrawArea->dAreaSize()->minX())) _point.setX(theDrawArea->dAreaSize()->minX());
	if (_point.y() < (theDrawArea->dAreaSize()->minY())) _point.setY(theDrawArea->dAreaSize()->minY());
	if (_point.x() > (theDrawArea->dAreaSize()->maxX())) _point.setX(theDrawArea->dAreaSize()->maxX());
	if (_point.y() > (theDrawArea->dAreaSize()->maxY())) _point.setY(theDrawArea->dAreaSize()->maxY());
	
	execute();

      }
      Rect<int> *getRect() { return (&_zoomRect); }
      
      OpenZoomRectCmd(char*, int);

      virtual const char *const className() {return ("OpenZoomRectCmd");}
      
}; // OpenZoomRectCmd

#endif
