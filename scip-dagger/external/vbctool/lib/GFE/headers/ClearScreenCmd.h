/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ClearScreenCmd.h
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
#ifndef CLEARSCREENCMD_H
#define CLEARSCREENCMD_H

#include "NoUndoCmd.h"
#include "GFE.inc"
#include "DrawArea.h"

class ClearScreenCmd : public NoUndoCmd	  
{
   private:
      ColorID   _colorID;

   protected:
     virtual void doit();

   public:
      virtual const char *const className() {return ("ClearScreenCmd");}
      ClearScreenCmd( char*, int);
      ColorID  colorID() {return _colorID; }
};

#endif

