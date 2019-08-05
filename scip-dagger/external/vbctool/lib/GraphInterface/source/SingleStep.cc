/////////////////////////////////////////////////////////////////////////
//
//
//      Filename :      SingleStep.cc
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


#include "SingleStep.h"

#include <assert.h>
#include <Xm/SelectioB.h>

#include "GFEWindow.h"

void SingleStep::createActionArea()
{
//   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_OK_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_SEPARATOR));
   Widget h = XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON);
   XtManageChild(h);
   XtVaSetValues(_w, XmNdefaultButton, h, NULL);

   XmString str = XmStringCreateSimple("    Next    ");
   XtVaSetValues(_w, XmNapplyLabelString, str, NULL);
   XmStringFree(str);

   str = XmStringCreateSimple("    Continue    ");
   XtVaSetValues(_w, XmNcancelLabelString, str, NULL);
   XmStringFree(str);
}

SingleStep::SingleStep() : InterruptBasePopup("Single Step")
{
   assert(theSingleStep == NULL); // has to be created only once
   theSingleStep = this;
}

SingleStep::~SingleStep()
{}

void SingleStep::apply()
{
   _sign = NEXT;
}


void SingleStep::cancel()
{
   _sign = CONTINUE;
}

void SingleStep::post(Boolean ia)
{
   if (!_w || !XtIsManaged(_w)) {
      _interruptAllowed = ia;
      _sign = NOTHING;

      PopupManager::post();
   }
}
