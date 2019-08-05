/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   LoadCmd.h
//
//	Version	 :   24.03.1994
//
//	Author	 :   Martin Diehl
//
//	Language :   C++ 
//
//	Purpose	 :   Genreal Load command for GFE
// 
//
////////////////////////////////////////////////////////////////////////	
#ifndef LOADCMD_H
#define LOADCMD_H

#include "NoUndoCmd.h"
#include "Geometric.h"
#include <Xm/FileSB.h>

class ClientInterface;

class LoadCmd : public NoUndoCmd
{
   private:
      Widget _dialog;
      
   protected:
     virtual void doit();

   public:
      LoadCmd(char*, int);
      
      static void loadFileCallback(Widget, XtPointer, XmFileSelectionBoxCallbackStruct*);
      void loadFile(XmFileSelectionBoxCallbackStruct*);
      static void unmanageDialogCallback(Widget, XtPointer, XmFileSelectionBoxCallbackStruct*);
      void unmanageDialog();

      virtual const char *const className() {return ("LoadCmd");}
};

#endif
