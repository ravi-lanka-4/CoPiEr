/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ColorContextCmd.h
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	see ColorContextCmd.cc
// 
//
////////////////////////////////////////////////////////////////////////	

#ifndef COLORCONTEXTCMD_H
#define COLORCONTEXTCMD_H

#include "NoUndoCmd.h"

class ColorContext;
class DrawArea;

class ColorContextCmd : public NoUndoCmd
{
   protected:
      virtual void doit();

   public:
      virtual const char *const className() { return ("ColorContextCmd"); }
      ColorContextCmd (char*, int);

};

#endif
