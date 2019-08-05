/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      InterruptBasePopup.cc
//
//      Version  :      24.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////


#include "InterruptBasePopup.h"
#include "GFEWindow.h"
#include "DisplayArea.h"
#include "DrawArea.h"
#include "MenuBar.h"

InterruptBasePopup::InterruptBasePopup(char *name) :
   PopupManager(name, theGFEWindow->menuBar()->baseWidget())
{}

void InterruptBasePopup::map()
{
   //centerPopupShell(theDisplayArea->baseWidget());

/*
   Position x, y;
   Position oldx, oldy;
   Dimension appw, apph;
   Dimension myw, myh;

   XtVaGetValues(theDisplayArea->baseWidget(),
		 XmNheight, &apph, NULL);

   XtVaGetValues(theDrawArea->baseWidget(),
		 XmNheight, &myh, NULL);

   XtVaGetValues(_w, XmNy, &y, NULL);

   y += (myh/2 + apph/2);

   XtVaSetValues(_w, XmNy, y, NULL);

/*   XtVaGetValues(theGFEWindow->baseWidget(),
		 XmNwidth, &appw,
		 XmNheight, &apph,
		 NULL);

   XtVaGetValues(_w,
		 XmNwidth, &myw,
		 XmNheight, &myh,
		 NULL);

   XtVaGetValues(_w,
		 XmNx, &oldx,
		 XmNy, &oldy,
		 NULL);

   x = 0;//oldx + appw/2 - myw/2;
   y = 0;//oldy - apph/2 + myh/2;

/*   XtVaSetValues(_w,
		 XmNx, x,
		 XmNy, y,
		 NULL);
*/
}
