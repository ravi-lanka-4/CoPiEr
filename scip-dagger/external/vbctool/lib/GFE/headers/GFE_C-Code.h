/*
/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	GFE_C-Code.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see .c
// 
//
////////////////////////////////////////////////////////////////////////	
*/

#ifndef GFECCODE_H
#define GFECCODE_H

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  c - IncludeFile for scalefonts.c */

/*  Prototypes */

Bool IsScalableFont(char*);

XFontStruct *LoadQueryScalableFont(Display*, int, char*, int);


/*  c - Includes for switching the Cursor */

Cursor SetCursor(Widget w, int cursorShape);

void ResetCursor(Widget w, Cursor c);


#ifdef __cplusplus
}
#endif

#endif
