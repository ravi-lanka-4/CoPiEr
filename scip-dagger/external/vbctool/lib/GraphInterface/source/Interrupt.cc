/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      Interrupt.cc
//
//      Version  :      22.02.95
//
//      Author   :      Joachim Kupke
//
//      Language :      C++
//
//      Purpose  :      
//
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <Xm/SelectioB.h>

#include "Interrupt.h"
#include "GFEWindow.h"

void Interrupt::createActionArea()
{
//   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_SEPARATOR));

   XmString str = XmStringCreateSimple("    Interrupt    ");
   XtVaSetValues(_w, XmNokLabelString, str, NULL);
   XmStringFree(str);

//   XtVaSetValues(_w, XmNdefaultPosition, False, NULL);
}

Interrupt::Interrupt() : InterruptBasePopup("Interrupt")
{
   assert(theInterrupt == NULL); // has to be created only once
   theInterrupt = this;
   _interrupted = False;
}

void Interrupt::ok()
{
   _interrupted = True;
}


void Interrupt::post()
{
   if (!_w || !XtIsManaged(_w)) {
      _interrupted = False;

      PopupManager::post();
   }
}
