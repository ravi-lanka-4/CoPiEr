/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   UsefulInlines.h
//			
//	Version	 :   07.04.94
//
//	Author	 :   Martin Diehl & Joachim Kupke
//
//	Compiler :   C++
//
//	Purpose	 :   Contains usefull inline functions.
//
////////////////////////////////////////////////////////////////////////	
#ifndef MARTINLIB_H
#define MARTINLIB_H

#include "Application.h"

// minimum    maximum
//
// minimum and maximum for all types which have '>' and '<' operators.
//
template<class T> inline T minimum(T A, T B) { return  ( A < B ? A : B );}
template<class T> inline T maximum(T A, T B) { return  ( A > B ? A : B );}

// kill
//  
// deletes a pointer if it is not NULL
//
template<class T> inline void kill(T p){ if (p) delete (p);}

// deletes a List of Pointers and the List itself
template<class T> inline void kill(T p, int count)
{
   for (int i = 0; i<count; i++)
      delete p[i];
   delete p;
}

template<class T> inline int round(T f)
{
   if (f>=0.0) return (int) (f + 0.5);
   else	       return (int) (f - 0.5);
}

inline void flush(Widget w)
{
   XFlush(XtDisplay(w));
}

inline void flushMainWindow()
{
   XFlush(XtDisplay(theApplication->mainWindow()->baseWidget()));
}

inline void dispatchEvents()
{
   XtAppContext app = theApplication->appContext();
   
   while(XtAppPending(app) & XtIMXEvent) {
      XEvent event;
      XtAppNextEvent(app, &event);
      XtDispatchEvent(&event);
   }
}

#endif


