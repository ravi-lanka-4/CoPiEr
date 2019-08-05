/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PopupManager.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	abstract Base Class for Popup-Menus
// 
//
////////////////////////////////////////////////////////////////////////	

#include <Xm/SelectioB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>


#include "PopupManager.h"
#include "DrawArea.h"
#include "DisplayArea.h"

static DrawArea *dra; // because of Error in static member function with non
		      // static object
static DisplayArea *dia;

PopupManager::PopupManager(char *name, Widget parent, unsigned char modality)
   : UIComponent(name)
{
   dra = theDrawArea;
   dia = theDisplayArea;
   _parent = parent;
   _modality = modality;
}

PopupManager::~PopupManager()
{
   unpost();
   
   if (_w)
      XtDestroyWidget(_w);
}


void PopupManager::getDefaultDialog()
{
   if (_w)
      return;

   _w = XmCreatePromptDialog(_parent, _name, NULL, 0);

   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_SELECTION_LABEL));
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_TEXT));

   XtVaSetValues(XtParent(_w),
		 XmNtitle, _name,
		 NULL);

   XtVaSetValues(_w, XmNdialogStyle, _modality, NULL);

   createActionArea();
   createControlArea();
   
}

void PopupManager::getSelectionDialog()
{
   if (_w)
      return;

   _w = XmCreateSelectionDialog(_parent, _name, NULL, 0);

   XtVaSetValues(XtParent(_w),
		 XmNtitle, _name,
		 NULL);


   XtVaSetValues(_w, XmNdialogStyle, _modality, NULL);

   createControlArea();
   createActionArea();
   
}


void PopupManager::getDialog()
{
   getDefaultDialog();
}


void PopupManager::centerPopupShell(Widget widget)
{
   Dimension w, h, ww, wh;
   Position x, y;
   Position px, py;
   
   XtVaGetValues(XtParent(widget), XmNx, &px, XmNy, &py, NULL);
   XtVaGetValues(widget, XmNx, &x, XmNy, &y, NULL);
   XtVaGetValues(widget, XmNwidth, &ww, XmNheight, &wh, NULL);
   XtVaGetValues(_w, XmNwidth, &w, XmNheight, &h, NULL);

   x += (px + ww/2 - w/2);
   y -= (py + wh/2 - h/2);

   XtVaSetValues(_w, XmNx, x, XmNy, y, NULL);
}

void PopupManager::post()
{
   if (_w && XtIsManaged (_w))
   {
      XSetWindowAttributes help;
      help.override_redirect = True;
      XChangeWindowAttributes(XtDisplay(_w), XtWindow(_w), CWOverrideRedirect, &help);
      
      XRaiseWindow(XtDisplay(_w), XtWindow(_w));
//      XMapRaised(XtDisplay(_w), XtWindow(_w));
      return;
   }

   if (!_w)
      getDialog();
   
   enableCallbacks();

   XtManageChild(_w);
}

void PopupManager::unpost()
{
   if (!_w)
      return;

   disableCallbacks();

   if (!XtIsManaged(_w))
      return;

   XtUnmanageChild(_w);
   dra->updateDisplay();
   dia->updateDisplay();
}

// function catenate :

// catenates a Widget in the Form
// st represents the edges, which attache the Form:
// 1 right edge
// 2 bottom edge
// 4 left edge
// 8 top edge
// the sum of the Form-attachments is st.

// w? represents the edges, which attache a widget:
// 1 right edge
// 2 bottom edge
// 3 left edge
// 4 top edge

// You can achieve non-attachement by setting the edge-Widget to NULL and
// the st-value for this edge to 0.

void PopupManager::catenate(Widget *w, Widget *w1, Widget *w2, Widget *w3, Widget *w4, int st)
{
   if (st <0 || st > 15)
      fprintf(stderr, "ERROR in PopupManager::catenate(...)\n");

   // left Attachment

   if (st & 4)
      XtVaSetValues(*w,
		    XmNleftAttachment, XmATTACH_FORM,
		    NULL);
   else
      if (!w3)
	 XtVaSetValues(*w,
		       XmNleftAttachment, XmATTACH_NONE,
		       NULL);
      else
	 XtVaSetValues(*w,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget,	  *w3,
		       NULL);

   // bottom Attachment

   if (st & 2)
      XtVaSetValues(*w,
		    XmNbottomAttachment, XmATTACH_FORM,
		    NULL);
   else
      if (!w2)
	 XtVaSetValues(*w,
		       XmNbottomAttachment, XmATTACH_NONE,
		       NULL);
      else
	 XtVaSetValues(*w,
		       XmNbottomAttachment,  XmATTACH_WIDGET,
		       XmNbottomWidget,	     *w2,
		       NULL);

   // right Attachment

   if (st & 1)
      XtVaSetValues(*w,
		    XmNrightAttachment, XmATTACH_FORM,
		    NULL);
   else
      if (!w1)
	 XtVaSetValues(*w,
		       XmNrightAttachment, XmATTACH_NONE,
		       NULL);
      else
	 XtVaSetValues(*w,
		       XmNrightAttachment, XmATTACH_WIDGET,
		       XmNrightWidget,	  *w1,
		       NULL);

   // top Attachment

   if (st & 8)
      XtVaSetValues(*w,
		    XmNtopAttachment, XmATTACH_FORM,
		    NULL);
   else
      if (!w4)
	 XtVaSetValues(*w,
		       XmNtopAttachment, XmATTACH_NONE,
		       NULL);
      else
	 XtVaSetValues(*w,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget,	  *w4,
		       NULL);
   
}


void PopupManager::enableCallbacks()
{
   Widget hw = XmSelectionBoxGetChild(_w, XmDIALOG_OK_BUTTON);
   XtAddCallback(hw, XmNactivateCallback, &PopupManager::okCallback, (XtPointer) this);
   
   XtAddCallback(_w, XmNcancelCallback, &PopupManager::cancelCallback, (XtPointer) this);
   XtAddCallback(_w, XmNapplyCallback, &PopupManager::applyCallback, (XtPointer) this);
   XtAddCallback(_w, XmNhelpCallback, &PopupManager::helpCallback, (XtPointer) this);

   XtAddCallback(_w, XmNmapCallback, &PopupManager::mapCallback, (XtPointer) this);
}



void PopupManager::disableCallbacks()
{
   Widget hw = XmSelectionBoxGetChild(_w, XmDIALOG_OK_BUTTON);
   XtRemoveCallback(hw, XmNactivateCallback, &PopupManager::okCallback, (XtPointer) this);

   XtRemoveCallback(_w, XmNcancelCallback, &PopupManager::cancelCallback, (XtPointer) this);
   XtRemoveCallback(_w, XmNapplyCallback, &PopupManager::applyCallback, (XtPointer) this);
   XtRemoveCallback(_w, XmNhelpCallback, &PopupManager::helpCallback, (XtPointer) this);

   XtRemoveCallback(_w, XmNmapCallback, &PopupManager::mapCallback, (XtPointer) this);
}



void PopupManager::okCallback(Widget, XtPointer clientData, XtPointer)
{
   PopupManager *obj = (PopupManager *) clientData;

   obj->disableCallbacks();
   dra->updateDisplay();
   dia->updateDisplay();

   obj->ok();
}

void PopupManager::cancelCallback(Widget, XtPointer clientData, XtPointer)
{
   PopupManager *obj = (PopupManager *) clientData;

   obj->disableCallbacks();
   dra->updateDisplay();
   dia->updateDisplay();

   obj->cancel();
}

void PopupManager::applyCallback(Widget, XtPointer clientData, XtPointer)
{
   PopupManager *obj = (PopupManager *) clientData;

   obj->apply();
}

void PopupManager::helpCallback(Widget, XtPointer clientData, XtPointer)
{
   PopupManager *obj = (PopupManager *) clientData;

   obj->help();
}

void PopupManager::mapCallback(Widget, XtPointer clientData, XtPointer)
{
   PopupManager *obj = (PopupManager *) clientData;

   obj->map();
}

InputField::InputField(Widget parent, char *row, char* label, char* defaultText,
		       unsigned char orientation, Bool withFrame, Bool textbox)
{
   Widget frame;

   if (withFrame)
      frame = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
   else frame = parent;

   char *hr;
   if (row) hr = row;
   else     hr = "row";
   _row = XtVaCreateManagedWidget(hr,
				  xmRowColumnWidgetClass,
				  frame,
				  XmNorientation,   orientation,
				  //XmNpacking,	    XmPACK_COLUMN,
				  //XmNadjustLast,    True,
				  NULL);

   _label = XtVaCreateManagedWidget(label,
				    xmLabelWidgetClass,
				    _row,
				    NULL);

   _text = XtVaCreateManagedWidget("inputText",
				   xmTextWidgetClass,
				   _row,
				   XmNvalue,  defaultText,
				   //XmNwidth,  100,
				   //XmNshadowThickness, 0,
				   //XmNhighlightThickness, 1,
				   NULL);

   if (!textbox)
      XtVaSetValues(_text,
		    XmNshadowThickness,	  0,
		    XmNhighlightThickness,1,
		    NULL);

}

void InputField::setValue(char *value)
{
   XtVaSetValues(_text, XmNvalue, value, NULL);
}
   

char *InputField::getValue() // You have to free the returned Pointer with XtFree(p) after using
{
   char *text;
   text = XmTextGetString(_text);
   return text;
}


RadioField::RadioField(Widget parent, char *row, int numberOfButtons,
		       char **buttonLabels, int activeButton,
		       unsigned char orientation, short columns, Bool withFrame)
{
   _numberOfButtons = numberOfButtons;
   
   Widget frame;

   if (withFrame)
      frame = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
   else frame = parent;

   char *hr;
   if (row) hr = row;
   else     hr = "row";
   _row = XmCreateRadioBox(frame, row, NULL, 0);
   XtVaSetValues(_row,
		 XmNorientation, orientation,
		 XmNnumColumns, columns,
		 NULL);
   XtManageChild(_row);

   _buttons = new Widget[numberOfButtons];

   for (int i = 0; i<numberOfButtons; i++)
      _buttons[i] = XtVaCreateManagedWidget(buttonLabels[i], xmToggleButtonGadgetClass,
					    _row, NULL);

   if (activeButton)
      XmToggleButtonGadgetSetState(_buttons[activeButton-1], True, True);
}

RadioField::~RadioField()
{
   delete _buttons;
}

int RadioField::getValue()
{
   int i = 0;
   for (; i<_numberOfButtons; i++)
      if (XmToggleButtonGadgetGetState(_buttons[i])) break;

   if (i == _numberOfButtons) return 0;
   else return ++i;
}

void RadioField::setValue(int button)
{
   XmToggleButtonGadgetSetState(_buttons[button-1], True, True);
}



MultipleInputField::MultipleInputField(Widget parent, char *row, char *labelName,
		int numberOfInputs, char **inputLabels,	unsigned char orientation,
		short columns, Bool withFrame)
{
   _numberOfInputs = numberOfInputs;
   
   Widget frame;

   if (withFrame)
      frame = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, parent, NULL);
   else frame = parent;

   // select new row
   frame = XtVaCreateManagedWidget("innerRow", xmRowColumnWidgetClass, frame, NULL);

   if (labelName)
      XtVaCreateManagedWidget(labelName, xmLabelWidgetClass, frame, NULL);

   char *hr;
   if (row) hr = row;
   else     hr = "row";
   _row = XtVaCreateManagedWidget(hr, xmRowColumnWidgetClass,
				  frame,
				  XmNpacking,	   XmPACK_COLUMN,
				  XmNorientation,  orientation,
				  XmNnumColumns,   numberOfInputs/columns,
				  XmNisAligned,	   True,
				  XmNentryAlignment, XmALIGNMENT_END,
				  NULL);

   _text = new Widget[numberOfInputs];

   for (int i = 0; i<numberOfInputs; i++)
   {
      XtVaCreateManagedWidget(inputLabels[i], xmLabelWidgetClass,
			      _row, NULL);
      _text[i] = XtVaCreateManagedWidget("text", xmTextWidgetClass,
					 _row, NULL);
   }

}

MultipleInputField::~MultipleInputField()
{
   delete _text;
}

char **MultipleInputField::getValues() // don't forget to free the return-value with kill(p, c)
{
   char **valList = new char*[_numberOfInputs];
   for (int i = 0; i<_numberOfInputs; i++)
   {
      char *h = XmTextGetString(_text[i]);
      valList[i] = strdup(h);
      XtFree(h);
   }

   return valList;
}

void MultipleInputField::setDefaults(char **inputDefaults)
{
   for (int i = 0; i<_numberOfInputs; i++)
      XtVaSetValues(_text[i], XmNvalue, inputDefaults[i], NULL);
}
      
