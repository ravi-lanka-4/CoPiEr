/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PrinterCmd.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see .cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef PRINTERCMD_H
#define PRINTERCMD_H

#include "NoUndoCmd.h"

class Postscript;

class PrinterCmd : public NoUndoCmd
{
   private:

   protected:
      virtual void doit();

   public:
      virtual const char *const className() { return ("PrinterCmd"); }
      PrinterCmd (char*, int);

};

#endif
