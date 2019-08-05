#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>

#include "NodeSelection.h"
#include "DrawArea.h"

NodeSelection::NodeSelection(char *name, Widget w) :
   PopupManager(name, w, XmDIALOG_MODELESS)
{
   _nodeIF = NULL;
}

NodeSelection::~NodeSelection()
{
   kill (_nodeIF);
}

void NodeSelection::createActionArea()
{
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON));
}

void NodeSelection::createControlArea()
{
   _mainRow = XtVaCreateManagedWidget("nodeSelectionRow",
				      xmRowColumnWidgetClass,
				      _w,
				      NULL);

   _label = XtVaCreateManagedWidget("Node - selection",
				    xmLabelWidgetClass,
				    _mainRow,
				    XmNalignment, XmALIGNMENT_CENTER,
				    NULL);

   Widget sep = XtVaCreateManagedWidget("sep",
					xmSeparatorWidgetClass,
					_mainRow,
					NULL);

   _nodeIF = new InputField(_mainRow, "nsRow", "Starting node:", "", XmHORIZONTAL, False);
}

void NodeSelection::ok()
{
   gotoOk(getValue());
}

void NodeSelection::cancel()
{
   gotoCancel();
}

void NodeSelection::setValue(int v)
{
   if (!_nodeIF) return;

   if (v<0) _nodeIF->setValue("");
   else
   {
      char buf[100];
      sprintf(buf, "%d", v);

      _nodeIF->setValue(buf);
   }
}

void NodeSelection::post(void (*ok)(int), void (*cancel)())
{
   gotoOk = ok;
   gotoCancel = cancel;
   
   PopupManager::post();

   setValue(-1);
   XtVaSetValues(_label, XmNalignment, XmALIGNMENT_CENTER, NULL);
}

int NodeSelection::getValue()
{
   char *text = _nodeIF->getValue();
   int v;
   if (sscanf(text, "%d", &v)<=0) v = -1;
   XtFree(text);
   return v;
}
