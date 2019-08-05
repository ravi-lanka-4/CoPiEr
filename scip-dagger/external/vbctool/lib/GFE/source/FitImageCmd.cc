/////////////////////////////////////////////////////////////////////////
//
//
//   Filename  :  FitImageCmd.C
//
//   Version   :  24.03.1994
//
//   Author    :  Martin Diehl
//
//   Language  :  C++
//
//   Purpose   :  displays the original size after Zooming
// 
//
////////////////////////////////////////////////////////////////////////   
#include "FitImageCmd.h"
#include "DrawArea.h"
#include "Geometric.h"

FitImageCmd::FitImageCmd(char *name,
			  int active) : Cmd(name, active)
{}

void FitImageCmd::doit()
{
   _oldVisibleRect = theDrawArea->_actualVisibleRange;
   theDrawArea->_demandedVisibleRange = theDrawArea->_coordinateRange;
   theDrawArea->adjustToNewView(GENERAL_NEW_VIEW);
}

void FitImageCmd::undoit()
{
   theDrawArea->_demandedVisibleRange = _oldVisibleRect;
   theDrawArea->adjustToNewView(GENERAL_NEW_VIEW);
}

