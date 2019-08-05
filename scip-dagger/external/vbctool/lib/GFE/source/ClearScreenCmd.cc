/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ClearScreenCmd.C
//
//	Version	 :	06.12.93
//
//	Author	 :	Martin Diehl
//
//	Language :	C++ 
//
//	Purpose	 :	Clears the DrawArea
// 
//
////////////////////////////////////////////////////////////////////////	
#include "DrawArea.h"
#include "ClearScreenCmd.h"
#include "GFEResourceManager.h"
#include "Postscript.h"

ClearScreenCmd::ClearScreenCmd(char *name,
			       int  active)
	    : NoUndoCmd(name, active)   
{   
   GFEResourceManager	rsc(STANDARDRESOURCE);

   char *h = rsc.getstring("DrawAreaBackgroundColor");
   if (!h)
   {
      _colorID = 0;
      return;
   }

   h = strdup(h);
   _colorID = theDrawArea->createNewROColor(h, "Background");
   theDrawArea->postscript()->setBackgroundColorName(h);
   delete(h);
   
   
   // Register the Background color
//   _colorID = theDrawArea->createNewROColor("green", "Background");
     //_colorID = 0;  // default-value for not explicitly registered BG-Color
}

void ClearScreenCmd::doit()
{
   theDrawArea->fillCanvas(_colorID);
}

