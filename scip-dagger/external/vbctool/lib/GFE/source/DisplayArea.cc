/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	DisplayArea.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	Displays Text and Information to the user
// 
//
////////////////////////////////////////////////////////////////////////	

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <iostream>

#include <iostream>

#include "DisplayArea.h"
#include "GFEResourceManager.h"
#include "GFEWindow.h"
#include "ClientInterface.h"
#include "GFEApp2.h"

static Boolean showDisplayArea = False;

DisplayArea::DisplayArea(Widget parent, char *name, Boolean show)
		     : UIComponent(name)

{
   theDisplayArea = this;

   showDisplayArea = show;
   
   _leftPos = _rightPos = 0;

   _w = XtVaCreateWidget (name,
			  xmFormWidgetClass,
			  parent,
 			  NULL);

   int ratio;

   GFEResourceManager rsc(STANDARDRESOURCE);
   char *string = rsc.getstring("DisplayAreaRatio");

   if (!string)
      ratio = 50;
   else
      if(sscanf(string, "%d", &ratio)<=0) ratio = 50;

   _leftScrolledW = XtVaCreateWidget("lscrW",
				     xmScrolledWindowWidgetClass,
				     _w,
				     XmNtopAttachment,	     XmATTACH_FORM,
				     XmNleftAttachment,	     XmATTACH_FORM,
				     XmNrightAttachment,     XmATTACH_POSITION,
				     XmNrightPosition,	     ratio,
				     XmNbottomAttachment,    XmATTACH_FORM,
				     XmNscrollingPolicy,     XmAUTOMATIC,
				     NULL);

   _leftDisplay = XtVaCreateManagedWidget("leftDisplay",
				   xmTextWidgetClass,
				   _leftScrolledW,
				   XmNshadowThickness,	     0,
				   XmNhighlightThickness,    0,
				   XmNalignment,	     XmALIGNMENT_BEGINNING,
				   XmNeditable,		     False,
				   XmNeditMode,		     XmMULTI_LINE_EDIT,
				   XmNwordWrap,		     True,
				   XmNcursorPositionVisible,  False,
				   XmNresizeHeight,	     True,
				   XmNresizeWidth,	     True,
				   XmNmarginHeight,	     5,
				   XmNmarginWidth,	     5,
				   NULL);
					  
   _rightScrolledW = XtVaCreateManagedWidget("rscrW",
				     xmScrolledWindowWidgetClass,
				     _w,
				     XmNtopAttachment,    XmATTACH_FORM,
				     XmNleftAttachment,   XmATTACH_WIDGET,
				     XmNleftWidget,	  _leftScrolledW,
				     XmNrightAttachment,  XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNscrollingPolicy,  XmAUTOMATIC,
				     //XmNscrollingPolicy,  XmAPPLICATION_DEFINED,
				     //XmNvisualPolicy,	  XmVARIABLE,
				     XmNscrollBarDisplayPolicy,	  XmAS_NEEDED,
				     NULL);

   XtVaGetValues(_rightScrolledW, XmNverticalScrollBar, &_rightVerticalScrollbar, NULL);

   _rightDisplay = XtVaCreateManagedWidget("rightDisplay",
				   xmTextWidgetClass,
				   _rightScrolledW,
				   XmNshadowThickness,	     0,
				   XmNhighlightThickness,    0,
				   XmNalignment,	     XmALIGNMENT_BEGINNING,
				   XmNeditable,		     False,
				   XmNeditMode,		     XmMULTI_LINE_EDIT,
				   XmNwordWrap,		     True,
				   XmNcursorPositionVisible,  False,
				   XmNresizeHeight,	     True,
				   XmNresizeWidth,	     True,
				   XmNmarginHeight,	     5,
				   XmNmarginWidth,	     5,
				   NULL);


   installDestroyHandler();
   
}

DisplayArea::~DisplayArea()
{
}

void DisplayArea::printRight(char *string)
{
   if (_showRight) {
       XmTextInsert(_rightDisplay, _rightPos, string);
       _rightPos += strlen(string);
       XtVaSetValues(_rightDisplay, XmNcursorPosition, _rightPos, NULL);
       XmTextShowPosition(_rightDisplay, _rightPos);
       
       int max, val, size, incr, pageIncr;
       XtVaGetValues(_rightVerticalScrollbar,
		     XmNsliderSize,	 &size,
		     XmNmaximum,	 &max,
		     XmNincrement,	 &incr,
		     XmNpageIncrement, &pageIncr,
		     NULL);
       
       XmScrollBarSetValues(_rightVerticalScrollbar,
			    max - size,
			    size,
			    incr,
			    pageIncr,
			    True);
       
//   XFlush(XtDisplay(_rightDisplay));
   }
   else
       std::cout << string;
}

void DisplayArea::printLeft(char *string)
{
   clearLeft();
   XmTextInsert(_leftDisplay, _leftPos, string);
   _leftPos += strlen(string);
   
   XtVaSetValues(_leftDisplay, XmNcursorPosition, _leftPos, NULL);
   XmTextShowPosition(_leftDisplay, _leftPos);
   if (!XtIsManaged(_leftScrolledW))
      XtManageChild(_leftScrolledW);

/*   flush(_leftDisplay);
   flush(_leftScrolledW);
   Widget horSB;
   Widget verSB;
   XtVaGetValues(_leftScrolledW, XmNhorizontalScrollBar, &horSB,
			         XmNverticalScrollBar,   &verSB, NULL);
   if (horSB)
      flush(horSB);
   if (verSB);
      flush(verSB);
*/
}

void DisplayArea::clearLeft()
{
   XmTextSetString(_leftDisplay, "");
   _leftPos = 0;
}

void DisplayArea::clearRight()
{
   XmTextSetString(_rightDisplay, "");
   _rightPos = 0;
}

void DisplayArea::updateDisplay()
{
   XmUpdateDisplay(_leftScrolledW);
   XmUpdateDisplay(_rightScrolledW);
   XmUpdateDisplay(_leftDisplay);
   XmUpdateDisplay(_rightDisplay);
}

void DisplayArea::initialize()
{
	 Dimension height;
	 XtVaGetValues(_w, XmNheight, &height, NULL);

         GFEApplication *app = (GFEApplication*) theApplication;
	 int i;
	 for (i = 0; i < app->argc(); i++)
         if (!strcmp("-noRightDisplay", app->argv()[i]) || height <= 1)
	     break;

	 if (i < app->argc())
	     _showRight = false;
	 else
	     _showRight = true;

}

// overloaded printf-functions

static char buf[4096];
static int returnValue;

int printf(const char *fmt, ...)
{
   va_list args;
      
   va_start(args, fmt);

   if (showDisplayArea)
      returnValue = vsprintf(buf, fmt, args);
   else
      returnValue = vfprintf(stdout, fmt, args);

   va_end(args);

   if (showDisplayArea)
      theDisplayArea->printRight(buf);

   return returnValue;
}

int rprintf(const char *fmt, ...)
{
   va_list args;
      
   va_start(args, fmt);

   if (showDisplayArea)
      returnValue = vsprintf(buf, fmt, args);
   else
      returnValue = vfprintf(stdout, fmt, args);

   va_end(args);

   if (showDisplayArea)
      theDisplayArea->printRight(buf);

   return returnValue;
}

int lprintf(const char *fmt, ...)
{
   va_list args;

   va_start(args, fmt);

   if (showDisplayArea)
      returnValue = vsprintf(buf, fmt, args);
   else
      returnValue = vfprintf(stdout, fmt, args);

   va_end(args);

   if (showDisplayArea)
      theDisplayArea->printLeft(buf);

   return returnValue;
}

