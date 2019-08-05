/////////////////////////////////////////////////////////////////////////
//
//
//	Filename :   SaveCmd.h
//
//	Version	 :   24.03.1994
//
//	Author	 :   Martin Diehl
//
//	Language :   C++ 
//
//	Purpose	 :   Genreal Save command for GFE
// 
//
////////////////////////////////////////////////////////////////////////	
#ifndef SAVECMD_H
#define SAVECMD_H

#include "NoUndoCmd.h"
#include "Geometric.h"
#include <Xm/FileSB.h>

class ClientInterface;

class SaveCmd : public NoUndoCmd
{
   private:
      Widget   _dialog;

   protected:
     virtual void doit();

   public:
      SaveCmd(char*, int);
      
      static void saveFileCallback(Widget, XtPointer, XmFileSelectionBoxCallbackStruct*);
      void saveFile(XmFileSelectionBoxCallbackStruct*);
      static void unmanageDialogCallback(Widget, XtPointer, XmFileSelectionBoxCallbackStruct*);
      void unmanageDialog();

      virtual const char *const className() {return ("SaveCmd");}
};
#endif
