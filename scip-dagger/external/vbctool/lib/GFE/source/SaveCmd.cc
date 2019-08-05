/////////////////////////////////////////////////////////////////////////
//
//
//   Filename  :  SaveCmd.cc
//
//   Version   :  24.03.194
//
//   Author    :  Martin Diehl
//
//   Language  :  C++
//
//   Purpose   :  General Save command for GFE
// 
//
////////////////////////////////////////////////////////////////////////   
#include "Application.h"
#include "SaveCmd.h"
#include "DrawArea.h"
#include "ClientInterface.h"
#include "GFEWindow.h"
#include "GFEResourceManager.h"
#include "GFE_C-Code.h"

#include <Xm/FileSB.h>

SaveCmd::SaveCmd(char *name,
                 int active) : NoUndoCmd(name, active)
{   
   _dialog          = NULL;
}


void SaveCmd::saveFileCallback(Widget _dialog,
                                  XtPointer clientData,
                                  XmFileSelectionBoxCallbackStruct *cbs)
{
   SaveCmd *obj = (SaveCmd *) clientData;
   obj->saveFile((XmFileSelectionBoxCallbackStruct*) cbs);
}


void SaveCmd::saveFile(XmFileSelectionBoxCallbackStruct *cbs)
{
   char *file = NULL;
   FILE *filePointer;
   float number;

   Cursor c1 = SetCursor(theApplication->mainWindow()->baseWidget(), XC_watch);
   Cursor c2 = SetCursor(_dialog, XC_watch);
   flushMainWindow();
   flush(_dialog);

   unmanageDialog();

   if (cbs)
   {
      if (!XmStringGetLtoR(cbs->value,
                           XmSTRING_DEFAULT_CHARSET, &file))   // convert to normal char.set
      {
         printf("Internal ERROR!!\n");
         return;
      }
      FILE *fp;
      fp = fopen(file, "w");
      if (fp)
      {
         theClientInterface->save(fp, file);
         fclose(fp);
      }
      kill(file);
   }
   ResetCursor(theApplication->mainWindow()->baseWidget(), c1);
   ResetCursor(_dialog, c2);
}
   
void SaveCmd::unmanageDialogCallback(Widget _dialog,
                 XtPointer clientData,
                 XmFileSelectionBoxCallbackStruct *cbs)
{
   SaveCmd *obj = (SaveCmd *) clientData;
   obj->unmanageDialog();
}

void SaveCmd::unmanageDialog()
{
   XtUnmanageChild(_dialog);
}


void SaveCmd::doit()
{
   if (!_dialog)
   {
         Cursor c = SetCursor(theApplication->mainWindow()->baseWidget(), XC_watch);
         flushMainWindow();

         _dialog = XmCreateFileSelectionDialog(theApplication->mainWindow()->baseWidget(),
                                           "Files", NULL, 0);

         XtVaSetValues(_dialog, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);

         XtSetSensitive(XmFileSelectionBoxGetChild(_dialog, XmDIALOG_HELP_BUTTON), False);

         XtVaSetValues(XtParent(_dialog),
                       XmNtitle, "Save File",
                       NULL);

         XtAddCallback(_dialog, XmNokCallback,
                    (void (*)(_WidgetRec*, void*, void*)) &SaveCmd::saveFileCallback,
                    (XtPointer) this);
                    
         XtAddCallback(_dialog, XmNcancelCallback,
                    (void (*)(_WidgetRec*, void*, void*)) &SaveCmd::unmanageDialogCallback,
                    (XtPointer) this);

         GFEResourceManager rsc(STANDARDRESOURCE);
         char *filter = rsc.getstring("SaveFileFilter");
         if (filter)
         {
            XmString str = XmStringCreateSimple(filter);
            XmFileSelectionDoSearch (_dialog, str);
            XmStringFree(str);
         }
         
         ResetCursor(theApplication->mainWindow()->baseWidget(), c);
   }
   XtManageChild(_dialog);
   XtPopup(XtParent(_dialog), XtGrabNone);
}

