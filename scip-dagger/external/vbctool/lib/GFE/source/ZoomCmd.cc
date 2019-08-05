/////////////////////////////////////////////////////////////////////////
//
//
// Filename :  ZoomCmd.cc
//
// Version  :  18.02.94
//
// Author   :  Martin Diehl & Joachim Kupke
//
// Language :  C++
//
// Purpose  :  methods for Zooming
// 
//
////////////////////////////////////////////////////////////////////////   

#include "ZoomCmd.h"
#include "SwitchToZoomingCmd.h"
#include "OpenZoomRectCmd.h"
#include "DrawArea.h"
#include "Geometric.h"

ZoomCmd::ZoomCmd(char *name,
		 int active) : Cmd(name, active)
{}

void ZoomCmd::doit()
{
   if(_ozrc->_dragging)
   {
      _ozrc->_dragging = 0;
      _zoomRect=_ozrc->_zoomRect;
      theDrawArea->drawBoxWithXOr (&_zoomRect);

      _oldZoomRange = theDrawArea->_demandedVisibleRange;
      theDrawArea->_scaler->reConvertRect (&_zoomRect, &theDrawArea->_demandedVisibleRange);
      theDrawArea->adjustToNewView(ZOOMING_NEW_VIEW);
      
      // more tasks
   }
}

void ZoomCmd::undoit()
{
      theDrawArea->_demandedVisibleRange = _oldZoomRange;
      theDrawArea->adjustToNewView(GENERAL_NEW_VIEW);

      theDrawArea->_switchToZoomingCmd->activate();
      theDrawArea->_openZoomRectCmd->deactivate();
      theDrawArea->_zoomCmd->deactivate();
}
