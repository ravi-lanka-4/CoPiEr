/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	DisplayArea.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see DisplayArea.cc
// 
//
////////////////////////////////////////////////////////////////////////	


#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

#include <stdarg.h>

#include "GFE.inc"
#include "UIComponent.h"

class GFEApplication;

class DisplayArea : public UIComponent {

friend class GFEApplication;

private:

   Widget   _leftScrolledW;
   Widget   _leftDisplay;
   Widget   _rightScrolledW;
   Widget   _rightDisplay;

   Widget   _rightVerticalScrollbar;
   
   XmTextPosition    _leftPos;
   XmTextPosition    _rightPos;

   Boolean  _showRight;
   
protected:

   void initialize();

public:

   DisplayArea(Widget, char*, Boolean);
   ~DisplayArea();

   void printLeft(char*);
   void printRight(char*);
   void clearLeft();
   void clearRight();

   void updateDisplay();
};

extern int printf(const char *, ...);
extern int lprintf(const char *, ...);
extern int rprintf(const char *, ...);

extern DisplayArea *theDisplayArea;

#endif
