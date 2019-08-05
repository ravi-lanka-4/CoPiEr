/////////////////////////////////////////////////////////////////////////
//
//
//   Filename  :   FitImageCmd.h
//
//   Version   :   24.03.94
//
//   Author    :   Martin Diehl
//
//   Language  :   C++ 
//
//   Purpose   :   see .cc
// 
//
////////////////////////////////////////////////////////////////////////   
#ifndef FITIMAGECMD_H
#define FITIMAGECMD_H

#include "Cmd.h"
#include "NoUndoCmd.h"
#include "Geometric.h"
#include "DrawArea.h"

class FitImageCmd : public Cmd
{
   private:
	 Rect<CoordType> _oldVisibleRect;

   protected:
	virtual void doit();
	virtual void undoit();

   public:
   FitImageCmd(char*, int);
   virtual const char *const className() {return ("FitImageCmd");}
};

class RepaintCmd : public NoUndoCmd
{
   private:

   protected:
	 virtual void doit() { theDrawArea->repaint(); }

   public:
	 RepaintCmd(char *name, int active) : NoUndoCmd(name, active)
	 {}
	 
	 virtual const char *const className() { return ("RepaintCmd"); }
};

#endif
