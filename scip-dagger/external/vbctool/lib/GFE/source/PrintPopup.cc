/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	PrintPopup.cc
//
//	Version	 :	23.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	Displays the Print-Popup
// 
//
////////////////////////////////////////////////////////////////////////	
#include <Xm/Label.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>

#include "PrintPopup.h"
#include "DrawArea.h"
#include "Postscript.h"


PrintPopup::PrintPopup(char *cname, Widget w)
   : PopupManager(cname, w, XmDIALOG_FULL_APPLICATION_MODAL)
{
   _filenameIF = NULL;
   _formatIF = NULL;
   _formatRF = NULL;
   _orientationRF = NULL;
   _titlePositionRF = NULL;
   _multipleIF = NULL;
   for (int i=0;i<=3;i++)
      _multipleDefaults[i] = NULL;
}

PrintPopup::~PrintPopup()
{
   kill (_filenameIF);
   kill (_formatIF);
   kill (_formatRF);
   kill (_orientationRF);
   kill (_titlePositionRF);
   kill (_multipleIF);

   kill (_multipleDefaults[0]);
   kill (_multipleDefaults[1]);
   kill (_multipleDefaults[2]);
   kill (_multipleDefaults[3]);
}


void PrintPopup::createActionArea()
{
   XtSetSensitive(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON), False);
}

void PrintPopup::createControlArea()
{
   _mainRow = XtVaCreateManagedWidget("printRow",
				      xmRowColumnWidgetClass,
				      _w,
				      NULL);

   _printName = XtVaCreateManagedWidget("topLabel",
					xmLabelWidgetClass,
					_mainRow,
					NULL);

   XtVaCreateManagedWidget("separator1",
			   xmSeparatorWidgetClass,
			   _mainRow,
			   NULL);

   char *pf = theDrawArea->postscript()->postscriptFilename();
   _filenameIF = new InputField(_mainRow,
				"filenameRow",
				"Filename:",
				pf,
				XmHORIZONTAL,
				False);
   delete pf;

   XtVaCreateManagedWidget("separator2",
			   xmSeparatorWidgetClass,
			   _mainRow,
			   NULL);

   _formatRF = new RadioField(_mainRow, "formatRadioRow", 5, dins, 3, XmVERTICAL,
			      3, True);

   _formatIF = new InputField(_mainRow, "formatRow", "Format:", "",
			      XmHORIZONTAL, False);

   

   _multipleIF = new MultipleInputField(_mainRow, "multipleInputField", "Margins (in cm):",
					4, _multipleLabels, XmHORIZONTAL, 2, True);

   

   _orientationRF = new RadioField(_mainRow, "orientationRadioRow", 2, orie, 1,
				   XmHORIZONTAL, 1, False);

   XtVaCreateManagedWidget("separator3",
			   xmSeparatorWidgetClass,
			   _mainRow,
			   NULL);

   Widget hrow = XtVaCreateManagedWidget("hRow", xmRowColumnWidgetClass, _mainRow, NULL);

   _printFrame = XtVaCreateManagedWidget("Frame",
					 xmToggleButtonGadgetClass,
					 hrow,
					 NULL);

   _titlePositionRow = XtVaCreateManagedWidget("titlePositionRow",
					       xmRowColumnWidgetClass,
					       _mainRow,
					       XmNorientation, XmHORIZONTAL,
					       NULL);

   _printTitle = XtVaCreateManagedWidget("Title",
					 xmToggleButtonGadgetClass,
					 _titlePositionRow,
					 NULL);

   // for more space between Title and Top/Bottom:
   XtVaCreateManagedWidget("", xmLabelWidgetClass, _titlePositionRow, NULL);

   _titlePositionRF = new RadioField(_titlePositionRow, "titlePositionRadioRow", 2,
				     posi, 1, XmHORIZONTAL, 1, False);
   XtVaSetValues(_titlePositionRF->row(), XmNpacking, XmPACK_TIGHT, NULL);

   XmString str = XmStringCreateSimple("Write to Postscript-file");
   XtVaSetValues(_printName,
		 XmNalignment, XmALIGNMENT_CENTER,
		 XmNlabelString, str,
		 NULL);
   XmStringFree(str);
   
   printTitleTB();

   str = XmStringCreateSimple("Write");
   XtVaSetValues(_w,
		 XmNokLabelString, str,
		 NULL);

   XmStringFree(str);

// set uncomplete entries to unactive
   XtUnmanageChild(_formatIF->row());
   XtUnmanageChild(_titlePositionRow);
}

void PrintPopup::ok()
{
   char *text = _filenameIF->getValue();
   char **multiText = _multipleIF->getValues();

   theDrawArea->postscript()->setMargins(multiText);
      

   theDrawArea->postscript()->setValues(XmToggleButtonGadgetGetState(_printFrame),
				     _orientationRF->getValue() -1,
				     _formatRF->getValue() -1);
				     
   theDrawArea->postscript()->printall(text);
   XtFree(text);
   kill (multiText, 4);
}

void PrintPopup::cancel()
{
}

void PrintPopup::printTitleTBCallback(Widget, XtPointer clientData, XtPointer)
{
   PrintPopup *obj = (PrintPopup *) clientData;

   obj->printTitleTB();
}

void PrintPopup::printTitleTB()
{
   static Boolean on = True;

   if (on)
   {
      on = False;
      XtSetSensitive(_titlePositionRF->row(), False);
   }
   else
   {
      on = True;
      XtSetSensitive(_titlePositionRF->row(), True);
   }
}

void PrintPopup::enableCallbacks()
{
   PopupManager::enableCallbacks();

   XtAddCallback(_printTitle, XmNvalueChangedCallback,
		 &PrintPopup::printTitleTBCallback, (XtPointer) this);
}

void PrintPopup::disableCallbacks()
{
   PopupManager::disableCallbacks();

   XtRemoveCallback(_printTitle, XmNvalueChangedCallback,
                    &PrintPopup::printTitleTBCallback, (XtPointer) this);
}


void PrintPopup::post(int l, int r, int t, int b)
{
   char buf[100];

   sprintf(buf, "%5.2f", (float) l * 2.54 / 72.0);
   kill (_multipleDefaults[0]);
   _multipleDefaults[0] = strdup(buf);

   sprintf(buf, "%5.2f", (float) r * 2.54 / 72.0);
   kill (_multipleDefaults[1]);
   _multipleDefaults[1] = strdup(buf);

   sprintf(buf, "%5.2f", (float) t * 2.54 / 72.0);
   kill (_multipleDefaults[2]);
   _multipleDefaults[2] = strdup(buf);

   sprintf(buf, "%5.2f", (float) b * 2.54 / 72.0);
   kill (_multipleDefaults[3]);
   _multipleDefaults[3] = strdup(buf);

   PopupManager::post();

   _multipleIF->setDefaults(_multipleDefaults);
}
