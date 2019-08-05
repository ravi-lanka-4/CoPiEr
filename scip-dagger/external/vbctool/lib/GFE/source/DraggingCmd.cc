/////////////////////////////////////////////////////////////////////////
//
//
// Filename :  DraggingCmd.C
//
// Version  :  18.05.94
//
// Author   :  Joachim Kupke
//
// Language :  C++
//
// Purpose  :  supports dragging with the middle mouse-button in zooming-mode
// 
//
////////////////////////////////////////////////////////////////////////   

#include "DraggingCmd.h"
#include "DrawArea.h"
#include "Geometric.h"

DraggingCmd::DraggingCmd(char *name,
			      int active) : NoUndoCmd(name, active)
{   
   _first = TRUE;
   int Cursorshape = XC_fleur;
   _cursor = XCreateFontCursor(theDrawArea->display(), Cursorshape);
}

DraggingCmd::~DraggingCmd()
{
      XFreeCursor(theDrawArea->display(), _cursor);
      XDefineCursor(theDrawArea->display(),
		    theDrawArea->window(), (Cursor) NULL);   // NULL switches back to
			                          // default cursor.
}
								  

void DraggingCmd::doit()
{
   if(theDrawArea->_horSBSliderSize == theDrawArea->_horSBMaximum &&
      theDrawArea->_verSBSliderSize == theDrawArea->_verSBMaximum) return;

   CoordType dx = theDrawArea->_scaler->reConvert(_oldPoint.x() - _point.x());
   CoordType dy = theDrawArea->_scaler->reConvert(_oldPoint.y() - _point.y());

   if (theDrawArea->_yInvert)
      dy = -dy;

   dx = (theDrawArea->_actualVisibleRange.minX()+dx>theDrawArea->_actualCoordRange.minX())? dx :
	 theDrawArea->_actualCoordRange.minX() - theDrawArea->_actualVisibleRange.minX();
   dy = (theDrawArea->_actualVisibleRange.minY()+dy>theDrawArea->_actualCoordRange.minY())? dy :
	 theDrawArea->_actualCoordRange.minY() - theDrawArea->_actualVisibleRange.minY();

   dx = (theDrawArea->_actualVisibleRange.maxX()+dx<theDrawArea->_actualCoordRange.maxX())? dx :
	 theDrawArea->_actualCoordRange.maxX() - theDrawArea->_actualVisibleRange.maxX();
   dy = (theDrawArea->_actualVisibleRange.maxY()+dy<theDrawArea->_actualCoordRange.maxY())? dy :
	 theDrawArea->_actualCoordRange.maxY() - theDrawArea->_actualVisibleRange.maxY();
      
   theDrawArea->_demandedVisibleRange.moveRect(dx, dy);
   
   theDrawArea->adjustToNewView(DRAGGING_NEW_VIEW);
   		
}

