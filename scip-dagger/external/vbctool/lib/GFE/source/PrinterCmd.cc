/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PrinterCmd.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	called after pressing the menu-button PRINT
// 
//
////////////////////////////////////////////////////////////////////////	

#include "PrinterCmd.h"
#include "PrintPopup.h"
#include "DrawArea.h"
#include "Postscript.h"

PrinterCmd::PrinterCmd(char *name,
			int active)
	 : NoUndoCmd(name, active)

{}

void PrinterCmd::doit()
{
   int l, r, t, b;
   
   theDrawArea->postscript()->getMargins(l, r, t, b);
   theDrawArea->printPopup()->post(l, r, t, b);
}
