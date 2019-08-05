/////////////////////////////////////////////////////////////////////////
//
//
//	Filename : 	ColorContext.cc
//
//	Version	 :	18.08.94
//
//	Author	 :	Joachim Kupke
//
//	Language :	C++ 
//
//	Purpose	 :	Displays the Color/Context-Chooser-Popup
// 
//
////////////////////////////////////////////////////////////////////////	
#include <Xm/Label.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>

#include "ColorContext.h"
#include "DrawArea.h"
#include "Context.h"
#include "CCChanger.h"
#include "Status.h"
#include "ColorInformation.h"
#include "FontInformation.h"


ColorContext::ColorContext(char *cname, Widget w)
   : PopupManager(cname, w)
{
   for (int i=0; i<=MAXCHANGERS; i++)
      _changerList[i] = NULL;

   _countChangers = 0;

   _colorListItems = theDrawArea->_colorInformation->numberOfColors();
   _colorList = (XmStringTable) XtMalloc(_colorListItems * sizeof(XmString *));

   char **colorItemList = theDrawArea->_colorInformation->colorNames();

   for (int j = 0; j < _colorListItems; j++)
      _colorList[j] = XmStringCreateSimple(colorItemList[j]);

   _fontInformation = new FontInformation;

   _fontListItems = _fontInformation->numberOfFonts();
   _fontList = (XmStringTable) XtMalloc(_fontListItems * sizeof(XmString *));

   char **fontItemList = _fontInformation->fontNames();

   for (int j = 0; j < _fontListItems; j++)
      _fontList[j] = XmStringCreateSimple(fontItemList[j]);

}

ColorContext::~ColorContext()
{
   for (int i=0; i<=MAXCHANGERS; i++)
      kill(_changerList[i]);

   for (int i = 0; i<_colorListItems; i++)
      XmStringFree(_colorList[i]);
   XtFree((char*) _colorList);

   for (int i = 0; i<_fontListItems; i++)
      XmStringFree(_fontList[i]);
   XtFree((char*) _fontList);

   delete _fontInformation;
}


void ColorContext::createControlArea()
{
}

void ColorContext::createActionArea()
{
   XtUnmanageChild(XmSelectionBoxGetChild(_w, XmDIALOG_APPLY_BUTTON));

   XtRemoveAllCallbacks(XmSelectionBoxGetChild(_w, XmDIALOG_OK_BUTTON), XmNactivateCallback);

}

void ColorContext::getDialog()
{
   getSelectionDialog();

   for (int i=0; i<=MAXCHANGERS; i++)
      _changerList[i] = new CCChanger("Changer", _w, i, this);
}

void ColorContext::ok()
{
   char *text;

   text = XmTextGetString(XmSelectionBoxGetChild(_w, XmDIALOG_TEXT));

   enableCallbacks();

   if (_countChangers > MAXCHANGERS)
   {
      if (text) XtFree(text);
      return;
   }

   if (_changerList[_countChangers]->setCC(text) == SUCCESS)
   {
      _changerList[_countChangers]->post();
      _countChangers++;
   }
   
   if (text) XtFree(text);

}

void ColorContext::cancel()
{
   for (int i = 0; i< _countChangers; i++)
      _changerList[i]->unpost();

   _countChangers = 0;
}

void ColorContext::oneCleared(int nr)
{
   if (nr == MAXCHANGERS + 1 || nr == MAXCHANGERS + 2 || nr == MAXCHANGERS + 3) return;

   if (!_countChangers)
   {
      fprintf(stderr, "ERROR in ColorContext::oneCleared\n");
      return;
   }

   _countChangers--;

   if (_countChangers > MAXCHANGERS)
   {
      fprintf(stderr, "ERROR in ColorContext::oneCleared\n");
      return;
   }

   CCChanger *h = _changerList[nr];
   _changerList[nr] = _changerList[_countChangers];
   _changerList[_countChangers] = h;

   _changerList[nr]->setNr(nr);
   _changerList[_countChangers]->setNr(_countChangers);

}

void ColorContext::post()
{
   if (!_w)
      getDialog();

   int index = 0;

   for (int i = 0; i < theDrawArea->_numberRegisteredColors; i++)
      if (!theDrawArea->_colorList[i+1]->isLocked()) index++;

   for (int j = 0; j < theDrawArea->_numberRegisteredContexts; j++)
      if (!theDrawArea->_contextList[j+1]->isLocked()) index++;

   XmString *str = (XmString *)XtMalloc(index * sizeof(XmString));
   XmString listLabel = XmStringCreateSimple("Registered Colors/Contexts");
   XmString okString = XmStringCreateSimple("Change");

   int hindex = 0;

   for (int i = 0; i < theDrawArea->_numberRegisteredColors; i++)
      if (!theDrawArea->_colorList[i+1]->isLocked())
	 str[hindex++] = XmStringCreateSimple(theDrawArea->_colorList[i+1]->customName());

   for (int j = 0; j < theDrawArea->_numberRegisteredContexts; j++)
      if (!theDrawArea->_contextList[j+1]->isLocked())
	 str[hindex++] = XmStringCreateSimple(theDrawArea->_contextList[j+1]->customName());

   XtVaSetValues(_w,
		 XmNlistLabelString,   listLabel,
		 XmNlistItems,	       str,
		 XmNlistItemCount,     index,
		 XmNokLabelString,     okString,
		 NULL);

   XtSetSensitive(XmSelectionBoxGetChild(_w, XmDIALOG_HELP_BUTTON), False);

   XmStringFree(listLabel);
   XmStringFree(okString);
   
   for (int i = 0; i < index; i++)
      XmStringFree(str[i]);

   XtFree((char*) str);



   PopupManager::post();
}


void ColorContext::postSpecifiedCC(char *customName)
{
   if (_countChangers > MAXCHANGERS)
      return;

   if (_changerList[_countChangers]->setCC(customName) == SUCCESS)
   {
      _changerList[_countChangers]->post();
      _countChangers++;
   }
}

void ColorContext::unpostAllSpecifiedCC()
{
   cancel();
   unpost();
}
