/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      DrawingField.cc
//
//      Version  :      19.09.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////


#include "DrawingField.h"
#include "DrawArea.h"

DrawingField::DrawingField(Widget parent, char *label, int width, int height, char *color)
{
   _da = XtVaCreateManagedWidget("DialogDrawingArea",
				 xmDrawingAreaWidgetClass, parent,
				 XmNwidth, width,
				 XmNheight, height,
				 NULL);

   XColor actColor;
   XColor RGBValues;
   XAllocNamedColor(XtDisplay(_da),
		    _cm = DefaultColormap(XtDisplay(_da), DefaultScreen(XtDisplay(_da))),
		    color,
		    &actColor,
		    &RGBValues);

   _pv = actColor.pixel;

   _gc = XCreateGC(XtDisplay(_da),
		   RootWindowOfScreen(XtScreen(_da)),
		   0, NULL);

   XSetForeground(XtDisplay(_da),
		  _gc,
		  _pv);
}

DrawingField::~DrawingField()
{
   unsigned long pixels[1];
   pixels[0] = _pv;
   XFreeColors(XtDisplay(_da), _cm, pixels, 1, 0);
}


void DrawingField::drawCircle(int x, int y, int rad)
{
   XDrawArc(XtDisplay(_da),
	    XtWindow(_da),
	    _gc,
	    x - rad, y - rad, rad * 2, rad * 2,
	    0, 360 * 64);
}

void DrawingField::fillCircle(int x, int y, int rad)
{
   XFillArc(XtDisplay(_da),
	    XtWindow(_da),
	    _gc,
	    x - rad, y - rad, rad * 2, rad * 2,
	    0, 360 * 64);
}
